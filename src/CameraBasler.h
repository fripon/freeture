/*
				CameraBasler.h

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
 * @file    CameraBasler.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    03/06/2014
 */

#pragma once

#include "includes.h"
#include "Camera.h"
#include "CameraSDK.h"
#include "Fifo.h"
#include "Frame.h"
#include "TimeDate.h"
#include "Conversion.h"
#include "SaveImg.h"
#include "Fits.h"
#include "Fits2D.h"
#include "Fits3D.h"
#include "ManageFiles.h"
#include "Conversion.h"
#include "EnumLog.h"
#include "EnumBitdepth.h"
#include "ECamBitDepth.h"
//#include "serialize.h"
#include <boost/filesystem.hpp>
#include <iterator>
#include <algorithm>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/zlib.hpp>

using namespace boost::filesystem;

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
using namespace bit_depth_enum;

//! Thread class for acquisition with Basler cameras
class CameraBasler : public Camera{

	private:

        //! Logger.
		src::severity_logger< severity_level > log;

        //! Shared queue of frames.
		Fifo<Frame> *framesQueue;

		//! Stop flag of the thread.
		bool mustStop;

		//! Mutex on the stop flag.
		boost::mutex mustStopMutex;

		//! Mutex on the shared queue.
		boost::mutex *mutexQueue;

		//! Condition if the shared queue is full.
		boost::condition_variable *condQueueFill;

		//! Condition if the shared queue has a new element.
		boost::condition_variable *condQueueNewElement;

        //! Acquisition thread.
		boost::thread *m_thread;

        //! Camera object.
		CameraSDK *camera;

		bool threadStopped;

        //! Exposure value.
		int exposure;

		//! Gain value.
        int gain;

        int fps;

        //! Bit depth.
        CamBitDepth bitdepth;

        //! Path of the configuration file.
        string configFile;

        string savePath;

        unsigned int frameCpt;

	public:

        //! Constructor
        /*!
          \param queue pointer on the shared queue
          \param m_mutex_queue pointer on a mutex used for the shared queue
          \param m_cond_queue_fill pointer on a condition used to notify when the shared queue is full
          \param m_cond_queue_new_element pointer on a condition used to notify when the shared queue has received a new frame
        */
		CameraBasler										(   Fifo<Frame> *queueRam,
                                                                boost::mutex *m_queue,
                                                                boost::condition_variable *c_queueFull,
                                                                boost::condition_variable *c_queueNew,
                                                                int         camExp,
                                                                int         camGain,
                                                                CamBitDepth camDepth,
                                                                int         camFPS);


        CameraBasler( int           camExp,
                      int           camGain,
                      CamBitDepth   camDepth);


		//! Constructor
		CameraBasler();

		//! Destructor
		~CameraBasler(void);

        //! Get width
		int		getCameraWidth();

		//! Get height
		int		getCameraHeight();

		double	getCameraExpoMin();

        //! List connected cameras
		void	getListCameras();

		bool    getDeviceById(int id, string &device);

		//! Select a device by id or by name
		bool	setSelectedDevice(int id, string name);

		bool	setSelectedDevice(string name);

        bool    setCameraFPS(int fps);

		//! Wait the end of the acquisition thread
		void	join();

        //! Set the pixel format : 8 or 12
		bool	setCameraPixelFormat(CamBitDepth depth);

        //! Acquisition thread operations
		void	operator()();

		//! Stop the thread
		void	stopThread();

		//! Start the acquisition thread
		void	startThread();

		//! Set the exposure time
		bool	setCameraExposureTime(double value);

		//! Set the gain
		bool	setCameraGain(int);

		bool    startGrab();

		void    stopGrab();

		bool    grabSingleFrame(Mat &frame, string &date);
};
