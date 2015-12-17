/*
                                Device.h

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
*   Last modified:      21/01/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    Device.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    13/06/2014
* \brief
*/

#pragma once

#include "config.h"

#ifdef LINUX
#define BOOST_LOG_DYN_LINK 1
#endif

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/core.hpp>
#include "ELogSeverityLevel.h"
#include "EImgBitDepth.h"
#include "ECamPixFmt.h"
#include "EParser.h"
#include "Conversion.h"
#include "Camera.h"
#include "CameraGigeAravis.h"
#include "CameraGigePylon.h"
#include "CameraGigeTis.h"
#include "CameraVideo.h"
#include "CameraV4l2.h"
#include "CameraFrames.h"
#include "CameraWindows.h"
#include <vector>
#include <algorithm>
#include <string>
#include <boost/filesystem.hpp>
#include <iterator>
#include <algorithm>
#include <boost/tokenizer.hpp>
#include <boost/circular_buffer.hpp>
#include "EInputDeviceType.h"
#include "ECamSdkType.h"
#include "SParam.h"

using namespace boost::filesystem;
using namespace cv;
using namespace std;

class Device {

    public :

        bool mVideoFramesInput; // TRUE if input is a video file or frames directories.

    private :

        static boost::log::sources::severity_logger< LogSeverityLevel > logger;

        static class Init {

            public:

                Init() {

                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("Device"));

                }

        } initializer;

        vector<pair<int,pair<int,CamSdkType>>> mDevices;

        bool        mCustomSize;
        int         mSizeWidth;
        int         mSizeHeight;
        int         mNightExposure;
        int         mNightGain;
        int         mDayExposure;
        int         mDayGain;
        int         mFPS;
        int         mCamID;         // ID in a specific sdk.
        int         mGenCamID;      // General ID.
        Camera      *mCam;
        bool        mShiftBits;
        bool        mVerbose;
        framesParam mfp;
        videoParam  mvp;

    public :

        int         mNbDev;
        CamPixFmt   mFormat;
        string      mCfgPath;
        InputDeviceType mDeviceType;
        double      mMinExposureTime;
        double      mMaxExposureTime;
        int         mMinGain;
        int         mMaxGain;
        //int         mNbFrame;

    public :

        Device(cameraParam cp, framesParam fp, videoParam vp, int cid);

        Device();

        ~Device();

        InputDeviceType getDeviceType(CamSdkType t);

        CamSdkType getDeviceSdk(int id);

        void listDevices(bool printInfos);

        bool createCamera(int id, bool create);

        bool createCamera();

        bool initializeCamera();

        bool runContinuousCapture(Frame &img);

        bool runSingleCapture(Frame &img);

        bool startCamera();

        bool stopCamera();

        bool setCameraPixelFormat();

        bool getCameraGainBounds(int &min, int &max);

        void getCameraGainBounds();

        bool getCameraExposureBounds(double &min, double &max);

        void getCameraExposureBounds();

        bool getDeviceName();

        bool recreateCamera();

        InputDeviceType getDeviceType();

        bool setCameraNightExposureTime();

        bool setCameraDayExposureTime();

        bool setCameraNightGain();

        bool setCameraDayGain();

        bool setCameraExposureTime(double value);

        bool setCameraGain(int value);

        bool setCameraFPS();

        bool setCameraSize();

        bool getCameraFPS(double &fps);

        bool getCameraStatus();

        bool getCameraDataSetStatus();

        bool getSupportedPixelFormats();

        bool loadNextCameraDataSet(string &location);

        bool getExposureStatus();

        bool getGainStatus();

        bool setCameraSize(int w, int h);

        int getNightExposureTime() {return mNightExposure;};
        int getNightGain() {return mNightGain;};
        int getDayExposureTime() {return mDayExposure;};
        int getDayGain() {return mDayGain;};

        void setVerbose(bool status);

    private :

        bool createDevicesWith(CamSdkType sdk);

};
