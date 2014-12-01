/*
								CameraDMK.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
 * @file    CameraDMK.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    03/06/2014
 */

#pragma once

#include "includes.h"

#include "Camera.h"
#include "CameraSDK.h"

//	#include "CameraSDKAravis.h"

#include "Conversion.h"
#include "Fifo.h"
#include "Frame.h"
#include "TimeDate.h"
#include "EnumLog.h"

//DMK
#include "glib.h"/*
#include "gst/gst.h"
#include "gst/gstbuffer.h"
#include "gst/app/gstappsrc.h"
#include <gst/app/gstappsink.h>*/

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

class CameraDMK: public Camera{

	private:

		src::severity_logger< severity_level > log;			// logger

		Fifo<Frame>                 *framesQueue;
		bool                        *mustStop;
		boost::mutex                *mustStopMutex;
		boost::mutex				*mutexQueue;
		boost::condition_variable	*condQueueFill;
		boost::condition_variable	*condQueueNewElement;
        string cameraName;
        int cameraFormat;

		boost::thread *m_thread;

		CameraSDK *camera;


    public:

        CameraDMK();

		CameraDMK(  string cam,
                    int format,
                    Fifo<Frame> *queue,
                    boost::mutex *m_mutex_queue,
                    boost::condition_variable *m_cond_queue_fill,
                    boost::condition_variable *m_cond_queue_new_element,
                    boost::mutex *m_mutex_thread_terminated,
                    bool *stop);

		~CameraDMK();

        //! Get width
		int		getCameraWidth();

		//! Get height
		int		getCameraHeight();

        //! List connected cameras
		void	getListCameras();

		//! Select a device by id or by name
		bool	setSelectedDevice(int id, string name);

		//! Wait the end of the acquisition thread
		void	join();

        //! Set the pixel format : 8 or 12
		void	setCameraPixelFormat(int depth);

        //! Acquisition thread operations
		void	operator()();

		//! Stop the thread
		void	stopThread();

		//! Start the acquisition thread
		void	startThread();

		//! Set the exposure time
		void	setCameraExposureTime(double value);

		//! Set the gain
		void	setCameraGain(int value);

		/*void    startGrab();

		void    stopGrab();

		void    grabOne();*/

};
