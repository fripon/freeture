/*
								CameraVideo.h

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
 * @file    CameraVideo.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    03/06/2014
 * @section DESCRIPTION
 *
 *          A video is used in entry as a camera which captures images
 */

#pragma once

#include "includes.h"
#include "Camera.h"
#include "Fifo.h"
#include "Frame.h"
#include "SaveImg.h"
#include "TimeDate.h"
#include "Conversion.h"
#include "ManageFiles.h"
#include "EnumLog.h"
//#include "serialize.h"
#include <boost/filesystem.hpp>
#include <iterator>
#include <algorithm>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/zlib.hpp>

using namespace boost::filesystem;

using namespace cv;
using namespace std;

namespace logging	= boost::log;
namespace sinks		= boost::log::sinks;
namespace attrs		= boost::log::attributes;
namespace src		= boost::log::sources;
namespace expr		= boost::log::expressions;
namespace keywords	= boost::log::keywords;

using namespace logenum;

//!  Load a video and use it as a camera
class CameraVideo : public Camera{

	private:

        //! A logger
        /*!
          Logger used to manage messages added to the log file
        */
		src::severity_logger< severity_level > log;

		//! Video's location
		string videoPath;

		//! Thread
		boost::thread *thread;

		//! Pointer on the shared queue
        /*!
          The thread of this class will push frames in the shared queue
        */
		Fifo<Frame> *frameQueue;

		//! Pointer on a terminated flag
		/*!
          Use to indicate the end of reading the video
        */
		bool *terminatedThread;

		//! Mutex on the shared queue
		boost::mutex				*mutexQueue;

		//! Condition to notify that the queue is full
		boost::condition_variable	*condQueueFill;

		//! Condition to notify that a new frame has been added to the queue
		boost::condition_variable	*condQueueNewElement;

		//! Height of the video's frames
		int imgH;

		//! Width of the video's frames
		int imgW;

		VideoCapture cap;

	public:

        //! Constructor
        /*!
          \param video_path location of the video
          \param queue pointer on the shared queue
          \param m_mutex_queue mutex on the shared queue
          \param m_cond_queue_fill condition to notify that the queue is full
          \param m_cond_queue_new_element condition to notify that a new frame has been added to the queue
        */
		CameraVideo(string video_path,
                    Fifo<Frame> *queue,
                    boost::mutex *m_mutex_queue,
                    boost::condition_variable *m_cond_queue_fill,
                    boost::condition_variable *m_cond_queue_new_element);

        //! Destructor
		~CameraVideo(void);

		//! Create thread
		void startThread();

        //! Thread operations
		void operator ()();

        //! Wait the end of the thread
		void join();

		//! Get frame's width
		/*!
          \return width
        */
		int			getCameraWidth                                ();

		//! Get frame's height
		/*!
          \return height
        */
		int			getCameraHeight                               ();
};

