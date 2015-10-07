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
#include "ECamBitDepth.h"
#include "EParser.h"
#include "Conversion.h"
#include "ECamType.h"
#include "Camera.h"
#include "CameraGigeAravis.h"
#include "CameraGigePylon.h"
#include "CameraGigeTis.h"
#include "CameraVideo.h"
#include "CameraV4l2.h"
#include "CameraFrames.h"
#include "AcqSchedule.h"
#include "Fits.h"
#include <vector>
#include <algorithm>
#include <string>
#include <boost/filesystem.hpp>
#include <iterator>
#include <algorithm>
#include <boost/tokenizer.hpp>
#include <boost/circular_buffer.hpp>
#include "Ephemeris.h"
#include "ETimeMode.h"
#include "CameraWindows.h"

using namespace boost::filesystem;
using namespace cv;
using namespace std;

 enum CamSdkType{

    ARAVIS,
    PYLONGIGE,
    TIS,
    VIDEOFILE,
    FRAMESDIR,
    V4L2,
    VIDEOINPUT

};

class Device {

    private:

        static boost::log::sources::severity_logger< LogSeverityLevel > logger;

        static class Init {

            public:

                Init() {

                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("Device"));

                }

        } initializer;

        vector<AcqSchedule> mSchedule;
        bool                mScheduleEnabled;
        string              mDataPath;
        string              mStationName;
        Mat                 mMask;
        bool                mMaskEnabled;
        string              mMaskPath;

        CamBitDepth         mBitDepth;
        int                 mNightExposure;
        int                 mNightGain;
        int                 mDayExposure;
        int                 mDayGain;
        int                 mFPS;


        bool                mExpCtrlSaveImg;
        bool                mExpCtrlSaveInfos;
        int                 mExpCtrlFrequency;
        vector<int>         mSunriseTime;
        vector<int>         mSunsetTime;
        int                 mSunsetDuration;
        int                 mSunriseDuration;
        bool                mRegularAcqEnabled;
        int                 mRegularInterval;
        int                 mRegularExposure;
        int                 mRegularGain;
        CamBitDepth         mRegularFormat;
        int                 mRegularRepetition;

        bool                mVideoFramesInInput;
        bool                mDetectionEnabled;
        int                 mMinExposureTime;
        int                 mMaxExposureTime;
        int                 mMinGain;
        int                 mMaxGain;
        Fits                mFitsHeader;
        string              mCfgPath;
        bool                mDebugEnabled;
        double              mSunHorizon1;
        double              mSunHorizon2;
        double              mStationLongitude;
        double              mStationLatitude;

        vector<pair<int,pair<int,CamSdkType>>> mDevices;

    public :

        bool                mEphemerisUpdated;
        bool                mEphemerisEnabled;
        string              mCurrentDate;

        int mStartSunriseTime;
        int mStopSunriseTime;
        int mStartSunsetTime;
        int mStopSunsetTime;
        int mCurrentTime; // In seconds.

        TimeMode            mRegularMode;
        ImgFormat           mRegularOutput;
        ImgFormat           mScheduleOutput;

        TimeMode            mDetectionMode;
        TimeMode            mStackMode;

        int mCamID;
        Camera *mCam;

    public :

        Device(string cfgPath);

        Device();

        ~Device();

        bool createCamera(int id);

        bool prepareDevice(int id);

        bool runContinuousAcquisition();

        vector<AcqSchedule> getSchedule()                   {return mSchedule;};
        string              getDataPath()                   {return mDataPath;};
        string              getStationName()                {return mStationName;};
        Fits                getFitsHeader()                 {return mFitsHeader;};
        Mat                 getMask()                       {return mMask;};
        int                 getMinExposureTime()            {return mMinExposureTime;};
        int                 getMaxExposureTime()            {return mMaxExposureTime;};
        bool                getExposureControlSaveImage()   {return mExpCtrlSaveImg;};
        bool                getExposureControlSaveInfos()   {return mExpCtrlSaveInfos;};
        int                 getExposureControlFrequency()   {return mExpCtrlFrequency;};
        vector<int>         getSunriseTime()                {return mSunriseTime;};
        int                 getSunriseDuration()            {return mSunriseDuration;};
        vector<int>         getSunsetTime()                 {return mSunsetTime;};
        int                 getSunsetDuration()             {return mSunsetDuration;};
        int                 getNightExposureTime()          {return mNightExposure;};
        int                 getDayExposureTime()            {return mDayExposure;};
        int                 getDayGain()                    {return mDayGain;};
        int                 getNightGain()                  {return mNightGain;};
        bool                getAcqRegularEnabled()          {return mRegularAcqEnabled;};
        int                 getAcqRegularTimeInterval()     {return mRegularInterval;};
        int                 getAcqRegularExposure()         {return mRegularExposure;};
        int                 getAcqRegularGain()             {return mRegularGain;};
        CamBitDepth         getAcqRegularFormat()           {return mRegularFormat;};
        int                 getAcqRegularRepetition()       {return mRegularRepetition;};
        bool                getAcqScheduleEnabled()         {return mScheduleEnabled;};
        bool                getVideoFramesInput()           {return mVideoFramesInInput;};

        bool getSunTimes();

        void listDevices(bool printInfos);

    private :

        bool createDevicesWith(CamSdkType sdk);

};
