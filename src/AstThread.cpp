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
                        string                      astMeth,
                        string                      configurationFilePath,
                        Fifo<Frame>                 *frame_queue,
                        int                         interval,
                        double                      expTime,
                        int                         acqFormat,
                        double                      longi,
                        boost::mutex                *m_frame_queue,
                        boost::condition_variable   *c_queue_full,
                        boost::condition_variable   *c_queue_new,
                        Fits fitsHead){

    fitsHeader              = fitsHead;
    stationName             = station;
    longitude               = longi;
    fitsMethod              = astMeth;
    configFile              = configurationFilePath;
    formatPixel				= acqFormat;
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
	BOOST_LOG_SEV(log, critical) << "\n";
	BOOST_LOG_SEV(log,notification) << "Astro thread started.";

    bool stop = false;

    int totalImgToSummed = exposureTime * 30;

    Mat resImg, img;

    int nbSummedImg = 0;

    bool recDateObs = true;
    string dateObs ="";
    vector <int> dateObsDeb;
    vector <int> dateObsEnd;
    int gain;
    int exposure;

    do{

        try{

            namespace fs = boost::filesystem;

            nbSummedImg     = 0;
            recDateObs      = true;
            dateObs         ="";
            dateObsDeb.clear();
            dateObsEnd.clear();
            gain = 0;
            exposure = 0;

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

            resImg = Mat::zeros(framesQueue->getFifoElementAt(0).getImg().rows,framesQueue->getFifoElementAt(0).getImg().cols, CV_32FC1);
            img = Mat::zeros(framesQueue->getFifoElementAt(0).getImg().rows,framesQueue->getFifoElementAt(0).getImg().cols,CV_32FC1);

            double t = (double)getTickCount();

            while(nbSummedImg<totalImgToSummed){

                BOOST_LOG_SEV(log,notification) << "Wait new image";

                while (!framesQueue->getThreadReadStatus("astCap")) condNewElem->wait(lock);
                framesQueue->setThreadRead("astCap",false);

                if(recDateObs){

                    dateObs = framesQueue->getFifoElementAt(0).getAcqDate();
                    dateObsDeb = framesQueue->getFifoElementAt(0).getDate();

                    gain = framesQueue->getFifoElementAt(0).getGain();

                    exposure = framesQueue->getFifoElementAt(0).getExposure();

                    recDateObs = false;

                }

                dateObsEnd.clear();
                dateObsEnd = framesQueue->getFifoElementAt(0).getDate();

                framesQueue->getFifoElementAt(0).getImg().convertTo(img, CV_32FC1);
                accumulate(img,resImg);
                nbSummedImg++;

            };

            BOOST_LOG_SEV(log,notification) << "Terminate to sum frames";
            cout << " Terminate to sum frames : "<< nbSummedImg <<endl;

            //moyenne
            if(fitsMethod == "mean"){

                resImg = resImg/nbSummedImg;
            }

            int debObsInSeconds = dateObsDeb.at(3)*3600 + dateObsDeb.at(4)*60 + dateObsDeb.at(5);
            int endObsInSeconds = dateObsEnd.at(3)*3600 + dateObsEnd.at(4)*60 + dateObsEnd.at(5);
            int elapTime = endObsInSeconds - debObsInSeconds;


            double julianDate = TimeDate::gregorianToJulian_2(dateObsDeb);
            double julianCentury = TimeDate::julianCentury(julianDate);

            double sideralT = TimeDate::localSideralTime_2(julianCentury, dateObsDeb.at(3), dateObsDeb.at(4), dateObsDeb.at(5), longitude);

            //Création d'un fits 2D
            Fits2D newFits(finalPath,fitsHeader);
            newFits.setOntime(totalImgToSummed / 30);
            newFits.setGaindb(gain);
            newFits.setObsmode("30");
            newFits.setDateobs(dateObs);//dateObs
            newFits.setSaturate(pow(2,formatPixel) - 1);
            newFits.setRadesys("ICRS");
            newFits.setEquinox(2000.0);
            newFits.setCtype1("RA---ARC");
            newFits.setCtype2("DEC--ARC");
            newFits.setExposure(exposure * 1e-6);
            newFits.setElaptime(elapTime);
            newFits.setCrval1(sideralT);//sideraltime

            if(newFits.writeFits(resImg, F32 , 0, true ))
                cout << "Fits saved" << endl;
            else
                cout << "Fits not saved" << endl;


            t = (((double)getTickCount() - t)/getTickFrequency())*1000;
            cout << "Astro thread time : " <<std::setprecision(5)<< std::fixed<< t << " ms"<< endl;

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

