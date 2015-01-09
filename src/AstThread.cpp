/*
								AstThread.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
 * @file    AstThread.cpp
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    20/10/2014
 */

#include "AstThread.h"

AstThread::AstThread(   string                      recPath,
                        string                      station,
                        AstStackMeth                astMeth,
                        string                      configurationFilePath,
                        Fifo<Frame>                 *frame_queue,
                        int                         interval,
                        double                      expTime,
                        CamBitDepth                 acqFormat,
                        double                      longi,
                        boost::mutex                *m_frame_queue,
                        boost::condition_variable   *c_queue_full,
                        boost::condition_variable   *c_queue_new,
                        Fits fitsHead,
                        int fps,
                        bool reduction){

    stackReduction          = reduction;
    fitsHeader              = fitsHead;
    stationName             = station;
    longitude               = longi;
    stackMthd              = astMeth;
    configFile              = configurationFilePath;
    camFormat				= acqFormat;
	thread      			= NULL;
	path_					= recPath;
	framesQueue				= frame_queue;
	m_mutex_queue			= m_frame_queue;
	capInterval				= interval;
	condFill        		= c_queue_full;
	condNewElem             = c_queue_new;
	mustStop				= false;
	exposureTime    		= expTime;
	threadStopped           = false;
	camFPS = fps;

}

AstThread::~AstThread(void){

	if (thread!=NULL) delete thread;

}

void AstThread::startCapture(){

    BOOST_LOG_SEV(log, notification) << " Astro thread memory allocation ";
    thread = new boost::thread(boost::ref(*this));

}

void AstThread::stopCapture(){

	// Signal the thread to stop (thread-safe)
	mustStopMutex.lock();
	mustStop=true;
	mustStopMutex.unlock();

    BOOST_LOG_SEV(log, notification) << "Wait 1 seconds for astro cap thread finish...";

    while(thread->timed_join(boost::posix_time::seconds(2)) == false){

        BOOST_LOG_SEV(log, notification) << "Thread not stopped, interrupt it now";
        thread->interrupt();
        BOOST_LOG_SEV(log, notification) << "Interrupt Request sent";

    }

    BOOST_LOG_SEV(log, notification) << "Astro thread stopped";

}

