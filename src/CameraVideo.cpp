/*
								CameraVideo.cpp

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
* \file    CameraVideo.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    13/06/2014
* \brief   Acquisition thread with video in input.
*/

#include "CameraVideo.h"

boost::log::sources::severity_logger< LogSeverityLevel >  CameraVideo::logger;
CameraVideo::_Init CameraVideo::_initializer;

CameraVideo::CameraVideo(string video_path){

	//open the video file for reading
    cap = VideoCapture(videoPath);
    videoPath = video_path;

	frameWidth = 0;
	frameHeight = 0;

	endReadDataStatus = false;

}

CameraVideo::~CameraVideo(void){}

bool CameraVideo::grabStart(){

	//if not success, exit program
    if ( !cap.isOpened() ){

		 cout << "Cannot open the video file" << endl;
		 return false;
    }

	frameHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	cout << "Reading frame height : " << frameHeight << endl;

	frameWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
	cout << "Reading frame width : " << frameWidth << endl;

	Size frameSize(static_cast<int>(frameWidth), static_cast<int>(frameHeight));

	oVideoWriter = VideoWriter("./", CV_FOURCC('D', 'I', 'V', 'X'), 30, frameSize, true); //initialize the VideoWriter object

	int ex = static_cast<int>(cap.get(CV_CAP_PROP_FOURCC));

	char EXT[] = {ex & 0XFF , (ex & 0XFF00) >> 8,(ex & 0XFF0000) >> 16,(ex & 0XFF000000) >> 24, 0};

	return true;
}

bool CameraVideo::getStopStatus(){
	return endReadDataStatus;
}

bool CameraVideo::grabImage(Frame &img){

		Mat frame, copyframe;

		if(cap.read(frame)){

            cout << "FORMAT : " << cap.get(CV_CAP_PROP_FORMAT)<<endl;

            //BGR (3 channels) to G (1 channel)
            cvtColor(frame, frame, CV_BGR2GRAY);

            cvtColor(frame, copyframe, CV_GRAY2BGR);

            img.setNumFrame(cap.get(CV_CAP_PROP_POS_FRAMES));
			img.setImg(frame);
            img.setFrameRemaining(cap.get(CV_CAP_PROP_FRAME_COUNT) - cap.get(CV_CAP_PROP_POS_FRAMES));

            if(oVideoWriter.isOpened()){

                oVideoWriter << copyframe;

            }
	
			return true;

        }else{

            endReadDataStatus = true;
			return false;

        }
}

