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
#include "ECamBitDepth.h"

#include "Conversion.h"
#include "Fifo.h"
#include "Frame.h"
#include "TimeDate.h"
#include "EnumLog.h"
#include "SaveImg.h"
//DMK
#include "glib.h"

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

        // Logger.
		src::severity_logger< severity_level >  log;

        // Shared queue of frames.
		Fifo<Frame>                             *framesQueue;

		// Stop mutex.
		bool                                    *mustStop;
		boost::mutex                            *mustStopMutex;

		// Mutex for the access on the shared queue.
		boost::mutex				            *mutexQueue;

		// Condition if the shared queue is full.
		boost::condition_variable	            *condQueueFill;

		// Condition about new frame in the shared queue.
		boost::condition_variable	            *condQueueNewElement;

		// Acquisition thread for DMK.
		boost::thread                           *m_thread;

        // DMK camera.
		CameraSDK                               *camera;

		CamBitDepth bitpix;

        unsigned int frameCpt;



    public :

        CameraDMK();

		CameraDMK(  Fifo<Frame>                 *queue,
                    boost::mutex                *m_mutex_queue,
                    boost::condition_variable   *m_cond_queue_fill,
                    boost::condition_variable   *m_cond_queue_new_element,
                    boost::mutex                *m_mutex_thread_terminated,
                    bool                        *stop);

        CameraDMK(  int           camExp,
                    int           camGain,
                    CamBitDepth   camDepth);

		~CameraDMK();

        //! List available cameras.
        void	getListCameras();

        //! Select a device.
        bool	setSelectedDevice(string name);

        //! Grab a single frame.
        bool    grabSingleFrame(Mat &frame, string &date);

        //! Wait the end of the acquisition thread.
        void	join();

        //! Acquisition thread operations.
		void	operator()();

		//! Stop the acquisition thread for DMK.
		void	stopThread();

		//! Start the acquisition thread for DMK.
		void	startThread();

		//! Start grabbing frames.
		bool    startGrab();

        //! Stop grabbing frames.
		void    stopGrab(){};

        //! Get width.
		int		getCameraWidth();

		//! Get height.
		int		getCameraHeight();

		bool    getDeviceById(int id, string &device);

        //! Set the pixel format according to type in CamBitDepth enumeration.
		bool	setCameraPixelFormat(CamBitDepth depth);

		//! Set the exposure time.
		bool	setCameraExposureTime(double value);

		//! Set the gain.
		bool	setCameraGain(int value);

		//! Set FPS.
		bool    setCameraFPS(int fps);

};
