/*
								Camera.cpp

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
 * \file    Camera.cpp
 * \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * \version 1.0
 * \date    13/06/2014
 * \brief   Acquisition thread.
 */

#include "Camera.h"

#ifdef USE_PYLON
    #include "CameraSDKPylon.h"
#else
    #include "CameraSDKAravis.h"
#endif

Camera::Camera(CamType camType){

    m_thread    = NULL;
    frameCpt    = 0;
    cameraType  = camType;

    switch(camType){

        case BASLER :

            {

                #ifdef USE_PYLON
                    camera = new CameraSDKPylon();
                #else
                    camera = new CameraSDKAravis();
                #endif

                cout << "camera basler" << endl;

            }

            break;

        case DMK :

            {

                camera = new CameraSDKAravis();

            }

            break;

        case FRAMES :

                camera = NULL;

            break;

        case VIDEO :

                camera = NULL;

            break;

    }
}

Camera::Camera( CamType     camType,
                int         camExp,
                int         camGain,
                CamBitDepth camDepth){

    m_thread	= NULL;
    cameraType  = camType;
    exposure    = camExp;
    gain        = camGain;
    bitdepth    = camDepth;
    frameCpt    = 0;

    switch(camType){

        case BASLER :

            {

                #ifdef USE_PYLON
                    camera = new CameraSDKPylon();
                #else
                    camera = new CameraSDKAravis();
                #endif

            }

            break;

        case DMK :

            {

                camera = new CameraSDKAravis();

            }

            break;

        case FRAMES :

                camera = NULL;

            break;

        case VIDEO :

                camera = NULL;

            break;

    }
}

Camera::Camera( CamType                                 camType,
                int                                     camExp,
                int                                     camGain,
                CamBitDepth                             camDepth,
                int                                     camFPS,
                int                                     imgToSum,
                int                                     imgToWait,
                bool                                    imgStack,
                boost::circular_buffer<Frame>           *cb,
                boost::mutex                            *m_cb,
                boost::condition_variable               *c_newElemCb,
                boost::circular_buffer<StackedFrames>   *stackedFb,
                boost::mutex                            *m_stackedFb,
                boost::condition_variable               *c_newElemStackedFb,
                bool                                    *newFrameForDet,
                boost::mutex                            *m_newFrameForDet,
                boost::condition_variable               *c_newFrameForDet){

    m_thread			            = NULL;
    mustStop			            = false;

    cameraType                      = camType;
    exposure                        = camExp;
    gain                            = camGain;
    bitdepth                        = camDepth;
    fps                             = camFPS;

    frameToSum                      = imgToSum;
    frameToWait                     = imgToWait;
    stackEnabled                    = imgStack;

    frameBuffer                     = cb;
    m_frameBuffer                   = m_cb;
    c_newElemFrameBuffer            = c_newElemCb;

    stackedFramesBuffer             = stackedFb;
    m_stackedFramesBuffer           = m_stackedFb;
    c_newElemStackedFramesBuffer    = c_newElemStackedFb;

    newFrameDet                     = newFrameForDet;
    m_newFrameDet                   = m_newFrameForDet;
    c_newFrameDet                   = c_newFrameForDet;

    frameCpt                        = 0;

    switch(camType){

        case BASLER :

            {

                #ifdef USE_PYLON
                    camera = new CameraSDKPylon();
                #else
                    camera = new CameraSDKAravis();
                #endif

            }

            break;

        case DMK :

            {

                camera = new CameraSDKAravis();

            }

            break;

        case FRAMES :

                camera = NULL;

            break;

        case VIDEO :

                camera = NULL;

            break;

    }
}

Camera::~Camera(void){

    if(camera != NULL)
        delete camera;

	if(m_thread != NULL)
        delete m_thread;
}

void Camera::join(){

	m_thread->join();

}

bool Camera::setSelectedDevice(int id, string name){

    return camera->chooseDevice(id, name);

}

bool Camera::setSelectedDevice(string name){

    return camera->chooseDevice(name);

}

void Camera::getListCameras(){

    camera->listCameras();

}

void Camera::stopThread(){

	// Signal the thread to stop (thread-safe)
	mustStopMutex.lock();
	mustStop = true;
	mustStopMutex.unlock();

	// Wait for the thread to finish.
	if (m_thread!=NULL){

        m_thread->join();

	}
}

void Camera::startThread(){

    camera->grabStart();

    m_thread = new boost::thread(boost::ref(*this));

}

bool Camera::startGrab(){

    return camera->grabStart();

}

bool Camera::getDeviceById(int id, string &device){

    return camera->getDeviceById(id, device);

}

void Camera::stopGrab(){

}

int Camera::getCameraHeight(){

    return camera->getHeight();

}

int Camera::getCameraWidth(){

    return camera->getWidth();

}

double Camera::getCameraExpoMin(){

    return camera->getExpoMin();

}

bool Camera::setCameraExposureTime(double exposition){

    return camera->setExposureTime(exposition);

}

