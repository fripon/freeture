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

     frameCpt = 0;

}

CameraBasler::CameraBasler( int         camExp,
                            int         camGain,
                            CamBitDepth camDepth){

    m_thread	= NULL;
    exposure    = camExp;
    gain        = camGain;
    bitdepth    = camDepth;

    #ifdef USE_PYLON
        camera = new CameraSDKPylon();
    #else
        camera = new CameraSDKAravis();
    #endif

    frameCpt = 0;

}

CameraBasler::CameraBasler( Fifo<Frame> *queueRam,
                            boost::mutex *m_queue,
                            boost::condition_variable *c_queueFull,
                            boost::condition_variable *c_queueNew,
                            int         camExp,
                            int         camGain,
                            CamBitDepth camDepth,
                            int         camFPS){

    m_thread			= NULL;
    framesQueue			= queueRam;
    mutexQueue			= m_queue;
    mustStop			= false;
    condQueueFill		= c_queueFull;
    condQueueNewElement	= c_queueNew;

    threadStopped       = false;

    exposure            = camExp;
    gain                = camGain;
    bitdepth            = camDepth;
    fps                 = camFPS;

    frameCpt = 0;

    #ifdef USE_PYLON
        camera = new CameraSDKPylon();
    #else
        camera = new CameraSDKAravis();
    #endif

}

CameraBasler::~CameraBasler(void){

    if(camera != NULL)
        delete camera;

	if(m_thread != NULL)
        delete m_thread;
}

void    CameraBasler::join(){

	m_thread->join();

}

bool    CameraBasler::setSelectedDevice(int id, string name){

    return camera->chooseDevice(id, name);

}

bool    CameraBasler::setSelectedDevice(string name){

    return camera->chooseDevice(name);

}

void    CameraBasler::getListCameras(){

    camera->listCameras();

}

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

}

bool    CameraBasler::startGrab(){

    return camera->grabStart();

}

bool    CameraBasler::getDeviceById(int id, string &device){

    return camera->getDeviceById(id, device);

}

void    CameraBasler::stopGrab(){



}

int     CameraBasler::getCameraHeight(){

    return camera->getHeight();

}

int     CameraBasler::getCameraWidth(){

    return camera->getWidth();

}

double	CameraBasler::getCameraExpoMin(){

    return camera->getExpoMin();

}

bool    CameraBasler::setCameraExposureTime(double exposition){

    return camera->setExposureTime(exposition);

}

bool    CameraBasler::setCameraGain(int gain){

     return camera->setGain(gain);

}

bool    CameraBasler::setCameraFPS(int fps){

     return camera->setFPS(fps);

}

bool    CameraBasler::setCameraPixelFormat(CamBitDepth depth){

    return camera->setPixelFormat(depth);

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

bool CameraBasler::grabSingleFrame(Mat &frame, string &date){

    Mat img;
    Frame *newFrame;

    switch(bitdepth){

        case MONO_8 :

            img = Mat(camera->getHeight(), camera->getWidth(), CV_8UC1);

            break;

        case MONO_12 :

            img = Mat(camera->getHeight(), camera->getWidth(), CV_16UC1);

            break;

    }

    camera->acqStart();

    if(camera->grabImage(newFrame, img)){

        date = newFrame->getAcqDate();
        img.copyTo(frame);

        camera->acqStop();
        camera->grabStop();
        return true;

    }else{

        camera->acqStop();
        camera->grabStop();
        return false;

    }
}

void    CameraBasler::operator()(){

	bool stop;

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "ACQ_THREAD");

	BOOST_LOG_SEV(log, critical)    << "\n";
	BOOST_LOG_SEV(log,notification) << "-- Acquisition thread start. ---";

    Mat img;

    switch(camera->getPixelFormat()){

       case MONO_8 :

            {

            img = Mat(camera->getHeight(), camera->getWidth(), CV_8UC1);

            }

            break;

       case MONO_12 :

            {

            img = Mat(camera->getHeight(), camera->getWidth(), CV_16UC1);

            }

            break;

    }

    camera->acqStart();

    int compteur = 0;

    vector<Mat> listForFits3D;

    //Thread loop
    do{

        Frame *newFrame;

        double tacq = (double)getTickCount();

        if(camera->grabImage(newFrame, img)){

            Frame f;

            bool r = f.copyFrame(newFrame);

            camera->getWidth();

            f.setNumFrame(frameCpt);

            boost::mutex::scoped_lock lock(*mutexQueue);

            framesQueue->pushInFifo(f);

           /* double t = (double)getTickCount();

            Fits fitsheader;
            fitsheader.setDateobs("2014-10-02T17:15:03");
            Fits2D newfits("/home/fripon/freeture_frames/", fitsheader);
            newfits.writeFits(img, US16, 0, false, "frame_" + Conversion::intToString(frameCpt));

            t = (((double)getTickCount() - t)/getTickFrequency())*1000;
            cout << "> Fits Time : " << t << endl;*/

            if(framesQueue->getFifoIsFull()) condQueueFill->notify_all();

            framesQueue->setThreadRead("imgCap", true);
            framesQueue->setThreadRead("astCap", true);
            framesQueue->setThreadRead("det", true);
            condQueueNewElement->notify_all();

            lock.unlock();

            frameCpt++;

            cout                            << "--------------- FRAME n°" << frameCpt << " ----------------- " << endl;
            BOOST_LOG_SEV(log,notification) << "--------------- FRAME n°" << frameCpt << " ----------------- ";

            delete newFrame;

        }else{

            BOOST_LOG_SEV(log,notification) << "Grabbing failed , frame lost";

        }

        tacq = (((double)getTickCount() - tacq)/getTickFrequency())*1000;

        cout << " [ ACQ ] : " << tacq << " ms" << endl;

        mustStopMutex.lock();
        stop = mustStop;
        mustStopMutex.unlock();

    }while(stop == false);

    camera->acqStop();
    camera->grabStop();

    BOOST_LOG_SEV(log, notification) << "--- Acquisition thread terminated. ---";

}
