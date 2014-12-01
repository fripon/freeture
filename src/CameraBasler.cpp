/*
								CameraBasler.cpp

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
 * @file    CameraBasler.cpp
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    13/06/2014
 */

#include "CameraBasler.h"

#ifdef USE_PYLON
    #include "CameraSDKPylon.h"
#else
    #include "CameraSDKAravis.h"
#endif

CameraBasler::CameraBasler(){

    m_thread = NULL;

    #ifdef USE_PYLON
        camera = new CameraSDKPylon();
    #else
        camera = new CameraSDKAravis();
    #endif

     cpt = 0;

}




CameraBasler::CameraBasler( int     exposure,
                            int     gain,
                            bool    saveFits2D,
                            bool    saveBmp,
                            int     acqFormat,
                            string  configPath,
                            string saveLocation,
                            Fits fitsHead){

    m_thread			= NULL;
    initialExpValue     = exposure;
    initialGainValue    = gain;
    saveFits          = saveFits2D;
    saveImg             = saveBmp;
    format              = acqFormat;
    configFile          = configPath;
    savePath            = saveLocation;
    fitsHeader          = fitsHead;

    #ifdef USE_PYLON
        camera = new CameraSDKPylon();
    #else
        camera = new CameraSDKAravis();
    #endif

     cpt = 0;

}

CameraBasler::CameraBasler( Fifo<Frame> *queueRam,
						   boost::mutex *m_queue,
						   boost::condition_variable *c_queueFull,
						   boost::condition_variable *c_queueNew,
						   int initialExposure,
						   int initialGain,
						   Mat frameMask,
                           bool enableMask){

    m_thread			= NULL;
    framesQueue			= queueRam;
    mutexQueue			= m_queue;
    mustStop			= false;
    condQueueFill		= c_queueFull;
    condQueueNewElement	= c_queueNew;
    initialExpValue     = initialExposure;
    initialGainValue    = initialGain;
    frameMask.copyTo(mask);
    maskEnable          = enableMask;

    #ifdef USE_PYLON
        camera = new CameraSDKPylon();
    #else
        camera = new CameraSDKAravis();
    #endif

    cpt = 0;

    threadStopped = false;

}

CameraBasler::~CameraBasler(void){
BOOST_LOG_SEV(log, notification) << "CameraBasler destructor";
    if (camera!=NULL){

        BOOST_LOG_SEV(log, notification) << "delete camera";
        delete camera;
    }


	if (m_thread!=NULL){

        BOOST_LOG_SEV(log, notification) << "delete acq thread";
        delete m_thread;

	}else
	BOOST_LOG_SEV(log, notification) << "no delete acq thread";

}

void    CameraBasler::join(){

	m_thread->join();

}

bool    CameraBasler::setSelectedDevice(int id, string name){

    return camera->chooseDevice(id, name);

}

void    CameraBasler::getListCameras(){

    camera->listCameras();

}

// Stop the thread
void    CameraBasler::stopThread(){

	// Signal the thread to stop (thread-safe)
	mustStopMutex.lock();
	mustStop = true;
	mustStopMutex.unlock();

	// Wait for the thread to finish.
	if (m_thread!=NULL){

        m_thread->join();

	}

    // The thread has been stopped
    BOOST_LOG_SEV(log, notification) << "Thread stopped";

}

void    CameraBasler::startThread(){

    camera->grabStart();

    m_thread = new boost::thread(boost::ref(*this));
    BOOST_LOG_SEV(log, notification) << "address: " << m_thread;

   // camera->acqStart();

}

void    CameraBasler::startGrab(){

    camera->grabStart();

    //camera->acqStart();

}

void    CameraBasler::stopGrab(){



}

int     CameraBasler::getCameraHeight(){

    return camera->getHeight();

}

int     CameraBasler::getCameraWidth(){

    return camera->getWidth();

}

void    CameraBasler::setCameraExposureTime(double exposition){

    camera->setExposureTime(exposition);

}

void    CameraBasler::setCameraGain(int gain){

     camera->setGain(gain);

}

void    CameraBasler::setCameraPixelFormat(int depth){

    camera->setPixelFormat(depth);

}

template <typename Container> void stringTOK(Container &container, string const &in, const char * const delimiters = "_"){


    const string::size_type len = in.length();

    string::size_type i = 0;


    while (i < len){


        // Eat leading whitespace

        i = in.find_first_not_of(delimiters, i);


        if (i == string::npos)

            return;   // Nothing left but white space


        // Find the end of the token

        string::size_type j = in.find_first_of(delimiters, i);


        // Push token

        if (j == string::npos){


            container.push_back(in.substr(i));

            return;


        }else


            container.push_back(in.substr(i, j-i));


        // Set up for next loop

        i = j + 1;

    }

}

