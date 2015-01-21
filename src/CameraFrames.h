/*
				CameraFrames.h

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
 * @file    CameraFrames.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    02/09/2014
 */

#pragma once

#include "includes.h"
#include "Camera.h"
#include "Conversion.h"

#include "Frame.h"
#include "SaveImg.h"
#include "TimeDate.h"
#include "Conversion.h"
#include "ManageFiles.h"
#include "Fits2D.h"
#include "Fits.h"
#include <list>

//#include "serialize.h"
#include <boost/filesystem.hpp>
#include <iterator>
#include <algorithm>
#include "ELogSeverityLevel.h"
#include "EImgBitDepth.h"

#include <boost/circular_buffer.hpp>

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

//!  Load a video and use it as a camera
class CameraFrames{

	private:

        //! A logger
        /*!
          Logger used to manage messages added to the log file
        */
		src::severity_logger< LogSeverityLevel > log;

		//! Video's location
		string dirPath;

		//! Thread
		boost::thread *thread;



		//! Pointer on a terminated flag
		/*!
          Use to indicate the end of reading the video
        */
		bool *terminatedThread;


		//! Height of the video's frames
		int imgH;

		//! Width of the video's frames
		int imgW;

		int frameStart_;

        int frameStop_;

        int bitpix;

        Fits fitsHeader;

        boost::circular_buffer<Frame> *frameBuffer;
        boost::mutex *m_frameBuffer;
        boost::condition_variable *c_newElemFrameBuffer;

        bool *newFrameDet;
        boost::mutex *m_newFrameDet;
        boost::condition_variable *c_newFrameDet;

	public:

        //! Constructor
        /*!
          \param video_path location of the video
          \param queue pointer on the shared queue
          \param m_mutex_queue mutex on the shared queue
          \param m_cond_queue_fill condition to notify that the queue is full
          \param m_cond_queue_new_element condition to notify that a new frame has been added to the queue
        */
		CameraFrames(string dir,
                    int frameStart,
                    int frameStop,
                    Fits fitsHead,
                    int bitdepth,
                    boost::circular_buffer<Frame> *cb,
                    boost::mutex *m_cb,
                    boost::condition_variable *c_newElemCb,
                    bool *newFrameForDet,
                    boost::mutex *m_newFrameForDet,
                    boost::condition_variable *c_newFrameForDet);

        //! Destructor
		~CameraFrames(void);

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
		int			getWidth                                ();

		//! Get frame's height
		/*!
          \return height
        */
		int			getHeight                               ();
};

