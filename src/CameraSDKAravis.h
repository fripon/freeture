/*
				CameraSDKAravis.h

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
*	Last modified:		30/06/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#pragma once

#include "includes.h"
#include "EnumLog.h"
#include "CameraSDK.h"
#include "Fifo.h"
#include "Frame.h"
#include "TimeDate.h"
#include "arv.h"
#include "arvinterface.h"
#include "ECamBitDepth.h"

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

using boost::shared_ptr;

using namespace cv;
using namespace std;

using namespace logenum;

class CameraSDKAravis: public CameraSDK{

//    enum severity_level{
//		normal,
//		notification,
//		warning,
//		fail,
//		critical
//	};

	private:

        ArvCamera       *camera;
        ArvPixelFormat  pixFormat;
        ArvStream       *stream;

        int             width;
        int             height;
        double          fps;
        double          gainMin,
                        gainMax;
        unsigned int    payload;

        const char      *pixel_format_string;
        double          exposureMin,
                        exposureMax;
        const char      *caps_string;
        int gain;
        double exp;

        guint64 n_completed_buffers;
        guint64 n_failures;
        guint64 n_underruns;

		src::severity_logger< severity_level > log;			// logger

    public:

		CameraSDKAravis();

		~CameraSDKAravis();

         //! List connected cameras
		void	listCameras(void);

		//! Select a connected device
		bool	chooseDevice(string);

		//! Start grabbing images
		bool		grabStart();

		//! Stop grabbing images
		void	grabStop();

		//! Grab an image
        bool    grabImage(Frame*& newFrame, Mat newImage);

		//! Restart grabbing images
		void	grabRestart();

        //! Get the minimum exposition value
		double	getExpoMin(void);

		//! Get the maximum exposition value
		double	getExpoMax(void);

		//! Get the minimum gain value
		int		getGainMin(void);

		//! Get the maximum exposition value
		int		getGainMax(void);

		//! Get the pixel's format
		CamBitDepth		getPixelFormat(void);

		//! Get the width
		int		getWidth(void);

		//! Get the height
		int		getHeight(void);

		//! Get the frame per second rate
		double	getFPS(void);

		//! Get the camera's model name
		string	getModelName();

		bool    getDeviceById(int id, string &device);

        //! Set the exposure time
        /*!
          \param exp exposure time
        */
		bool	setExposureTime(double exposition);

		//! Set the gain
        /*!
          \param gain
        */
		bool	setGain(int gain);

		bool    setFPS(int fps);

		//! Set the pixel format
        /*!
          \param format 12 or 8 bits
        */
		bool	setPixelFormat(CamBitDepth depth);

		void    acqStart();

        void    acqStop();

};
