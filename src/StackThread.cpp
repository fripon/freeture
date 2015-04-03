/*
								StackThread.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau -- FRIPON-GEOPS-UPSUD
*
*	License:		GNU General Public License
*
*	FreeTure is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*	FreeTure is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*	You should have received a copy of the GNU General Public License
*	along with FreeTure. If not, see <http://www.gnu.org/licenses/>.
*
*	Last modified:		20/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    StackThread.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    19/06/2014
* \brief   Stack frames.
*/

#include "StackThread.h"

boost::log::sources::severity_logger< LogSeverityLevel >  StackThread::logger;
StackThread::_Init StackThread::_initializer;

StackThread::StackThread(   boost::mutex							*cfg_m,
							string									cfg_p,
							bool									*sS,
							boost::mutex							*sS_m,
							boost::condition_variable				*sS_c,
							boost::circular_buffer<Frame>		    *fb,
							boost::mutex                            *fb_m,
							boost::condition_variable               *fb_c){

	cfg_mutex				= cfg_m;
	cfgPath					= cfg_p;
	thread      			= NULL;
	mustStop				= false;
    frameBuffer             = fb;
    frameBuffer_mutex       = fb_m;
    frameBuffer_condition   = fb_c;
	stackSignal				= sS;
    stackSignal_mutex		= sS_m;
    stackSignal_condition	= sS_c;
	completeDataPath		= "";

}

StackThread::~StackThread(void){

	if (thread!=NULL) delete thread;

}

bool StackThread::startThread(){

	boost::mutex::scoped_lock lock(*cfg_mutex);
	if(!loadStackParameters()){
		lock.unlock();
		return false;
	}
	lock.unlock();

    thread = new boost::thread(boost::ref(*this));
	return true;
}

void StackThread::stopThread(){

	// Signal the thread to stop (thread-safe)
	mustStopMutex.lock();
	mustStop =true;
	mustStopMutex.unlock();

    while(thread->timed_join(boost::posix_time::seconds(2)) == false){

        thread->interrupt();

    }
}

bool StackThread::loadStackParameters(){

	try{

		Configuration cfg;
		cfg.Load(cfgPath);

        // Get acquisition frequency.
		int ACQ_FPS;
		cfg.Get("ACQ_FPS", ACQ_FPS);

        // Get camera format.
		string acq_bit_depth; cfg.Get("ACQ_BIT_DEPTH", acq_bit_depth);
		EParser<CamBitDepth> cam_bit_depth;
		ACQ_BIT_DEPTH = cam_bit_depth.parseEnum("ACQ_BIT_DEPTH", acq_bit_depth);

        // Get time of stacking frames.
		cfg.Get("STACK_TIME", STACK_TIME);
		STACK_TIME = STACK_TIME * ACQ_FPS;

        // Get time to wait before a new stack.
		cfg.Get("STACK_INTERVAL", STACK_INTERVAL);

        // Get stack method to use.
		string stack_method; cfg.Get("STACK_MTHD", stack_method);
		EParser<StackMeth> stack_mth;
		STACK_MTHD = stack_mth.parseEnum("STACK_MTHD", stack_method);

        cfg.Get("STACK_REDUCTION", STACK_REDUCTION);
		cfg.Get("STATION_NAME", STATION_NAME);
		cfg.Get("CFG_FILECOPY_ENABLED", CFG_FILECOPY_ENABLED);
		cfg.Get("DATA_PATH", DATA_PATH);

		fitsHeader.loadKeywordsFromConfigFile(cfgPath);


	}catch(exception& e){

		cout << e.what() << endl;
		return false;

	}

	return true;

}

