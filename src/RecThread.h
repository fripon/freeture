/*
								RecThread.h

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
 * @file    RecThread.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    26/06/2014
 */

#pragma once

#include "includes.h"
#include "RecEvent.h"
#include "Fits.h"
#include "Fits2D.h"
#include "Fits3D.h"
#include "Conversion.h"
#include "Frame.h"
#include "ManageFiles.h"
#include "SaveImg.h"
#include "TimeDate.h"
#include "EnumLog.h"
//#include "serialize.h"
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

//!  A thread class to record detected events
/*!
  This class manage a thread used to record detected events.
*/
class RecThread{

	private:

        //! A logger
        /*!
          Logger used to manage messages added to the log file
        */
		src::severity_logger< severity_level > log;

		bool threadStopped;

		//! A thread
        /*!
          This class has a proper thread
        */
		boost::thread *recThread;

		//! A shared list
        /*!
          This is a buffer that contains the events to record
        */
		vector<RecEvent> *listEventToRec;

		//! Mutex on the shared list
        /*!
          This is a mutex used to synchronise access to the list shared.
        */
		boost::mutex *mutex_listEventToRec;

		//! Condition
		boost::condition_variable *condNewElem;

		//! Path record
		string recPath;

		//! Flag to stop the thread
        /*!
          Flag used to stop the thread
        */
		bool mustStop;

		//! Mutex on the stop flag
		boost::mutex mustStopMutex;

        //! Enable to record video
		bool recAvi;

        //! Enable to record fits 3D
		bool recFits3D;

		//! Enable to recort fits 2D
		bool recFits2D;

        //! Enable to record a file of positions
		bool recPositionFile;

        //! Enable to record bmp for each frame
		bool recBmp;

        //! Enable to record the event's shape
		bool recShape;

		//! Enable to record the trail
		/*!
          This a sum of consecutive frame difference
        */
		bool recTrail;

		//! Enable to record the map of the globalEvent
		bool recMapGE;

		int pixelFormat;

		Fits fitsHeader;

	public:

        //! Constructor
        /*!
          \param recpath location of the recording
          \param sharedList pointer on the shared list which contains the events to record
          \param sharedListMutex pointer on a mutex used for the shared list
          \param sharedListMutex pointer on a condition used to notify when the shared list has a new element
        */
        RecThread(  string recpath,
                    vector<RecEvent> *sharedList,
                    boost::mutex *sharedListMutex,
                    boost::condition_variable *condition,
                    int pixFormat,
                    bool avi,
                    bool fits3D,
                    bool fits2D,
                    bool positionFile,
                    bool bmp,
                    bool trail,
                    bool shape,
                    bool mapGE,
                    Fits fitsHead );


        //! Destructor
        ~RecThread( void );

        //! Create a thread
		void    start();

		//! Stop the thread
		void    stop();

        //! Thread operations
		void    operator()();

};

