/*
								AstThread.cpp

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
* \file    AstThread.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    21/01/2015
* \brief   Produce a thread for stacking frames. Save stacks which will be
*          use to make astrometry.
*/

#include "AstThread.h"

AstThread::AstThread(   string                                  recPath,
                        string                                  station,
                        StackMeth                               astMeth,
                        string                                  configurationFilePath,
                        double                                  expTime,
                        CamBitDepth                             acqFormat,
                        double                                  longi,
                        Fits                                    fitsHead,
                        int                                     fps,
                        bool                                    reduction,
                        boost::circular_buffer<StackedFrames>   *stackedFb,
                        boost::mutex                            *m_stackedFb,
                        boost::condition_variable               *c_newElemStackedFb){

    stackReduction          = reduction;
    fitsHeader              = fitsHead;
    stationName             = station;
    longitude               = longi;
    stackMthd               = astMeth;
    configFile              = configurationFilePath;
    camFormat				= acqFormat;
	thread      			= NULL;
	path_					= recPath;

	mustStop				= false;
	exposureTime    		= expTime;
	threadStopped           = false;
	camFPS                  = fps;

    stackedFramesBuffer             = stackedFb;
    m_stackedFramesBuffer           = m_stackedFb;
    c_newElemStackedFramesBuffer    = c_newElemStackedFb;

}

AstThread::~AstThread(void){

	if (thread!=NULL) delete thread;

}

void AstThread::startThread(){

    thread = new boost::thread(boost::ref(*this));

}

void AstThread::stopThread(){

	// Signal the thread to stop (thread-safe)
	mustStopMutex.lock();
	mustStop =true;
	mustStopMutex.unlock();

    while(thread->timed_join(boost::posix_time::seconds(2)) == false){

        thread->interrupt();

    }
}

void AstThread::operator()(){

    BOOST_LOG_SCOPED_THREAD_TAG("LogName", "STACK_THREAD");
	BOOST_LOG_SEV(log,notification) << "\n";
	BOOST_LOG_SEV(log,notification) << "==============================================";
	BOOST_LOG_SEV(log,notification) << "============== Start stack thread ============";
	BOOST_LOG_SEV(log,notification) << "==============================================";

    // Flag to know if the thread has to be stopped.
    bool stop = false;

    do{

        try{

            namespace fs = boost::filesystem;

            boost::mutex::scoped_lock lock(*m_stackedFramesBuffer);
            while (stackedFramesBuffer->empty()) c_newElemStackedFramesBuffer->wait(lock);

            double t = (double)getTickCount();

            vector<string> firstDateString  = TimeDate::splitStringToStringVector(stackedFramesBuffer->front().startDate);
            vector<int> firstDateInt        = TimeDate::splitStringToIntVector(stackedFramesBuffer->front().startDate);
            vector<int> lastDateInt         = TimeDate::splitStringToIntVector(stackedFramesBuffer->front().endDate);
            string date                     = firstDateString.at(0)+"-"+firstDateString.at(1)+"-"+firstDateString.at(2)+"T"+firstDateString.at(3)+":"+firstDateString.at(4)+":"+firstDateString.at(5);
            string acqDate                  = stackedFramesBuffer->front().acqFirstDate;
            int camGain                     = stackedFramesBuffer->front().gain;
            int camExp                      = stackedFramesBuffer->front().exp;
            int imgSum                      = stackedFramesBuffer->front().imgSum;

            Mat stackImg;
            stackedFramesBuffer->front().stackedImg.copyTo(stackImg);
            stackedFramesBuffer->pop_front();

            lock.unlock();

            string root                     = path_ + stationName + "_" + firstDateString.at(0) + firstDateString.at(1) + firstDateString.at(2) +"/";
            string subDir                   = "astro/";
            string finalPath                = root + subDir;

            path p(path_);
            path p1(root);
            path p2(root + subDir);

            if(fs::exists(p)){

                if(fs::exists(p1)){

                    if(!fs::exists(p2)){

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

            double  debObsInSeconds = firstDateInt.at(3)*3600 + firstDateInt.at(4)*60 + firstDateInt.at(5);
            double  endObsInSeconds = lastDateInt.at(3)*3600 + lastDateInt.at(4)*60 + lastDateInt.at(5);
            double  elapTime        = endObsInSeconds - debObsInSeconds;
            double  julianDate      = TimeDate::gregorianToJulian_2(firstDateInt);
            double  julianCentury   = TimeDate::julianCentury(julianDate);
            double  sideralT        = TimeDate::localSideralTime_2(julianCentury, firstDateInt.at(3), firstDateInt.at(4), firstDateInt.at(5), longitude);

            // Fits creation.
            Fits2D newFits(finalPath,fitsHeader);

            boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
            newFits.setDate(to_iso_extended_string(time));

            // Frame exposure time (sec.)
            newFits.setOntime(camExp);
            // Detector gain
            newFits.setGaindb(camGain);
            // Acquisition date of the first frame 'YYYY-MM-JJTHH:MM:SS.SS'
            newFits.setDateobs(acqDate);
            // Integration time : 1/fps * nb_frames (sec.)
            newFits.setExposure((1.0f/camFPS)*imgSum);
            // end obs. date - start obs. date (sec.)
            newFits.setElaptime(elapTime);
            // Sideral time
            newFits.setCrval1(sideralT);
            // Fps
            newFits.setCd3_3((double)camFPS);
            // Projection and reference system
            newFits.setCtype1("RA---ARC");
            newFits.setCtype2("DEC--ARC");
            // Equinox
            newFits.setEquinox(2000.0);

            switch(stackMthd){

                case MEAN :

                    {
                        // 'SINGLE' 'SUM' 'AVERAGE' ('MEDIAN')
                        newFits.setObsmode("AVERAGE");
                        stackImg = stackImg/imgSum;
                        double minVal, maxVal;
                        minMaxLoc(stackImg, &minVal, &maxVal);

                        // Saturated or max value (not saturated) in case where OBS_MODE = SUM
                        newFits.setSaturate(maxVal);

                    }

                    break;

                case SUM :

                    {
                        // 'SINGLE' 'SUM' 'AVERAGE' ('MEDIAN')
                        newFits.setObsmode("SUM");
                        double minVal, maxVal;
                        minMaxLoc(stackImg, &minVal, &maxVal);

                        // Saturated or max value (not saturated) in case where OBS_MODE = SUM
                        newFits.setSaturate(maxVal);

                    }

                    break;

            }

            if(stackReduction){

                Mat newMat ;

                float bzero     = 0.0;
                float bscale    = 1.0;

                ImgReduction::dynamicReductionByFactorDivision(stackImg, camFormat, imgSum, bzero, bscale).copyTo(newMat);

                newFits.setBzero(bzero);
                newFits.setBscale(bscale);

                switch(camFormat){

                    case MONO_8 :

                        {

                             newFits.writeFits(newMat, C8, firstDateString, true,"" );

                        }

                        break;

                    case MONO_12 :

                        {

                             newFits.writeFits(newMat, S16, firstDateString, true,"" );

                        }

                        break;

                }

            }else{

                // Save fits in 32 bits.
                newFits.writeFits(stackImg, F32, firstDateString, true,""  );

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

