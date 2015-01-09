/*
								CameraAravis.cpp

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
*	Last modified:		22/12/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
 * @file    CameraAravis.cpp
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    22/12/2014
 */

#include "CameraDMK.h"
#include "CameraSDKAravis.h"

CameraDMK::CameraDMK(){

    m_thread    = NULL;

    // Use Aravis Library for DMK cameras.
    camera      = new CameraSDKAravis();

}

CameraDMK::CameraDMK(   Fifo<Frame> *queue,
                        boost::mutex *m_mutex_queue,
                        boost::condition_variable *m_cond_queue_fill,
                        boost::condition_variable *m_cond_queue_new_element,
                        boost::mutex *m_mutex_thread_terminated, bool *stop){

	m_thread			= NULL;
	framesQueue			= queue;
	mutexQueue			= m_mutex_queue;
	mustStopMutex       = m_mutex_thread_terminated;
	mustStop			= stop;
	condQueueFill		= m_cond_queue_fill;
	condQueueNewElement	= m_cond_queue_new_element;

     // Use Aravis Library for DMK cameras.
	camera              = new CameraSDKAravis();

	frameCpt = 0;

}

CameraDMK::CameraDMK(   int           camExp,
                        int           camGain,
                        CamBitDepth   camDepth){

    m_thread			= NULL;
    bitpix  =   camDepth;

    camera              = new CameraSDKAravis();

    frameCpt = 0;

}

CameraDMK::~CameraDMK(){


    if (camera!=NULL) delete camera;

	if (m_thread!=NULL) delete m_thread;

}

/// THREAD FUNCTIONS

void    CameraDMK::join(){

	m_thread->join();

}

bool    CameraDMK::startGrab(){

    return camera->grabStart();

}

void    CameraDMK::startThread(){

    camera->grabStart();

	// Launch acquisition thread
	m_thread = new boost::thread(boost::ref(*this));
    cout << "t"<<endl;

}

void    CameraDMK::stopThread(){

	// Signal the thread to stop (thread-safe)
	mustStopMutex->lock();
	*mustStop = true;
	mustStopMutex->unlock();



    cout << "DMK Thread must stop" <<endl;


	// Wait for the thread to finish.
	if (m_thread!=NULL) m_thread->join();

	cout << "DMK Thread stopped"<<endl;

}

void    CameraDMK::operator()(){

    //https://github.com/xamox/aravis/blob/master/tests/arvheartbeattest.c
    //http://blogs.gnome.org/emmanuel/2010/04/03/chose-promise-chose-due/
    //https://code.google.com/p/tiscamera/source/browse/gige/aravis/examples/c/gstexample.c?spec=svn0c121ab10f32440f10bdad3ed7264fbf1e6b55f9&r=0c121ab10f32440f10bdad3ed7264fbf1e6b55f9
    //https://gitorious.org/rock-drivers/camera-aravis/source/5d1b8f749670e0348b9b2e2d760204ed71c73010:src/recorder.cpp#L106

    bool stop = false;

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "acqThread");
	BOOST_LOG_SEV(log, notification) << "\n";
	BOOST_LOG_SEV(log, notification) << "Acquisition thread started.";
    //camera->grabStart();
    camera->acqStart();

    int c = 0;
    do {

        Frame *newFrame;
        Mat img;

        double tacq = (double)getTickCount();

        if(camera->grabImage(newFrame, img)){

            boost::mutex::scoped_lock lock(*mutexQueue);

            Frame f;
            bool r = f.copyFrame(newFrame);

            f.setNumFrame(frameCpt);

            framesQueue->pushInFifo(f);

            lock.unlock();
            //cout << "save : " << "testDMK-"+Conversion::intToString(c) <<endl;
            //SaveImg::saveBMP(f.getImg(),"testDMK-"+Conversion::intToString(c));
            //c++;

            if(framesQueue->getFifoIsFull())
                condQueueFill->notify_all();

            framesQueue->setThreadRead("imgCap", true);
            framesQueue->setThreadRead("astCap", true);
            framesQueue->setThreadRead("det", true);

            condQueueNewElement->notify_all();

            frameCpt++;

            cout                            << "--------------- FRAME n°" << frameCpt << " ----------------- " << endl;
            BOOST_LOG_SEV(log,notification) << "--------------- FRAME n°" << frameCpt << " ----------------- ";

            tacq = (((double)getTickCount() - tacq)/getTickFrequency())*1000;

            cout << " [ ACQ ] : " << tacq << " ms" << endl;

            delete newFrame;

        }



        // Get the "must stop" state (thread-safe)
        mustStopMutex->lock();

        stop = *mustStop;

        mustStopMutex->unlock();

    }while (stop == false);

    camera->acqStop();

    camera->grabStop();

    BOOST_LOG_SEV(log, notification) <<"Acquisition thread terminated";

}

/// ACQUISITION FUNCTIONS

bool	CameraDMK::setSelectedDevice(string name){

    return camera->chooseDevice(name);

}

bool    CameraDMK::grabSingleFrame(Mat &frame, string &date){

    Mat img;
    Frame *newFrame;

    switch(bitpix){

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

/// GETTER FUNCTIONS

int		CameraDMK::getCameraWidth(){

    return camera->getWidth();

}

int		CameraDMK::getCameraHeight(){

    return camera->getHeight();

}

void	CameraDMK::getListCameras(){

    camera->listCameras();

}

bool    CameraDMK::getDeviceById(int id, string &device){

    return camera->getDeviceById(id, device);

}

/// SETTER FUNCTIONS

bool    CameraDMK::setCameraFPS(int fps){

    return camera->setFPS(fps);

}


bool	CameraDMK::setCameraPixelFormat(CamBitDepth depth){

    return camera->setPixelFormat(depth);

}

bool	CameraDMK::setCameraExposureTime(double value){

    return camera->setExposureTime(value);

}

bool	CameraDMK::setCameraGain(int value){

    return camera->setGain(value);

}
