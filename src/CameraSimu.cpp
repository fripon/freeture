/*
								CameraSimu.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau
*                               FRIPON-GEOPS-UPSUD-CNRS
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
 * @file    CameraSimu.cpp
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    13/06/2014
 */

#include "CameraSimu.h"

CameraSimu::CameraSimu(    Fifo<Frame> *queue,
						   boost::mutex *m_mutex_queue,
						   boost::condition_variable *m_cond_queue_fill,
						   boost::condition_variable *m_cond_queue_new_element){

	m_thread			= NULL;
	framesQueue			= queue;
	mutexQueue			= m_mutex_queue;
	mustStop			= false;
	condQueueFill		= m_cond_queue_fill;
	condQueueNewElement	= m_cond_queue_new_element;

	camFPS = 30.0;
	camSizeH = 960;
	camSizeW = 1280;

    Ax = 50;
    Ay = 50;
    Bx = 1200;
    By = 900;
}

CameraSimu::~CameraSimu(void){

	if (m_thread!=NULL) delete m_thread;

}

void CameraSimu::join(){

	m_thread->join();

}

int CameraSimu::grabStart(){
	// Launch acquisition thread
	m_thread = new boost::thread(boost::ref(*this));
	return 0;
}

void CameraSimu::grabStop(){


};

int CameraSimu::getHeight(){

	return camSizeH;

}

int CameraSimu::getWidth(){

	return camSizeW;

}

double CameraSimu::getFPS(){

	return camFPS;

}

// Stop the thread
void CameraSimu::stopThread(){

	// Signal the thread to stop (thread-safe)
	mustStopMutex.lock();
	mustStop = true;
	mustStopMutex.unlock();


	// Wait for the thread to finish.
	if (m_thread!=NULL) m_thread->join();

	cout << "SIMU Thread stopped"<<endl;

}

// Thread function
void CameraSimu::operator()(){

	bool stop = false;

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "acqThread");
	BOOST_LOG_SEV(log, critical) << "\n";
	BOOST_LOG_SEV(log,notification) << "Acquisition thread started.";

	namedWindow("MyWindow", CV_WINDOW_AUTOSIZE);

    do{

        double tt = (double)getTickCount();

        if(Ax + 5 < camSizeW && Ay + 5 < camSizeH){

            Mat frame = Mat::zeros(camSizeH, camSizeW, CV_8UC1);

            frame.at<uchar>(Ay,Ax) = 255;
            frame.at<uchar>(Ay+1,Ax+1) = 255;
            frame.at<uchar>(Ay+2,Ax+2) = 255;
            frame.at<uchar>(Ay+1,Ax+2) = 255;
            frame.at<uchar>(Ay+2,Ax+1) = 255;

            Ax += 5;
            Ay += 5;

            //imshow("MyWindow", frame);
            waitKey(27);

            tt = (((double)getTickCount() - tt)/getTickFrequency())*1000;

            cout << "time : " <<std::setprecision(5)<< std::fixed<< tt << " ms"<< endl;

            // Get the "must stop" state (thread-safe)
            mustStopMutex.lock();
            stop = mustStop;
            mustStopMutex.unlock();

        }else{
            stop = true;
            cout << "END" <<endl;
        }

    }while(stop==false);


}
