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

    //********************* MASK PATH *****************************

    if(!cfg.Get("ACQ_MASK_PATH", mMaskPath)) {
        throw "Fail to load ACQ_MASK_PATH from configuration file.";
    }

    string acqFormat;
    cfg.Get("ACQ_FORMAT", acqFormat);
    EParser<CamPixFmt> camFormat;
    CamPixFmt bitDepth = camFormat.parseEnum("ACQ_FORMAT", acqFormat);

    mMaskControl = new Mask(10, mMaskEnabled, mMaskPath, mDownsampleEnabled, bitDepth, true);

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

    accStatus = {0,0,0,0,0,0,0,0,0,0,0,0};

    accMax = {5,10,15,20,25,30,35,40,45,50,55,60};



}

DetectionTemplate::~DetectionTemplate() {
    if(mMaskControl != nullptr)
        delete mMaskControl;
}

void DetectionTemplate::createDebugDirectories(bool cleanDebugDirectory) {

}

bool DetectionTemplate::run(Frame &c) {

    Mat currImg;

    if(mDownsampleEnabled)
        pyrDown(c.mImg, currImg, Size(c.mImg.cols / 2, c.mImg.rows / 2));
    else
        c.mImg.copyTo(currImg);

    // --------------------------------
    //          OPERATIONS
    // --------------------------------

    if(mMaskControl->applyMask(currImg)) {

        // --------------------------------
        //      Check previous frame.
        // --------------------------------

        if(!mPrevFrame.data) {

            cout << "PrevFrame has no data ! " << endl;
            currImg.copyTo(mPrevFrame);
            return false;

        }

        //SaveImg::saveJPEG(currImg, "/home/fripon/debug/original/frame_"+Conversion::intToString(c.mFrameNumber));

        Mat absdiffImg;
        cv::absdiff(currImg, mPrevFrame, absdiffImg);
        SaveImg::saveJPEG(Conversion::convertTo8UC1(absdiffImg), "/home/fripon/debug/absdiff/frame_" + Conversion::intToString(c.mFrameNumber));

        // ---------------------------------
        //  Dilatation absolute difference.
        // ---------------------------------

        int dilation_size = 2;
        Mat element = getStructuringElement(MORPH_RECT, Size(2*dilation_size + 1, 2*dilation_size+1), Point(dilation_size, dilation_size));
        cv::dilate(absdiffImg, absdiffImg, element);
        SaveImg::saveJPEG(Conversion::convertTo8UC1(absdiffImg), "/home/fripon/debug/dilate/frame_" + Conversion::intToString(c.mFrameNumber));

        // ----------------------------------
        //   Threshold absolute difference.
        // ----------------------------------

        Mat absDiffBinaryMap = Mat(currImg.rows,currImg.cols, CV_8UC1,Scalar(0));
        Scalar meanAbsDiff, stddevAbsDiff;
        cv::meanStdDev(absdiffImg, meanAbsDiff, stddevAbsDiff, mMaskControl->mCurrentMask);
        int absDiffThreshold = /*stddevAbsDiff[0] * 5 + 10;//*/meanAbsDiff[0] * 3;

        if(absdiffImg.type() == CV_16UC1) {

            unsigned short * ptrAbsDiff;
            unsigned char * ptrMap;

            for(int i = 0; i < absdiffImg.rows; i++) {

                ptrAbsDiff = absdiffImg.ptr<unsigned short>(i);
                ptrMap = absDiffBinaryMap.ptr<unsigned char>(i);

                for(int j = 0; j < absdiffImg.cols; j++){

                    if(ptrAbsDiff[j] > absDiffThreshold) {
                        ptrMap[j] = 255;
                    }
                }
            }

            SaveImg::saveJPEG(absDiffBinaryMap, "/home/fripon/debug/thresh/frame_" + Conversion::intToString(c.mFrameNumber));

        }

        currImg.copyTo(mPrevFrame);

    }else{

        mPrevFrame.release();

    }

/*
    for(int i =0; i < accStatus.size(); i++) {

        int value = (int)((255/accMax.at(i)) * accStatus.at(i));
        cout << "status : " << accStatus.at(i) << "/" << accMax.at(i) << " -> value : " << value << endl;

        if(accStatus.at(i) == 0 ) {

            Mat t = Mat(currImg.rows,currImg.cols, CV_8UC1,Scalar(0));
            accImg.push_back(t);
            Mat temp = Mat(currImg.rows,currImg.cols, CV_8UC1,Scalar(value));
            Mat res;
            temp.copyTo(res,absDiffBinaryMap);
            res.copyTo(accImg.at(i), absDiffBinaryMap);
            accStatus.at(i)++;

        }else{

            if(accStatus.at(i) == accMax.at(i)) {

                SaveImg::saveJPEG(accImg.at(i), "/home/fripon/debug/acc/frame_" + Conversion::intToString(c.mFrameNumber) + "_" + Conversion::intToString(accMax.at(i)));
                accStatus.at(i) = 1;

            }else{

                Mat temp = Mat(currImg.rows,currImg.cols, CV_8UC1,Scalar(value));
                Mat res;
                temp.copyTo(res,absDiffBinaryMap);
                res.copyTo(accImg.at(i), absDiffBinaryMap);
                accStatus.at(i)++;

            }

        }

    }
*/
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