bool StackThread::buildStackDataDirectory(string date){

	namespace fs = boost::filesystem;
	string	YYYYMMDD	= TimeDate::get_YYYYMMDD_fromDateString(date);
	string	root		= DATA_PATH + STATION_NAME + "_" + YYYYMMDD +"/";
    string	subDir		= "astro/";
    string	finalPath	= root + subDir;

	completeDataPath	= finalPath;

    path p(DATA_PATH);
    path p1(root);
    path p2(root + subDir);

    if(fs::exists(p)){

        if(fs::exists(p1)){

            if(!fs::exists(p2)){
				return false;
                cout << "directory not exist : " << p2.string() << endl;

                if(!fs::create_directory(p2)){
					return false;
                    cout << "directory not created : " << p2.string() << endl;
                    //BOOST_LOG_SEV(log,notification) << "Unable to create destination directory" << p2.string();

                }else{

					return true;
                    cout << "directory created : " << p2.string() << endl;
                   // BOOST_LOG_SEV(log,notification) << "Following directory created : " << p2.string();

                }
            }

        }else{

            if(!fs::create_directory(p1)){
                cout << "Unable to create destination directory" << endl;
				return false;
                //BOOST_LOG_SEV(log,notification) << "Unable to create destination directory" << p1.string();

            }else{
                cout << "Following directory created : " << p1.string() << endl;
               // BOOST_LOG_SEV(log,notification) << "Following directory created : " << p1.string();

                if(!fs::create_directory(p2)){
					return false;
                   // BOOST_LOG_SEV(log,notification) << "Unable to create destination directory" << p2.string();

                }else{
					return true;
                    //BOOST_LOG_SEV(log,notification) << "Following directory created : " << p2.string();

                }
            }
        }

    }else{

        if(!fs::create_directory(p)){
			return false;
            //BOOST_LOG_SEV(log,notification) << "Unable to create destination directory" << p.string();

        }else{

            if(!fs::create_directory(p1)){
				return false;
                //BOOST_LOG_SEV(log,notification) << "Unable to create destination directory" << p1.string();

            }else{

                //BOOST_LOG_SEV(log,notification) << "Following directory created : " << p1.string();

                if(!fs::create_directory(p2)){
					return false;
                   // BOOST_LOG_SEV(log,notification) << "Unable to create destination directory" << p2.string();

                }else{
					return true;
                    //BOOST_LOG_SEV(log,notification) << "Following directory created : " << p2.string();

                }
            }
        }
    }
}

void StackThread::operator()(){

    BOOST_LOG_SCOPED_THREAD_TAG("LogName", "STACK_THREAD");
	BOOST_LOG_SEV(logger,notification) << "\n";
	BOOST_LOG_SEV(logger,notification) << "==============================================";
	BOOST_LOG_SEV(logger,notification) << "============== Start stack thread ============";
	BOOST_LOG_SEV(logger,notification) << "==============================================";

    bool stop = false;

    do{

        try{

			std::cout << "Stack Thread sleep... " << endl;
			boost::this_thread::sleep(boost::posix_time::millisec(STACK_INTERVAL*1000));
			std::cout << "Stack Thread wake up ! " << endl;

			Stack stack(STACK_TIME);

			do{

				// Communication with AcqThread.
				boost::mutex::scoped_lock lock(*stackSignal_mutex);
				while(!(*stackSignal)) stackSignal_condition->wait(lock);
				 *stackSignal = false;
				lock.unlock();

				double t = (double)getTickCount();

				// Fetch last frame grabbed.
				boost::mutex::scoped_lock lock2(*frameBuffer_mutex);
				Frame newFrame = frameBuffer->back();
				std::cout << "New frame received by stackThread :  "<< newFrame.getNumFrame() << endl;
				lock2.unlock();

				// Add the new frame to the stack.
				stack.addFrame(newFrame);

				if(stack.getFullStatus()){

					if(buildStackDataDirectory(stack.getDateFirstFrame()))
						stack.saveStack(fitsHeader, completeDataPath, STACK_MTHD, STATION_NAME, STACK_REDUCTION);

                    cout << "Save stack : " << completeDataPath << endl;

				}

				t = (((double)getTickCount() - t)/getTickFrequency())*1000;
				std::cout << "[ Stack time ] : " << std::setprecision(5) << std::fixed << t << " ms" << endl;

			}while(!stack.getFullStatus());

            // Get the "must stop" state (thread-safe)
            mustStopMutex.lock();
            stop = mustStop;
            mustStopMutex.unlock();

        }catch(const boost::thread_interrupted&){

            std::cout << "Stack thread INTERRUPTED" <<endl;
            break;

        }

    }while(!stop);

	std::cout << "Stack Thread terminated" << endl;

}

