/*
								AcqThread.cpp

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
*	Last modified:		21/01/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    AcqThread.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    21/01/2015
* \brief   Acquisition thread.
*/

#include "AcqThread.h"

boost::log::sources::severity_logger< LogSeverityLevel >  AcqThread::logger;
AcqThread::_Init AcqThread::_initializer;

AcqThread::AcqThread(	CamType									camType,
						boost::mutex							*cfg_m,
						string									cfg_p,
					    boost::circular_buffer<Frame>           *fb,
						boost::mutex                            *fb_m,
						boost::condition_variable               *fb_c,
						bool									*sSignal,
						boost::mutex                            *sSignal_m,
						boost::condition_variable               *sSignal_c,
						bool                                    *dSignal,
						boost::mutex                            *dSignal_m,
						boost::condition_variable               *dSignal_c){

    acquisitionThread			    = NULL;
    mustStop				        = false;
	cam								= NULL;
    cfg_mutex						= cfg_m;
	cfg_path						= cfg_p;
	srcType							= camType;
    frameBuffer                     = fb;
    frameBuffer_mutex               = fb_m;
    frameBuffer_condition           = fb_c;

    stackSignal						= sSignal;
    stackSignal_mutex				= sSignal_m;
    stackSignal_condition			= sSignal_c;

    detSignal						= dSignal;
    detSignal_mutex                 = dSignal_m;
    detSignal_condition             = dSignal_c;

    frameCpt                        = 0;
    nbFailGrabbedFrames             = 0;
    nbSuccessGrabbedFrames          = 0;

	threadTerminated				= false;

}

AcqThread::~AcqThread(void){

    if(cam != NULL) delete cam;
	if(acquisitionThread != NULL) delete acquisitionThread;

}

void AcqThread::join(){

	acquisitionThread->join();

}

void AcqThread::stopThread(){

	// Signal the thread to stop (thread-safe)
	mustStopMutex.lock();
	mustStop = true;
	mustStopMutex.unlock();

	// Wait for the thread to finish.
	
	while(acquisitionThread->timed_join(boost::posix_time::seconds(2)) == false){

        acquisitionThread->interrupt();

    }

}

bool AcqThread::startThread(){

	BOOST_LOG_SEV(logger,normal) << "Create new Device.";

	cam = new Device(srcType);

	BOOST_LOG_SEV(logger,normal) << "Prepare device.";
	boost::mutex::scoped_lock lock_(*cfg_mutex);
	if(!cam->prepareDevice(srcType, cfg_path)){
		lock_.unlock();
		BOOST_LOG_SEV(logger,fail) << "Fail to prepare device.";
		return false;
	}
	lock_.unlock();
	BOOST_LOG_SEV(logger,normal) << "Sucess to prepare device.";
	BOOST_LOG_SEV(logger,normal) << "Create acquisition thread.";

    acquisitionThread = new boost::thread(boost::ref(*this));
	return true;

}

bool AcqThread::getThreadTerminatedStatus(){

	return threadTerminated;

}

void AcqThread::operator()(){

	bool stop = false;

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "ACQ_THREAD");
	BOOST_LOG_SEV(logger,notification) << "\n";
	BOOST_LOG_SEV(logger,notification) << "==============================================";
	BOOST_LOG_SEV(logger,notification) << "========== Start acquisition thread ==========";
	BOOST_LOG_SEV(logger,notification) << "==============================================";

	try{

		do{

			Frame newFrame;

			double tacq = (double)getTickCount();
			BOOST_LOG_SEV(logger, normal) << "============= FRAME " << frameCpt << " ============= ";
			cout << "============= FRAME " << frameCpt << " ============= " << endl;

			if(cam->grabImage(newFrame)){

				newFrame.setNumFrame(frameCpt);

				boost::mutex::scoped_lock lock(*frameBuffer_mutex);
				frameBuffer->push_back(newFrame);
				lock.unlock();
			
				boost::mutex::scoped_lock lock2(*detSignal_mutex);
				*detSignal = true;
				detSignal_condition->notify_one();
				lock2.unlock();

				boost::mutex::scoped_lock lock3(*stackSignal_mutex);
				*stackSignal = true;
				stackSignal_condition->notify_one();
				lock3.unlock();
		
				nbFailGrabbedFrames++;

			}else{

				BOOST_LOG_SEV(logger, fail) << "> Fail to grab frame " << frameCpt + 1;
				nbFailGrabbedFrames++;
			}

			frameCpt++;

			tacq = (((double)getTickCount() - tacq)/getTickFrequency())*1000;
			std::cout << " [ TIME ACQ ] : " << tacq << " ms" << endl;
			BOOST_LOG_SEV(logger, normal) << " [ TIME ACQ ] : " << tacq << " ms";
		
			mustStopMutex.lock();
			stop = mustStop;
			mustStopMutex.unlock();
		
		}while(stop == false && !cam->getDeviceStopStatus());

	}catch(const boost::thread_interrupted&){

			BOOST_LOG_SEV(logger,notification) << "Acquisition Thread INTERRUPTED";
            cout << "Acquisition Thread INTERRUPTED" <<endl;

    }
		
    cam->acqStop();
	cam->grabStop();

	threadTerminated = true;

	std::cout << "Acquisition Thread terminated." << endl;

}
