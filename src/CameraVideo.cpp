/*
                                CameraVideo.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*   This file is part of:   freeture
*
*   Copyright:      (C) 2014-2015 Yoan Audureau
*                               FRIPON-GEOPS-UPSUD-CNRS
*
*   License:        GNU General Public License
*
*   FreeTure is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*   FreeTure is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*   You should have received a copy of the GNU General Public License
*   along with FreeTure. If not, see <http://www.gnu.org/licenses/>.
*
*   Last modified:      20/07/2015
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

CameraVideo::Init CameraVideo::initializer;

CameraVideo::CameraVideo(vector<string> videoList, bool verbose):mVideoID(0), mFrameWidth(0), mFrameHeight(0), mReadDataStatus(false){

    mVideoList = videoList;

    // Open the video file for reading.
    if(mVideoList.size()>0)
        mCap = VideoCapture(videoList.front());
    else 
        throw "No video path in input.";

    mExposureAvailable = false;
    mGainAvailable = false;
    mInputDeviceType = VIDEO;
    mVerbose = verbose;

}

CameraVideo::~CameraVideo(void){

}

bool CameraVideo::grabInitialization(){

    if(!mCap.isOpened()) {

         if(mVerbose) BOOST_LOG_SEV(logger,fail) << "Cannot open the video file";
         if(mVerbose) cout << "Cannot open the video file" << endl;
         return false;
    }

    return true;

}

bool CameraVideo::getStopStatus(){

    return mReadDataStatus;

}

bool CameraVideo::getDataSetStatus(){

    if(mVideoID == mVideoList.size())
        return false;
    else
        return true;
}

bool CameraVideo::loadNextDataSet(string &location){

    if(mVideoID != 0){

        cout << "Change video : " << mVideoID << " - Path : " << mVideoList.at(mVideoID) << endl;

        mCap = VideoCapture(mVideoList.at(mVideoID));

        if(!mCap.isOpened()){

             cout << "Cannot open the video file" << endl;
             return false;

        }else{

            cout << "Success to open the video file" << endl;

        }

        mFrameHeight = mCap.get(CV_CAP_PROP_FRAME_HEIGHT);

        mFrameWidth = mCap.get(CV_CAP_PROP_FRAME_WIDTH);

        mReadDataStatus = false;

    }

    return true;

}

bool CameraVideo::createDevice(int id) {
    return true;
}

bool CameraVideo::grabImage(Frame &img){

    Mat frame;

    if(mCap.read(frame)) {

        //BGR (3 channels) to G (1 channel)
        cvtColor(frame, frame, CV_BGR2GRAY);

        boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();

        Frame f = Frame(frame, 0, 0, to_iso_extended_string(time));

        img = f;
        img.mFrameNumber = mCap.get(CV_CAP_PROP_POS_FRAMES);
        img.mFrameRemaining = mCap.get(CV_CAP_PROP_FRAME_COUNT) - mCap .get(CV_CAP_PROP_POS_FRAMES);
        return true;

    }

    if(mCap.get(CV_CAP_PROP_FRAME_COUNT) - mCap .get(CV_CAP_PROP_POS_FRAMES) <=0) {

        mVideoID++;
        mReadDataStatus = true;

    }

    return false;
}


