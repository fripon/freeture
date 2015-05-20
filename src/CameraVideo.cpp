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

CameraVideo::CameraVideo(vector<string> video_list){

    videoList = video_list;

    string source = video_list.front();

	//open the video file for reading
    cap = VideoCapture(source);

    videoID = 0;

	frameWidth = 0;
	frameHeight = 0;

	endReadDataStatus = false;

}

CameraVideo::~CameraVideo(void){}

bool CameraVideo::grabInitialization(){

	//if not success, exit program
    if ( !cap.isOpened() ){

		 cout << "Cannot open the video file" << endl;
		 return false;
    }

	return true;

}

bool CameraVideo::getStopStatus(){
	return endReadDataStatus;
}

bool CameraVideo::getDataStatus(){

    if(videoID == videoList.size())
        return false;
    else
        return true;
}

bool CameraVideo::loadNextDataSet(){

    if(videoID!=0){

        cout << "change video ! " << videoID << endl;
        string source = videoList.at(videoID);
        cout << "source : " << source << endl;
        //open the video file for reading
        cap = VideoCapture(source);

        //if not success, exit program
        if ( !cap.isOpened() ){

             cout << "Cannot open the video file" << endl;
             return false;

        }else{

            cout << "Success to open the video file" << endl;

        }

        frameHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
        cout << "Reading frame height : " << frameHeight << endl;

        frameWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        cout << "Reading frame width : " << frameWidth << endl;

        cout << "cap.get(CV_CAP_PROP_POS_FRAMES) :  " << cap.get(CV_CAP_PROP_POS_FRAMES) << endl;

        cout << "cap.get(CV_CAP_PROP_FRAME_COUNT) :  " << cap.get(CV_CAP_PROP_FRAME_COUNT) << endl;

        endReadDataStatus = false;

    }

    return true;

}

bool CameraVideo::grabImage(Frame &img){

		Mat frame, copyframe;

		if(cap.read(frame)){

            //BGR (3 channels) to G (1 channel)
            cvtColor(frame, frame, CV_BGR2GRAY);

            cout << "FORMAT : " << Conversion::matTypeToString(frame.type())<<endl;

            string acquisitionDate = TimeDate::localDateTime(microsec_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");
            boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
            string acqDateInMicrosec = to_iso_extended_string(time);

            Frame f = Frame(frame, 0, 0, acquisitionDate);

            img = f;

            img.setNumFrame(cap.get(CV_CAP_PROP_POS_FRAMES));

            img.setFrameRemaining(cap.get(CV_CAP_PROP_FRAME_COUNT) - cap.get(CV_CAP_PROP_POS_FRAMES));

            img.setAcqDateMicro(acqDateInMicrosec);

            img.setFPS(1);

            img.setBitDepth(MONO_8);

            waitKey(150);

			return true;

        }else{

            waitKey(150);

            videoID++;

            endReadDataStatus = true;
			return false;

        }
}