bool Camera::setCameraGain(int gain){

    return camera->setGain(gain);

}

bool Camera::setCameraFPS(int fps){

    return camera->setFPS(fps);

}

bool Camera::setCameraPixelFormat(CamBitDepth depth){

    return camera->setPixelFormat(depth);

}

bool Camera::grabSingleFrame(Mat &frame, string &date){

    Frame newFrame;

    // Start single acquisition.
    camera->acqStart(false);

    // Grab a frame.
    if(camera->grabImage(newFrame)){

        date = newFrame.getAcqDate();

        newFrame.getImg().copyTo(frame);

        camera->acqStop();

        return true;

    }else{

        camera->acqStop();

        return false;

    }
}

void Camera::operator()(){

	bool stop;

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "ACQ_THREAD");
	BOOST_LOG_SEV(log,notification) << "\n";
	BOOST_LOG_SEV(log,notification) << "==============================================";
	BOOST_LOG_SEV(log,notification) << "========== Start acquisition thread ==========";
	BOOST_LOG_SEV(log,notification) << "==============================================";

    camera->acqStart(true);

    Mat stackImg    = Mat::zeros(camera->getHeight(), camera->getWidth(), CV_32FC1);
    Mat currImg     = Mat::zeros(camera->getHeight(), camera->getWidth(), CV_32FC1);

    int nbFrameSummed = 0;
    int nbFrameWaited = 0;

    bool waitToStack = true;

    string firstDate;

    int acqGain = 0;
    int acqExp = 0;

    do{

        Frame newFrame;

        double tacq = (double)getTickCount();

        if(camera->grabImage(newFrame)){

            newFrame.setNumFrame(frameCpt);

            boost::mutex::scoped_lock lock(*m_frameBuffer);
            frameBuffer->push_back(newFrame);

            lock.unlock();

           /* Fits ff;
            Fits2D newFits("/home/fripon/data2/framefits_" + Conversion::intToString(frameCpt) + "_",ff);
            newFits.writeFits(newFrame.getImg(), US16, 0, true,"" );*/

            boost::mutex::scoped_lock lock2(*m_newFrameDet);
            *newFrameDet = true;
            c_newFrameDet->notify_one();
            lock2.unlock();

            if(stackEnabled){

                // Wait some times before to stack n frames.
                if(waitToStack){

                    if(nbFrameWaited < frameToWait){

                        cout << "WAIT : " << nbFrameWaited << " / " << frameToWait  << endl;
                        nbFrameWaited++;

                    }else{

                        nbFrameWaited = 0;
                        waitToStack = false;

                    }

                }

                // Stack n frames before to wait.
                if(!waitToStack){

                    // Get infos about the first frame pushed in the stack.
                    if(nbFrameSummed == 0){

                        firstDate   = newFrame.getRawDate();
                        acqGain     = newFrame.getGain();       // Gain used to grab the first frame.
                        acqExp      = newFrame.getExposure();   // Exposure used to grab the first frame.

                    }

                    // While stack is not full, accumulate frames.
                    if(nbFrameSummed < frameToSum){

                        cout << "STACK : " << nbFrameSummed << " / " << frameToSum  << endl;

                        double t = (double)getTickCount();
                        newFrame.getImg().convertTo(currImg, CV_32FC1);
                        accumulate(currImg,stackImg);
                        t = (((double)getTickCount() - t)/getTickFrequency())*1000;
                        //cout << "> stack Time : " << t << endl;
                        nbFrameSummed++;

                    }else{

                        nbFrameSummed = 0;
                        waitToStack = true;

                    }

                    // N frames have been stacked, save the result.
                    if(nbFrameSummed == frameToSum){

                        BOOST_LOG_SEV(log,normal) << "Stack of " << frameToSum << " frames finished.";

                        string lastDate = newFrame.getRawDate();

                        StackedFrames sum = StackedFrames(firstDate, lastDate, acqGain, acqExp, stackImg, nbFrameSummed);

                        boost::mutex::scoped_lock lock(*m_stackedFramesBuffer);
                        stackedFramesBuffer->push_back(sum);
                        c_newElemStackedFramesBuffer->notify_all();
                        BOOST_LOG_SEV(log,notification) << "Notification sent to AST_THREAD.";
                        lock.unlock();

                        stackImg  = Mat::zeros(camera->getHeight(), camera->getWidth(), CV_32FC1);

                    }
                }
            }

            frameCpt++;

            cout << "============= FRAME n°" << frameCpt << " ============= " << endl;

        }else{

            cout << "Failed to grab frame " << frameCpt + 1 << endl;

        }

        tacq = (((double)getTickCount() - tacq)/getTickFrequency())*1000;
        cout << " [ ACQ Time ] : " << tacq << " ms" << endl;

        mustStopMutex.lock();
        stop = mustStop;
        mustStopMutex.unlock();

    }while(stop == false);

    camera->acqStop();


    BOOST_LOG_SEV(log, notification) << "--- Acquisition thread terminated. ---";

}
