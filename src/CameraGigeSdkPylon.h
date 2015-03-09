/*
							CameraGigeSdkPylon.h

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
 * \file    CameraGigeSdkPylon.cpp
 * \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * \version 1.0
 * \date    03/07/2014
 * \brief   Use Pylon library to pilot GigE Cameras.
 */
#pragma once

#include "config.h"

#ifdef USE_PYLON



#ifdef LINUX
    #define BOOST_LOG_DYN_LINK 1
#endif

	#include "Frame.h"
	#include "TimeDate.h"
	#include "Conversion.h"
	#include "SaveImg.h"
	#include "Camera.h"
	#include <boost/log/common.hpp>
//#include <boost/log/expressions.hpp>
//#include <boost/log/utility/setup/file.hpp>
//#include <boost/log/utility/setup/console.hpp>
//#include <boost/log/utility/setup/common_attributes.hpp>
//#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/attributes.hpp>
//#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/core.hpp>
#include "ELogSeverityLevel.h"
	// pylon api
	#include <pylon/PylonIncludes.h>
	#include <pylon/gige/BaslerGigEInstantCamera.h>
	#include <pylon/gige/BaslerGigECamera.h>

	using namespace Pylon;
	using namespace GenApi;
	using namespace cv;
	using namespace std;
	using namespace Basler_GigECameraParams;

	// Buffer's number used for grabbing
	static const uint32_t nbBuffers = 20;

	class CameraGigeSdkPylon : public Camera{

		private:

			static boost::log::sources::severity_logger< LogSeverityLevel > logger;

			static class _Init{

				public:
					_Init()
					{
						logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("CameraGigeSdkPylon"));
					}
			} _initializer;

			Pylon::PylonAutoInitTerm autoInitTerm;

			//! Buffer for the grabbed images in 8 bits format
			uint8_t* ppBuffersUC[nbBuffers];

			//! Buffer for the grabbed images in 8 bits format
			uint16_t* ppBuffersUS[nbBuffers];

			StreamBufferHandle handles[nbBuffers];

			//! Pointer on the transport layer
			CTlFactory			*pTlFactory;

			//! Pointer on basler camera
			CBaslerGigECamera	*pCamera;

			//! Pointer on device
			IPylonDevice		*pDevice;

			DeviceInfoList_t	 devices;

			CBaslerGigECamera::EventGrabber_t	* pEventGrabber;
			IEventAdapter						* pEventAdapter;
			CBaslerGigECamera::StreamGrabber_t	* pStreamGrabber;

			int nbEventBuffers;
			GrabResult result;
			bool connectionStatus;

		public:

			CameraGigeSdkPylon();
			~CameraGigeSdkPylon(void);

			void	listGigeCameras();
			bool	createDevice(int id);
			bool    getDeviceNameById(int id, string &device);

			bool	grabStart();
			void	grabStop();
			void    acqStart();
			void    acqStop();

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

#endif