void CameraBasler::grabOne(){

    Mat img;

    if(camera->getPixelFormat() == 8){

        img = Mat(camera->getHeight(), camera->getWidth(), CV_8UC1);

    }else if(camera->getPixelFormat() == 12){

        img = Mat(camera->getHeight(), camera->getWidth(), CV_16UC1);

    }

    // Intialize exposition and gain
    camera->setExposureTime((double)initialExpValue);
    camera->setGain(initialGainValue);
    camera->setPixelFormat(format);

    camera->acqStart();


    namedWindow( "Grabbed frame", WINDOW_AUTOSIZE );// Create a window for display.

    Frame *newFrame;

    if(camera->grabImage(newFrame, img )){

        Frame f;

        bool r = f.copyFrame(newFrame);

        if(camera->getPixelFormat() == 8){

            //SaveImg::saveBMP(f.getImg(),"/home/fripon/testcap");

            if(saveFits){

                Fits2D newFits(savePath,fitsHeader);
                newFits.setGaindb(initialGainValue);
                //newFits.setDateobs("");//dateObs
                newFits.setSaturate(255);
                newFits.setRadesys("ICRS");
                newFits.setEquinox(2000.0);
                newFits.setCtype1("RA---ARC");
                newFits.setCtype2("DEC---ARC");
                newFits.setExposure(initialExpValue * 1e-6);
                //newFits.setElaptime(0);
                //newFits.setCrval1(0);//sideraltime
                newFits.writeFits(f.getImg(), UC8, 0, true );

            }

            if(saveImg){

                SaveImg::saveBMP(f.getImg(),savePath + "capture");

            }

            imshow( "Grabbed frame", f.getImg() );                   // Show our image inside it.

        //conversion to 8UC
        }else if(camera->getPixelFormat() == 12){

            Mat temp = f.getImg();
            Mat res;
            /*Mat res = */Conversion::convertTo8UC1(temp).copyTo(res);

            if(saveFits){

                Fits2D newFits(savePath,fitsHeader);
                newFits.setGaindb(initialGainValue);
                //newFits.setDateobs("");//dateObs
                newFits.setSaturate(4095);
                newFits.setRadesys("ICRS");
                newFits.setEquinox(2000.0);
                newFits.setCtype1("RA---ARC");
                newFits.setCtype2("DEC---ARC");
                newFits.setExposure(initialExpValue * 1e-6);
                //newFits.setElaptime(0);
                //newFits.setCrval1(0);//sideraltime
                newFits.writeFits(f.getImg(), US16, 0, true );

            }

            if(saveImg){

                SaveImg::saveBMP(res,savePath + "capture");

            }

            imshow( "Grabbed frame", res );

        }else
            cout << "Format Unknow"<<endl;

        waitKey(0);

    }else{

        cout << "Grab failed" << endl;

    }

    camera->acqStop();

    camera->grabStop();

}

