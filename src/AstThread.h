/*
								AstThread.h

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
 * @file    AstThread.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    03/06/2014
 */

#pragma once

#include "includes.h"
#include "EnumLog.h"
#include "Fifo.h"
#include "Frame.h"
#include "Fits2D.h"
#include "Histogram.h"
#include "TimeDate.h"

#include <boost/filesystem.hpp>
#include <iterator>
#include <algorithm>

using namespace boost::filesystem;

using namespace std;
using namespace cv;
using namespace boost::posix_time;

namespace logging	= boost::log;
namespace sinks		= boost::log::sinks;
namespace attrs		= boost::log::attributes;
namespace src		= boost::log::sources;
namespace expr		= boost::log::expressions;
namespace keywords	= boost::log::keywords;

using namespace logenum;

//!  A thread class to make captures regurlarly in fits format.
/*!
  This class launchs a thread to make regular captures in fits format for astrometry.
*/
class AstThread{

	private:

        //! A logger
        /*!
          Logger used to manage messages added to the log file
        */
		src::severity_logger< severity_level > log;

		//! A thread
        /*!
          This class has a proper thread
        */
		boost::thread               *thread;

		//! A shared queue
        /*!
          This is a buffer shared between threads.
          It contains the n last frames captured by the acquisition process.
        */
		Fifo<Frame>                 *framesQueue;

		//! Mutex on the shared queue
        /*!
          This is a mutex used to synchronise access to the buffer shared.
        */
		boost::mutex                *m_mutex_queue;

		//! Condition on the shared queue if it's full
        /*!
          Used to check if the shared queue is full
        */
		boost::condition_variable   *condFill;

		//! Condition on the shared queue if there is new frame
        /*!
          Used to check if a new element has been added in the shared queue
        */
		boost::condition_variable   *condNewElem;

		//! Path of images captured
        /*!
          Define the path where the fits files will be save
        */
		string                      path_;

		bool threadStopped;

		//! Capture time interval
        /*!
          Time interval between captures
        */
		int                         capInterval;

		//! Exposure time used for capture images
        /*!
          This is the maximum value of the camera's exposure time to be at the maximum frame per second rate.
          This value is configurate in the configuration file
        */
		double                      exposureTime;

		//! Flag to stop the thread
        /*!
          Flag used to stop the thread
        */
		bool                        mustStop;

		//! Mutex on the stop flag
		boost::mutex                mustStopMutex;

		//! Format of the captures
        /*!
          Is define in the configuration file.
        */
		int formatPixel ;

		//! Path of the configuration file
        /*!
          Is define in argument of the program
        */
		string                      configFile;

		//! Method of the fits creation
        /*!
          Is define in the configuration file. To generate a fits, N frames are integrated during n seconds.
          This parameter is used to decide if we want to conserve the integration or if we want to do a mean to generate the final image.
        */
		string                      fitsMethod;

		//! Longitude of the station's position
		double longitude;

		string stationName;

	public:

        //! Constructor
        /*!
          \param recPath location of the captured images
          \param astMeth mean or sum
          \param configurationFilePath location of the configuration file
          \param frame_queue pointer on the shared queue which contains last grabbed frames
          \param interval capture's time interval in seconds
          \param expTime integration time
          \param acqFormat format of grabbed image
          \param m_frame_queue pointer on a mutex used for the shared queue
          \param c_queue_full pointer on a condition used to notify when the shared queue is full
          \param c_queue_new pointer on a condition used to notify when the shared queue has received a new frame
        */

        AstThread(  string recPath,
                    string station,
                    string astMeth,
                    string configurationFilePath,
                    Fifo<Frame> *frame_queue,
                    int interval,
                    double expTime,
                    int acqFormat,
                    double longi,
                    boost::mutex *m_frame_queue,
                    boost::condition_variable *c_queue_full,
                    boost::condition_variable *c_queue_new);

        //! Destructor
        ~AstThread(void);

        //! Create a thread
		void startCapture();

		//! Stop the thread
		void stopCapture();

        //! Thread operations
		void operator()();

};
