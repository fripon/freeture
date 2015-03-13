/*								CameraGigeSdkIc.h

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
* \file    CameraGigeSdkIc.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    21/01/2015
* \brief   Use Imaging source sdk to pilot GigE Cameras.
*          https://wiki.gnome.org/action/show/Projects/Aravis?action=show&redirect=Aravis
*/

#pragma once

#include "config.h"

#ifdef WINDOWS

	#include "opencv2/highgui/highgui.hpp"
	#include <opencv2/imgproc/imgproc.hpp>

	#include <iostream>
	#include <string> 
	#include "Frame.h"
	#include "TimeDate.h"
	#include "Camera.h"
	
	#include "ECamBitDepth.h"
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

	#include "tisudshl.h" 
	#include <algorithm>

	using namespace cv;
	using namespace std;

	class CameraGigeSdkIc: public Camera{

		private:

			static boost::log::sources::severity_logger< LogSeverityLevel > logger;

			static class _Init{

				public:
					_Init()
					{
						logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("CameraGigeSdkIc"));
					}
			} _initializer;		

			DShowLib::tIVCDRangePropertyPtr	getPropertyRangeInterface(_DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr& pItems, const GUID& id);
			bool propertyIsAvailable(const GUID& id, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer);
			long getPropertyValue(const GUID& id, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer);
			void setPropertyValue(const GUID& id, long val, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer);
			long getPropertyRangeMin(const GUID& id, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer);
			long getPropertyRangeMax(const GUID& id, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer);

		public:

			DShowLib::Grabber* m_pGrabber;

			CameraGigeSdkIc();

			void test();

			~CameraGigeSdkIc();

			void listGigeCameras();

			bool grabSingleImage(Frame &frame, int camID);

			bool getAvailableFormat();

			bool	createDevice(int id, string name){};
			/*bool    getDeviceNameById(int id, string &device);

			bool	grabStart();
			void	grabStop();
			void    acqStart();
			void    acqStop();

			bool    grabImage(Frame& newFrame);
			bool	grabSingleImage(Frame &frame, int camID);

			void	getExposureBounds(int &eMin, int &eMax);
			void	getGainBounds(int &gMin, int &gMax);
			bool	getPixelFormat(CamBitDepth &format);
			int		getWidth();
			int		getHeight();
			int		getFPS();
			string	getModelName();

			bool	setExposureTime(int exp);
			bool	setGain(int gain);
			bool    setFPS(int fps);
			bool	setPixelFormat(CamBitDepth depth);*/

	};

#endif
