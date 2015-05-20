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

		static class _Init{

			public:
				_Init()
				{
					logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("Device"));
				}
		} _initializer;

        Camera *cam;

        vector<AcqSchedule>  ACQ_SCHEDULE;
        bool                ACQ_SCHEDULE_ENABLED;
        string              DATA_PATH;
        string              STATION_NAME;
        CamBitDepth         ACQ_BIT_DEPTH;
        int                 ACQ_NIGHT_EXPOSURE;
        int                 ACQ_NIGHT_GAIN;
        int                 ACQ_DAY_EXPOSURE;
        int                 ACQ_DAY_GAIN;
        int                 ACQ_FPS;
        int                 CAMERA_ID;
        Mat                 ACQ_MASK;
        bool                ACQ_MASK_ENABLED;
        string              ACQ_MASK_PATH;
        bool                ACQ_DAY_ENABLED;
        bool                EPHEMERIS_ENABLED;
        bool                EXPOSURE_CONTROL_SAVE_IMAGE;
        bool                EXPOSURE_CONTROL_SAVE_INFOS;
        int                 EXPOSURE_CONTROL_FREQUENCY;
        vector<int>         SUNRISE_TIME;
        vector<int>         SUNSET_TIME;
        int                 SUNSET_DURATION;
        int                 SUNRISE_DURATION;
        bool                ACQ_REGULAR_ENABLED;
        int                 ACQ_REGULAR_INTERVAL;
        int                 ACQ_REGULAR_EXPOSURE;
        int                 ACQ_REGULAR_GAIN;
        CamBitDepth         ACQ_REGULAR_FORMAT;
        int                 ACQ_REGULAR_REPETITION;
        bool                DISPLAY_INPUT;
        bool                VIDEO_FRAMES_INPUT;
        bool                DETECTION_ENABLED;


        int minExposureTime;
        int maxExposureTime;
        int minGain;
        int maxGain;


        Fits fitsHeader;

	public:

        Device(CamType type);

		~Device();

		bool	prepareDevice(CamType type, string cfgFile);

		void	listGigeCameras();

		void	grabStop();

		void    acqStop();

		void    acqRestart();

		bool	getDeviceStopStatus();

		bool    getDatasetStatus();

		bool    grabImage(Frame& newFrame);

		bool	grabSingleImage(Frame &frame, int camID);

		void	getExposureBounds(int &gMin, int &gMax);

		void	getGainBounds(int &eMin, int &eMax);

		bool	getPixelFormat(CamBitDepth &format);

		int		getWidth();

		int		getHeight();

		int		getFPS();

		string	getModelName();

		bool	setExposureTime(int exp);

		int     getExposureTime();

		bool	setGain(int gain);

		bool    setFPS(int fps);

		bool	setPixelFormat(CamBitDepth depth);

		bool    loadDataset();

		vector<AcqSchedule>    getSchedule()       {return ACQ_SCHEDULE;};

		string      getDataPath()                   {return DATA_PATH;};
		string      getStationName()                {return STATION_NAME;};
		int         getCameraId()                   {return CAMERA_ID;};
		Fits        getFitsHeader()                 {return fitsHeader;};
		Mat         getMask()                       {return ACQ_MASK;};
		int         getMaxGain()                    {return maxGain;};
		int         getMinGain()                    {return minGain;};
        int         getMinExposureTime()            {return minExposureTime;};
        int         getMaxExposureTime()            {return maxExposureTime;};
        bool        getExposureControlSaveImage()   {return EXPOSURE_CONTROL_SAVE_IMAGE;};
        bool        getExposureControlSaveInfos()   {return EXPOSURE_CONTROL_SAVE_INFOS;};
        int         getExposureControlFrequency()   {return EXPOSURE_CONTROL_FREQUENCY;};
        bool        getAcqDayEnabled()              {return ACQ_DAY_ENABLED;};
        vector<int> getSunriseTime()                {return SUNRISE_TIME;};
        int         getSunriseDuration()            {return SUNRISE_DURATION;};
        vector<int> getSunsetTime()                 {return SUNSET_TIME;};
        int         getSunsetDuration()             {return SUNSET_DURATION;};
        int         getNightExposureTime()          {return ACQ_NIGHT_EXPOSURE;};
        int         getDayExposureTime()            {return ACQ_DAY_EXPOSURE;};
        int         getDayGain()                    {return ACQ_DAY_GAIN;};
        int         getNightGain()                  {return ACQ_NIGHT_GAIN;};
        bool        getAcqRegularEnabled()          {return ACQ_REGULAR_ENABLED;};
        int         getAcqRegularTimeInterval()     {return ACQ_REGULAR_INTERVAL;};
        int         getAcqRegularExposure()         {return ACQ_REGULAR_EXPOSURE;};
        int         getAcqRegularGain()             {return ACQ_REGULAR_GAIN;};
        CamBitDepth getAcqRegularFormat()           {return ACQ_REGULAR_FORMAT;};
        int         getAcqRegularRepetition()       {return ACQ_REGULAR_REPETITION;};
        bool        getAcqScheduleEnabled()         {return ACQ_SCHEDULE_ENABLED;};
        bool        getDisplayInput()               {return DISPLAY_INPUT;};
        bool        getVideoFramesInput()           {return VIDEO_FRAMES_INPUT;};
        bool        getDetectionEnabled()           {return DETECTION_ENABLED;};


    private :

        void    runContinuousAcquisition();

};
