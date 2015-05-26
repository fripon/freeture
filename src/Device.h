/*
                                Device.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
*	Last modified:		21/01/2015
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
#include "ECamType.h"
#include "Camera.h"
#include "CameraGigeSdkAravis.h"
#include "CameraGigeSdkPylon.h"
#include "CameraGigeSdkIc.h"
#include "CameraVideo.h"
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

using namespace boost::filesystem;
using namespace cv;
using namespace std;

class Device{

    private:

        static boost::log::sources::severity_logger< LogSeverityLevel > logger;

        static class Init {

            public:

                Init() {

                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("Device"));

                }

        } initializer;

        Camera              *cam;
        vector<AcqSchedule> mSchedule;
        bool                mScheduleEnabled;
        string              mDataPath;
        string              mStationName;
        CamBitDepth         mBitDepth;
        int                 mNightExposure;
        int                 mNightGain;
        int                 mDayExposure;
        int                 mDayGain;
        int                 mFPS;
        int                 mCamID;
        Mat                 mMask;
        bool                mMaskEnabled;
        string              mMaskPath;
        bool                mDayAcqEnabled;
        bool                mEphemerisEnabled;
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
        bool                mDisplayVideoInInput;
        bool                mVideoFramesInInput;
        bool                mDetectionEnabled;
        int                 mMinExposureTime;
        int                 mMaxExposureTime;
        int                 mMinGain;
        int                 mMaxGain;
        Fits                mFitsHeader;
        CamType             mType;
        string              mCfgPath;

    public :

        /**
        * Constructor.
        *
        * @param type Type of camera in input.
        */
        Device(CamType type);

        /**
        * Constructor.
        *
        * @param type Type of camera in input.
        * @param cfgPath Path of the configuration file.
        */
        Device(CamType type, string cfgPath);

        /**
        * Destructor.
        *
        */
        ~Device();

        /**
        * Load camera's parameters.
        *
        */
        bool prepareDevice();

        /**
        * Configure camera in order to run continuous acquisition.
        *
        */
        void runContinuousAcquisition();

        Camera              *getCam() const                 {return cam;}
        vector<AcqSchedule> getSchedule()                   {return mSchedule;};
        string              getDataPath()                   {return mDataPath;};
        string              getStationName()                {return mStationName;};
        int                 getCameraId()                   {return mCamID;};
        Fits                getFitsHeader()                 {return mFitsHeader;};
        Mat                 getMask()                       {return mMask;};
        int                 getMaxGain()                    {return mMaxGain;};
        int                 getMinGain()                    {return mMinGain;};
        int                 getMinExposureTime()            {return mMinExposureTime;};
        int                 getMaxExposureTime()            {return mMaxExposureTime;};
        bool                getExposureControlSaveImage()   {return mExpCtrlSaveImg;};
        bool                getExposureControlSaveInfos()   {return mExpCtrlSaveInfos;};
        int                 getExposureControlFrequency()   {return mExpCtrlFrequency;};
        bool                getAcqDayEnabled()              {return mDayAcqEnabled;};
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
        bool                getDisplayInput()               {return mDisplayVideoInInput;};
        bool                getVideoFramesInput()           {return mVideoFramesInInput;};
        bool                getDetectionEnabled()           {return mDetectionEnabled;};

    private :

        /**
        * Initialize device and select correct SDK.
        *
        */
        void initialization();
};
