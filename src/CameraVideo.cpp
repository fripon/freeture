/*
								CameraVideo.cpp

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
 * @file    CameraVideo.cpp
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    13/06/2014
 */

#include "CameraVideo.h"


CameraVideo::CameraVideo(   string video_path,
                            Fifo<Frame> *queue,
                            boost::mutex *m_mutex_queue,
                            boost::condition_variable *m_cond_queue_fill,
                            boost::condition_variable *m_cond_queue_new_element){

	videoPath       = video_path;
	thread          = NULL;
	frameQueue      = queue;
	//terminatedThread = threadTerminated;
	mutexQueue			= m_mutex_queue;
	condQueueFill			= m_cond_queue_fill;
	condQueueNewElement	= m_cond_queue_new_element;

	//open the video file for reading
    cap = VideoCapture(videoPath);

	imgH = 0;
	imgW = 0;

}

CameraVideo::~CameraVideo(void){

}

void CameraVideo::startThread(){

	// Launch acquisition thread
	thread=new boost::thread(boost::ref(*this));

}

void CameraVideo::join(){

    cout << "waiting"<<endl;
	thread->join();

}


template <typename Container> void stringtok(Container &container, string const &in, const char * const delimiters = "_"){


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

void CameraVideo::operator () (){

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "acqThread");

	BOOST_LOG_SEV(log, critical) << "\n";

	BOOST_LOG_SEV(log,notification) << "Acquisition thread started.";

	int framebufferActualSize = 0;

	int fois = 0;

	//if not success, exit program
    if ( !cap.isOpened() ){

		 BOOST_LOG_SEV(log,fail) << "Cannot open the video file";

    }else{

		BOOST_LOG_SEV(log,notification) << "Open the video file for reading succeed";
		BOOST_LOG_SEV(log,normal) << "FPS : " << cap.get(CV_CAP_PROP_FPS);

		imgH = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
		BOOST_LOG_SEV(log,normal) << "Reading frame height : " << imgH;

		imgW = cap.get(CV_CAP_PROP_FRAME_WIDTH);
		BOOST_LOG_SEV(log,normal) << "Reading frame width : " << imgW;

		cout << "nb TOTAL: "<< cap.get(CV_CAP_PROP_FRAME_COUNT)<<endl;

		bool bSuccess = true;


        Size frameSize(static_cast<int>(imgW), static_cast<int>(imgH));

        VideoWriter oVideoWriter("/home/fripon/videoMet4.avi", CV_FOURCC('D', 'I', 'V', 'X'), 30, frameSize, true); //initialize the VideoWriter object

        int ex = static_cast<int>(cap.get(CV_CAP_PROP_FOURCC));

        char EXT[] = {ex & 0XFF , (ex & 0XFF00) >> 8,(ex & 0XFF0000) >> 16,(ex & 0XFF000000) >> 24, 0};


        cout << EXT << endl;


		do{


			Mat frame;
			Mat copyframe ;//= Mat::zeros();

            double tacq = (double)getTickCount();

			//Read a new video frame
			bSuccess = cap.read(frame);

			if(bSuccess){

                double numFrame = cap.get(CV_CAP_PROP_POS_FRAMES);

                BOOST_LOG_SEV(log,normal) << "Reading frame n° " << numFrame;

                cout << endl << endl << "-------- FRAME n°"<< numFrame << "--------" << endl;

                double tacq = (double)getTickCount();

                cout << "FROMAT : " << cap.get(CV_CAP_PROP_FORMAT)<<endl;

                //BGR (3 channels) to G (1 channel)
                cvtColor(frame, frame, CV_BGR2GRAY);

                 cvtColor(frame, copyframe, CV_GRAY2BGR);
                //Timestamping
                string acquisitionDate = TimeDate::localDateTime(second_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");

                Frame newFrameRAM( frame, 0, 0, acquisitionDate );

                newFrameRAM.setNumFrame(numFrame);

                newFrameRAM.setFrameRemaining(cap.get(CV_CAP_PROP_FRAME_COUNT) - cap.get(CV_CAP_PROP_POS_FRAMES));

                boost::mutex::scoped_lock lock(*mutexQueue);

                frameQueue->pushInFifo(newFrameRAM);


/*
                line( copyframe, Point( 392, 0),
                       Point( 392, imgH -1 ),
                       Scalar( 0,0,255), 1, 4, 0  );

                line( copyframe, Point( 125, 0),
                       Point( 125, imgH -1 ),
                       Scalar( 0,0,255), 1, 4, 0  );

                SaveImg::saveBMP(copyframe,"/home/fripon/testColor_"+Conversion::intToString(numFrame));
*/
                if(oVideoWriter.isOpened()){

                    oVideoWriter << copyframe;

                }else{

                cout << "not opend"<<endl;
                }


                lock.unlock();



                if(frameQueue->getFifoIsFull()) condQueueFill->notify_all();

                frameQueue->setThreadRead("imgCap", true);
                frameQueue->setThreadRead("astCap", true);
                frameQueue->setThreadRead("det", true);

                condQueueNewElement->notify_all();

                tacq = (((double)getTickCount() - tacq)/getTickFrequency())*1000;
                cout << " [-ACQ-]    Time : " << tacq << " ms " << endl;

                //Wait in ms
                waitKey(100) ;

            }else{

                cout << "Can't read the video or end to read the video"<<endl;
                break;
            }

		}while(bSuccess);

		BOOST_LOG_SEV(log,notification) << "Video frame read process terminated.";

	}

	BOOST_LOG_SEV(log,notification) << "Acquisition thread terminated.";

}

int CameraVideo::getCameraHeight(){

    int h = 0;


    if ( cap.isOpened() ){
        h = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    }

	return h;

}

int CameraVideo::getCameraWidth(){

	int w = 0;

    if ( cap.isOpened() ){
        w = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    }

	return w;

}
