/*
				CameraSDKPylon.h

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
 * @file    CameraSDKPylon.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    03/07/2014
 */

#include "SaveImg.h"
#include "EnumLog.h"
#include "includes.h"

#ifdef USE_PYLON

#include "CameraSDK.h"
#include "Fifo.h"
#include "Frame.h"
#include "TimeDate.h"
#include "Conversion.h"

// pylon api
#include <pylon/PylonIncludes.h>
#include <pylon/gige/BaslerGigEInstantCamera.h>
#include <pylon/gige/BaslerGigECamera.h>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

using boost::shared_ptr;

using namespace Pylon;
using namespace GenApi;
using namespace cv;
using namespace std;
using namespace logenum;

using namespace Basler_GigECameraParams;

// buffer's number used for grabbing
static const uint32_t nbBuffers = 20;

class CameraSDKPylon : public CameraSDK{

    public:

        /** Severity level enumeration for log file.
         */
//        enum severity_level{
//                normal,         /**< enum normal */
//                notification,   /**< enum notification */
//                warning,        /**< enum warning */
//                fail,           /**< enum fail */
//                critical        /**< enum critical */
//        };

	private:

        //! A logger
        /*!
          Logger used to manage messages added to the log file
        */
		src::severity_logger< severity_level > log;

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

		//! List of basler connected cameras
		DeviceInfoList_t	 devices;

		CBaslerGigECamera::EventGrabber_t	* pEventGrabber;

		IEventAdapter						* pEventAdapter;

		CBaslerGigECamera::StreamGrabber_t	* pStreamGrabber;

		int nbEventBuffers;

		GrabResult result;

		bool connectionStatus;


	public:

		//! Constructor
		CameraSDKPylon();

		//! Destructor
		~CameraSDKPylon(void);

         //! List connected cameras
		void	listCameras(void);

		//! Select a connected device
		bool	chooseDevice(int, string);

		//! Start grabbing images
		int		grabStart();

        //! Stop acquisition
        void	acqStop();

        //! Start acquisition
        void	acqStart();

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
		int		getPixelFormat(void);

		//! Get the width
		int		getWidth(void);

		//! Get the height
		int		getHeight(void);

		//! Get the frame per second rate
		double	getFPS(void);

		//! Get the camera's model name
		string	getModelName();

        //! Set the exposure time
        /*!
          \param exp exposure time
        */
		bool	setExposureTime(double);

		//! Get the exposure time
		double	getExposureTime();

		//! Set the gain
        /*!
          \param gain
        */
		bool	setGain(int);

		//! Get the gain
		int	    getGain();

		//! Set the pixel format
        /*!
          \param format 12 or 8 bits
        */
		bool	setPixelFormat(int PixelFormatEnums);

};

#endif