// Thread function
void    CameraBasler::operator()(){

	bool stop;

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "acqThread");
	BOOST_LOG_SEV(log, critical) << "\n";
	BOOST_LOG_SEV(log,notification) << "-- Acquisition thread start. ---";

    // Used to construct a frame
    Mat img;

    if(camera->getPixelFormat() == 8){
        BOOST_LOG_SEV(log,notification) << "Pixel format is 8 bits";
        img = Mat(camera->getHeight(), camera->getWidth(), CV_8UC1);

    }else if(camera->getPixelFormat() == 12){
        BOOST_LOG_SEV(log,notification) << "Pixel format is 12 bits";
        img = Mat(camera->getHeight(), camera->getWidth(), CV_16UC1);

    }

    // Intialize exposition
    BOOST_LOG_SEV(log,notification) << "Set exposure time value to : " << (double)initialExpValue;
    camera->setExposureTime((double)initialExpValue);

    // Intialize gain
    BOOST_LOG_SEV(log,notification) << "Set gain value to : " << initialGainValue;
    camera->setGain(initialGainValue);
  //   camera->setPixelFormat(8);
    camera->acqStart();

    int framebufferActualSize = 0;

    double tMSMean =0.0;
    double tLSMean=0.0;
    double tBMPMean=0.0;
    int div = 1;

    bool saveISS = false;
    string rep = "memoryTest/";//"ISS_20140925_070111";
    //ISS_20140923_052830

    int compteur = 0;

    vector<Mat> listForFits3D;
    bool savef3D = true;


    //Thread loop
    do{

        Frame *newFrame;
       // Mat img;

        double tacq = (double)getTickCount();

        if(camera->grabImage(newFrame, img )){

            Frame f;

            if(maskEnable){

                bool r = f.copyFrame(newFrame/*, mask*/);

            }else{

                bool r = f.copyFrame(newFrame);

            }

            camera->getWidth();

            if(saveISS && compteur < 9000){

                if(camera->getPixelFormat() != 8){

                     Fits2D newFits("/home/fripon/memoryTest/fitsTest_"+Conversion::intToString(compteur),fitsHeader);
                    newFits.writeFits(newFrame->getImg(), US16, 0, false );
/*
                        Fits2D fit("/home/fripon/data/" + rep + "/MOON_"+Conversion::intToString(cpt), 0, "", 0, 30, 4095, 33333.0, 400, 0.0 );
                        fit.loadKeywordsFromConfigFile("/home/fripon/friponProject/friponCapture/configuration.cfg");
                        fit.writeimage(f.getImg(), 16, "" , false);
                        Mat temp1;
                       /* f.getImg().copyTo(temp1);

                        SaveImg::saveBMP( Conversion::convertTo8UC1(temp1),"/home/fripon/data/" + rep +  "/ISS_BMP_" + Conversion::intToString(cpt) );
*/
                }else{



                     Fits2D newFits("/home/fripon/data2/" + rep +  "/MOON_"+Conversion::intToString(compteur),fitsHeader);
                    newFits.writeFits(newFrame->getImg(), UC8, 0, false );

                      /*  Fits2D fit("/home/fripon/data/" + rep +  "/MOON_"+Conversion::intToString(cpt), 0, "", 0, 30, 255, 33333.0, 850, 0.0 );
                        fit.loadKeywordsFromConfigFile("/home/fripon/friponProject/friponCapture/configuration.cfg");
                        fit.writeimage(f.getImg(), 8, "", false );
                        Mat temp1;
                         f.getImg().copyTo(temp1);*/
                        //temp1cd.convertTo(temp1, CV_8UC1, 255/4095, 0);
                      // SaveImg::saveBMP(temp1,"/home/fripon/data/" + rep +  "/ISS_BMP_" + Conversion::intToString(cpt) );
                }

                compteur++;
            }




           /* Fits fitsHeader;
            fitsHeader.loadKeywordsFromConfigFile("/home/fripon/friponProject/friponCapture/configuration.cfg");

            Fits2D newFits("/home/fripon/data2/",fitsHeader);
            newFits.setOntime(0); // ontime/fps
            newFits.setGaindb(850);
            newFits.setObsmode(30);//fps
            newFits.setDateobs("");//dateObs
            newFits.setSaturate(255);
            newFits.setRadesys("ICRS");
            newFits.setEquinox(2000.0);
            newFits.setCtype1("RA---ARC");
            newFits.setCtype2("DEC---ARC");
            newFits.setExposure(33333 * 1e-6);
            newFits.setElaptime(0);
            newFits.setCrval1(0);//sideraltime

            if(newFits.writeimage(f.getImg(), 8, Conversion::intToString(compteur), true ))
            cout << "saved"<<endl;
            else
            cout << "not saved"<<endl;

            compteur++;
*/

            /*if(listForFits3D.size() < 80){


                listForFits3D.push_back(f.getImg());



            }else{



                    Fits3D newF(listForFits3D.size(), f.getImg().rows,f.getImg().cols, &listForFits3D);

                    if(newF.writeFits3D_UC("/home/fripon/saveFits3D_"+Conversion::intToString(compteur))){
                        cout << "fits 3D saved" <<endl;

                    }
                    else
                        cout << "fits 3D not saved" <<endl;


                    compteur ++;
                    listForFits3D.clear();


            }*/



            f.setNumFrame(cpt);

            boost::mutex::scoped_lock lock(*mutexQueue);

            framesQueue->pushInFifo(f);

            if(framesQueue->getFifoIsFull()) condQueueFill->notify_all();

            framesQueue->setThreadRead("imgCap", true);
            framesQueue->setThreadRead("astCap", true);
            framesQueue->setThreadRead("det", true);
            condQueueNewElement->notify_all();

            lock.unlock();

            cpt++;

            cout<< "--------------- FRAME n°" << cpt << " ----------------- " << endl;
            BOOST_LOG_SEV(log,notification) << "Grabbing frame n° " << cpt;

            delete newFrame;

        }else{

            BOOST_LOG_SEV(log,notification) << "Grabbing failed , frame lost";

        }

        tacq = (((double)getTickCount() - tacq)/getTickFrequency())*1000;
        cout << " [ ACQ ] : " << tacq << " ms" << endl;
      //  cout << "in "<<endl;

      // BOOST_LOG_SEV(log, notification) << "Thread joinable : " << m_thread->joinable();
        // Get the "must stop" state (thread-safe)
        mustStopMutex.lock();
        stop = mustStop;
        mustStopMutex.unlock();

    }while(stop == false);

    camera->acqStop();

    camera->grabStop();

    cout                                << "--- Acquisition thread terminated. ---" << endl;
    BOOST_LOG_SEV(log, notification)    << "--- Acquisition thread terminated. ---";

}
