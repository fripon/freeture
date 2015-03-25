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

	public:

        Device(CamType type);

		~Device();

		bool	prepareDevice(CamType type, string cfgFile);

		void	listGigeCameras();

		void	grabStop();

		void    acqStop();

		bool	getDeviceStopStatus();

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

		bool	setGain(int gain);

		bool    setFPS(int fps);

		bool	setPixelFormat(CamBitDepth depth);

};
