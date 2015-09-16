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

DetectionTemplate::DetectionTemplate():
mDownsampleEnabled(false), mSavePos(false), mMaskEnabled(false), mImgNum(0),
mDebugEnabled(false), mDebugVideo(false), mMaskToCreate(false),
mDataSetCounter(0) {

}

DetectionTemplate::~DetectionTemplate() {

}

bool DetectionTemplate::initMethod(string cfgPath) {

    try {

        Configuration cfg;
        cfg.Load(cfgPath);

        // Get debug option.
        cfg.Get("DET_DEBUG", mDebugEnabled);
        BOOST_LOG_SEV(logger, notification) << "DET_DEBUG : " << mDebugEnabled;

        // Get downsample option.
        cfg.Get("DET_DOWNSAMPLE_ENABLED", mDownsampleEnabled);
        BOOST_LOG_SEV(logger, notification) << "DET_DOWNSAMPLE_ENABLED : " << mDownsampleEnabled;

        // Get save position option.
        cfg.Get("DET_SAVE_POS", mSavePos);
        BOOST_LOG_SEV(logger, notification) << "DET_SAVE_POS : " << mSavePos;

        // Get use mask option.
        cfg.Get("ACQ_MASK_ENABLED", mMaskEnabled);
        BOOST_LOG_SEV(logger, notification) << "ACQ_MASK_ENABLED : " << mMaskEnabled;

        if(mMaskEnabled) {

            cfg.Get("ACQ_MASK_PATH", mMaskPath);
            BOOST_LOG_SEV(logger, notification) << "ACQ_MASK_PATH : " << mMaskPath;

            mMask = imread(mMaskPath, CV_LOAD_IMAGE_GRAYSCALE);

            if(!mMask.data){
                cout << " Can't load the mask from this location : " << mMaskPath;
                BOOST_LOG_SEV(logger, notification) << " Can't load the mask from this location : " << mMaskPath;
                throw "Can't load the mask. Wrong location.";
            }

            if(mDownsampleEnabled){

                int imgH = mMask.rows/2;
                int imgW = mMask.cols/2;

                pyrDown(mMask, mMask, Size(imgW, imgH));

            }

            mMask.copyTo(mOriginalMask);

        }else{

            mMaskToCreate = true;

        }

        // Get debug path.
        cfg.Get("DET_DEBUG_PATH", mDebugPath);
        BOOST_LOG_SEV(logger, notification) << "DET_DEBUG_PATH : " << mDebugPath;
        mDebugCurrentPath = mDebugPath;

        // Get debug video option.
        cfg.Get("DET_DEBUG_VIDEO", mDebugVideo);
        BOOST_LOG_SEV(logger, notification) << "DET_DEBUG_VIDEO : " << mDebugVideo;

        // Create directories for debugging method.
        if(mDebugEnabled)
            createDebugDirectories(true);

        // Create debug video.
        if(mDebugVideo)
            mVideoDebug = VideoWriter(mDebugCurrentPath + "debug-video.avi", CV_FOURCC('M', 'J', 'P', 'G'), 5, Size(static_cast<int>(1280), static_cast<int>(960)), true);

    }catch(exception& e){

        cout << e.what() << endl;
        return false;

    }catch(const char * msg){

        cout << msg << endl;
        return false;

    }

    return true;

}

void DetectionTemplate::createDebugDirectories(bool cleanDebugDirectory) {

    /*cout << "Debug mode : enabled." << endl;

    mDebugCurrentPath = mDebugPath + "debug_" + Conversion::intToString(mDataSetCounter) + "/" ;

    if(cleanDebugDirectory) {

        cout << "Clean and create debug directories..." << endl;

        const boost::filesystem::path p0 = path(mDebugPath); // .../debug/

        if(boost::filesystem::exists(p0))
            boost::filesystem::remove_all(p0);

        if(!boost::filesystem::exists(p0))
            boost::filesystem::create_directories(p0);

    }else {

        cout << "Create debug directories..." << endl;

    }

    const boost::filesystem::path p1 = path(mDebugCurrentPath);             // .../debug/debug_0/
    const boost::filesystem::path p2 = path(mDebugCurrentPath + "local");   // .../debug/debug_0/local/
    const boost::filesystem::path p3 = path(mDebugCurrentPath + "global");  // .../debug/debug_0/global/

    if(!boost::filesystem::exists(p1))
        boost::filesystem::create_directories(p1);

    if(!boost::filesystem::exists(p2))
        boost::filesystem::create_directories(p2);

    if(!boost::filesystem::exists(p3))
        boost::filesystem::create_directories(p3);

    vector<string> debugSubDir;
    debugSubDir.push_back("absolute_difference");
    debugSubDir.push_back("absolute_difference_thresholded");
    debugSubDir.push_back("event_map_initial");
    debugSubDir.push_back("event_map_filtered");
    debugSubDir.push_back("absolute_difference_dilated");
    debugSubDir.push_back("neg_difference_thresholded");
    debugSubDir.push_back("pos_difference_thresholded");
    debugSubDir.push_back("neg_difference");
    debugSubDir.push_back("pos_difference");

    for(int i = 0; i< debugSubDir.size(); i++){

        const boost::filesystem::path path(mDebugCurrentPath + "global/" + debugSubDir.at(i));

        if(!boost::filesystem::exists(path))
            boost::filesystem::create_directories(path);

    }*/

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