// Thread function
void AstThread::operator()(){

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "imgAstThread");
	BOOST_LOG_SEV(log,notification) << "\n Astro thread started.";

    // Flag to know if the thread has to be stopped.
    bool stop = false;

    // Number of frames to sum.
    // If we want to sum 1 minute of frames with a camera which can grabs 30 frames per seconds,
    // we have finally to sum 60 * 30 = 1800 frames.
    int imgToSum = exposureTime * camFPS;

    // Total number of sum images.
    int imgSum;

    bool recDateObs = true;
    string dateObs ="";
    vector <int> dateObsDeb;
    vector <int> dateObsEnd;
    int camGain;
    int camExp;

    do{

        try{

            namespace fs = boost::filesystem;

            imgSum     = 0;
            recDateObs      = true;
            dateObs         ="";
            dateObsDeb.clear();

            dateObsEnd.clear();
            camGain = 0;
            camExp = 0;

            boost::this_thread::sleep(boost::posix_time::millisec(capInterval*1000));

            boost::mutex::scoped_lock lock(*m_mutex_queue);

            while (!framesQueue->getFifoIsFull()) condFill->wait(lock);




            string root = path_ + stationName + "_" + framesQueue->getFifoElementAt(0).getDateString().at(0) + framesQueue->getFifoElementAt(0).getDateString().at(1) + framesQueue->getFifoElementAt(0).getDateString().at(2) +"/";

            string subDir =  "astro/";

            string finalPath = root + subDir;

            path p(path_);

            path p1(root);

            path p2(root + subDir);

            if(fs::exists(p)){

                cout << "directory exist: " << p.string() << endl;

                if(fs::exists(p1)){

                    cout << "directory exist : " << p1.string() << endl;

                    BOOST_LOG_SEV(log,notification) << "Destination directory " << p1.string() << " already exists.";

                    if(fs::exists(p2)){

                        cout << "directory exist : " << p2.string() << endl;


                        BOOST_LOG_SEV(log,notification) << "Destination directory " << p2.string() << " already exists.";

                    }else{

                        cout << "directory not exist : " << p2.string() << endl;


                        if(!fs::create_directory(p2)){

                             cout << "directory not created : " << p2.string() << endl;

                            BOOST_LOG_SEV(log,notification) << "Unable to create destination directory" << p2.string();

                        }else{

                            cout << "directory created : " << p2.string() << endl;

                            BOOST_LOG_SEV(log,notification) << "Following directory created : " << p2.string();

                        }
                    }

                }else{

                    if(!fs::create_directory(p1)){
                        cout << "Unable to create destination directory" << endl;
                        BOOST_LOG_SEV(log,notification) << "Unable to create destination directory" << p1.string();

                    }else{
                        cout << "Following directory created : " << p1.string() << endl;
                        BOOST_LOG_SEV(log,notification) << "Following directory created : " << p1.string();

                        if(!fs::create_directory(p2)){

                            BOOST_LOG_SEV(log,notification) << "Unable to create destination directory" << p2.string();

                        }else{

                            BOOST_LOG_SEV(log,notification) << "Following directory created : " << p2.string();

                        }
                    }
                }

            }else{

                if(!fs::create_directory(p)){

                    BOOST_LOG_SEV(log,notification) << "Unable to create destination directory" << p.string();

                }else{

                    if(!fs::create_directory(p1)){

                        BOOST_LOG_SEV(log,notification) << "Unable to create destination directory" << p1.string();

                    }else{

                        BOOST_LOG_SEV(log,notification) << "Following directory created : " << p1.string();

                        if(!fs::create_directory(p2)){

                            BOOST_LOG_SEV(log,notification) << "Unable to create destination directory" << p2.string();

                        }else{

                            BOOST_LOG_SEV(log,notification) << "Following directory created : " << p2.string();

                        }
                    }
                }
            }



            // Container of 32 bits to accumulate frames.
            Mat resImg = Mat::zeros(framesQueue->getFifoElementAt(0).getImg().rows,framesQueue->getFifoElementAt(0).getImg().cols, CV_32FC1);
            Mat img = Mat::zeros(framesQueue->getFifoElementAt(0).getImg().rows,framesQueue->getFifoElementAt(0).getImg().cols,CV_32FC1);

            double t = (double)getTickCount();

            while(imgSum < imgToSum){

                BOOST_LOG_SEV(log,notification) << "Wait new image";

                while (!framesQueue->getThreadReadStatus("astCap")) condNewElem->wait(lock);
                framesQueue->setThreadRead("astCap",false);

                if(recDateObs){

                    dateObs = framesQueue->getFifoElementAt(0).getAcqDate();
                    dateObsDeb = framesQueue->getFifoElementAt(0).getDate();

                    camGain = framesQueue->getFifoElementAt(0).getGain();

                    camExp = framesQueue->getFifoElementAt(0).getExposure();

                    recDateObs = false;

                }

                dateObsEnd.clear();
                dateObsEnd = framesQueue->getFifoElementAt(0).getDate();

                framesQueue->getFifoElementAt(0).getImg().convertTo(img, CV_32FC1);
                accumulate(img,resImg);
                imgSum++;

            };

            BOOST_LOG_SEV(log,notification) << "Terminate to sum frames";
            cout << " Terminate to sum frames : "<< imgSum <<endl;

            int     debObsInSeconds = dateObsDeb.at(3)*3600 + dateObsDeb.at(4)*60 + dateObsDeb.at(5);
            int     endObsInSeconds = dateObsEnd.at(3)*3600 + dateObsEnd.at(4)*60 + dateObsEnd.at(5);
            int     elapTime        = endObsInSeconds - debObsInSeconds;
            double  julianDate      = TimeDate::gregorianToJulian_2(dateObsDeb);
            double  julianCentury   = TimeDate::julianCentury(julianDate);
            double  sideralT        = TimeDate::localSideralTime_2(julianCentury, dateObsDeb.at(3), dateObsDeb.at(4), dateObsDeb.at(5), longitude);

            // Fits creation.
            Fits2D newFits(finalPath,fitsHeader);
            // Frame exposure time (sec.)
            newFits.setOntime(camExp);
            // Detector gain
            newFits.setGaindb(camGain);
            // Acquisition date of the first frame 'YYYY-MM-JJTHH:MM:SS.SS'
            newFits.setDateobs(dateObs);
            // Integration time : 1/fps * nb_frames (sec.)
            newFits.setExposure((1.0f/camFPS)*imgSum);
            // end obs. date - start obs. date (sec.)
            newFits.setElaptime(elapTime);
            // Sideral time
            newFits.setCrval1(sideralT);
            // Fps
            newFits.setCd3_3((double)camFPS);

            switch(stackMthd){

                case MEAN :

                    {
                        // 'SINGLE' 'SUM' 'AVERAGE' ('MEDIAN')
                        newFits.setObsmode("AVERAGE");
                        resImg = resImg/imgSum;
                        double minVal, maxVal;
                        minMaxLoc(resImg, &minVal, &maxVal);

                        // Saturated or max value (not saturated) in case where OBS_MODE = SUM
                        newFits.setSaturate(maxVal);

                    }

                    break;

                case SUM :

                    {
                        // 'SINGLE' 'SUM' 'AVERAGE' ('MEDIAN')
                        newFits.setObsmode("SUM");
                        double minVal, maxVal;
                        minMaxLoc(resImg, &minVal, &maxVal);

                        // Saturated or max value (not saturated) in case where OBS_MODE = SUM
                        newFits.setSaturate(maxVal);

                    }

                    break;

            }

            if(stackReduction){

                Mat newMat ;

                float bzero     = 0.0;
                float bscale    = 1.0;

                ImgReduction::dynamicReductionByFactorDivision(resImg, camFormat, imgSum, bzero, bscale).copyTo(newMat);

                newFits.setBzero(bzero);
                newFits.setBscale(bscale);

                switch(camFormat){

                    case MONO_8 :

                        {

                             newFits.writeFits(newMat, bit_depth_enum::C8, 0, true,"" );

                        }

                        break;

                    case MONO_12 :

                        {

                             newFits.writeFits(newMat, bit_depth_enum::S16, 0, true,"" );

                        }

                        break;

                }

            }else{

                // Save fits in 32 bits.
                newFits.writeFits(resImg, F32 , 0, true,"" );

            }

            t = (((double)getTickCount() - t)/getTickFrequency())*1000;
            cout << "Astro thread time : " << std::setprecision(5) << std::fixed << t << " ms" << endl;

            //Get the "must stop" state (thread-safe)
            mustStopMutex.lock();
            stop = mustStop;
            mustStopMutex.unlock();

        }catch(const boost::thread_interrupted&){

            cout<< "Astro capture thread INTERRUPTED" <<endl;
            break;

        }

    }while(!stop);

	BOOST_LOG_SEV(log,notification) << "Astro thread terminated";

}

