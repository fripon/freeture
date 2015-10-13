/*
                        DetectionTemplate.cpp

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
* \file    DetectionTemplate.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/03/2015
*/

#include "DetectionTemplate.h"

boost::log::sources::severity_logger< LogSeverityLevel > DetectionTemplate::logger;

DetectionTemplate::Init DetectionTemplate::initializer;

DetectionTemplate::DetectionTemplate(string cfgPath):
mDownsampleEnabled(false), mSavePos(false), mMaskEnabled(false), mImgNum(0),
mDebugEnabled(false), mDebugVideo(false), mMaskToCreate(false),
mDataSetCounter(0) {

    Configuration cfg;
    cfg.Load(cfgPath);

    //********************* DEBUG OPTION.******************************

    if(!cfg.Get("DET_DEBUG", mDebugEnabled)) {
        mDebugEnabled = false;
        BOOST_LOG_SEV(logger, warning) << "Fail to load DET_DEBUG from configuration file. Set to false.";
    }

    //********************* DOWNSAMPLE OPTION *************************

    if(!cfg.Get("DET_DOWNSAMPLE_ENABLED", mDownsampleEnabled)) {
        mDownsampleEnabled = true;
        BOOST_LOG_SEV(logger, warning) << "Fail to load DET_DOWNSAMPLE_ENABLED from configuration file. Set to true.";
    }

    //********************* SAVE POSITION OPTION **********************

    if(!cfg.Get("DET_SAVE_POS", mSavePos)) {
        mSavePos = true;
        BOOST_LOG_SEV(logger, warning) << "Fail to load DET_SAVE_POS from configuration file. Set to true.";
    }

    //********************* USE MASK OPTION ***************************

    if(!cfg.Get("ACQ_MASK_ENABLED", mMaskEnabled)) {
        mMaskEnabled = false;
        BOOST_LOG_SEV(logger, warning) << "Fail to load ACQ_MASK_ENABLED from configuration file. Set to false.";
    }

    if(mMaskEnabled) {

        //********************* MASK PATH *****************************

        if(!cfg.Get("ACQ_MASK_PATH", mMaskPath)) {
            throw "Fail to load ACQ_MASK_PATH from configuration file.";
        }

        mMask = imread(mMaskPath, CV_LOAD_IMAGE_GRAYSCALE);

        if(!mMask.data){
            throw "Fail to read the mask specified in ACQ_MASK_PATH.";
        }

        if(mDownsampleEnabled){
            pyrDown(mMask, mMask, Size(mMask.cols/2, mMask.rows/2));
        }

        mMask.copyTo(mOriginalMask);

    }else{

        mMaskToCreate = true;

    }

    //********************* DEBUG PATH ********************************

    if(!cfg.Get("DET_DEBUG_PATH", mDebugPath) && mDebugEnabled) {
        mDebugEnabled = false;
        mDebugPath = "";
        BOOST_LOG_SEV(logger, warning) << "Error about DET_DEBUG_PATH from configuration file. DET_DEBUG option disabled.";
    }

    mDebugCurrentPath = mDebugPath;

    //********************* DEBUG VIDEO OPETION ***********************

    if(!cfg.Get("DET_DEBUG_VIDEO", mDebugVideo)) {
        mDebugVideo = false;
        BOOST_LOG_SEV(logger, warning) << "Fail to load DET_DEBUG_VIDEO from configuration file. Set to false.";
    }

    // Create directories for debugging method.
    if(mDebugEnabled)
        createDebugDirectories(true);

    // Create debug video.
    if(mDebugVideo)
        mVideoDebug = VideoWriter(mDebugCurrentPath + "debug-video.avi", CV_FOURCC('M', 'J', 'P', 'G'), 5, Size(static_cast<int>(1280), static_cast<int>(960)), true);


}

DetectionTemplate::~DetectionTemplate() {

}

void DetectionTemplate::createDebugDirectories(bool cleanDebugDirectory) {

}

bool DetectionTemplate::run(Frame &c) {

    int h = 1, w = 1;

    // Current frame.
    Mat currImg;

    if(mDownsampleEnabled) {

        //mDownsampleTime = (double)getTickCount();
        h = c.mImg.rows/2;
        w = c.mImg.cols/2;
        pyrDown(c.mImg, currImg, Size(c.mImg.cols / 2, c.mImg.rows / 2));
        //mDownsampleTime = ((double)getTickCount() - mDownsampleTime);

    }else {

        h = c.mImg.rows;
        w = c.mImg.cols;
        c.mImg.copyTo(currImg);

    }

    // -------------------------------
    //  Create default mask if needed.
    // -------------------------------

    if(mMaskToCreate && !mMaskEnabled) {

        mMask = Mat(h, w, CV_8UC1,Scalar(255));
        mMask.copyTo(mOriginalMask);
        mMaskToCreate = false;

    }

    // --------------------------------
    //           Apply mask.
    // --------------------------------

    if(currImg.rows == mMask.rows && currImg.cols == mMask.cols) {

        Mat temp;
        currImg.copyTo(temp, mMask);
        temp.copyTo(currImg);

    }else {

        throw "ERROR : Mask size is not correct according to the frame size.";

    }

    // --------------------------------
    //      Check previous frame.
    // --------------------------------

    if(!mPrevFrame.data) {

        currImg.copyTo(mPrevFrame);
        return false;

    }

    // --------------------------------
    //          OPERATIONS
    // --------------------------------

    cout << "Frame : "
    << Conversion::intToString(c.mDate.year)
    << Conversion::intToString(c.mDate.month)
    << Conversion::intToString(c.mDate.day)
    << Conversion::intToString(c.mDate.hours)
    << Conversion::intToString(c.mDate.minutes)
    << Conversion::intToString(c.mDate.seconds) << endl;

    // No detection : return false
    return false;

}

void DetectionTemplate::saveDetectionInfos(string p) {


}

void DetectionTemplate::resetDetection(bool loadNewDataSet) {


}

void DetectionTemplate::resetMask() {


}

int DetectionTemplate::getNumFirstEventFrame() {

    return 0;

}

TimeDate::Date DetectionTemplate::getDateEvent() {

    TimeDate::Date d;
    return d;

}

int DetectionTemplate::getNumLastEventFrame() {

    return 0;

}
