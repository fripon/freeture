/*
								DetThread.h

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
 * @file    DetThread.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    13/06/2014
 */

#pragma once

#include "includes.h"
#include "EnumLog.h"
#include "Fifo.h"
#include "Frame.h"
#include "Fits.h"
#include "Fits2D.h"
#include "Fits3D.h"
#ifdef CFITSIO_H
  #include CFITSIO_H
#else
  #include "fitsio.h"
#endif
#include "Conversion.h"
#include "GlobalEvent.h"
#include "LocalEvent.h"
#include "PixelEvent.h"
#include "RecEvent.h"
#include "DetByLines.h"
#include "DetByLists.h"

//#include "serialize.h"
#include <boost/filesystem.hpp>
#include <iterator>
#include <algorithm>

using namespace boost::filesystem;

using namespace cv;

using namespace std;

using namespace boost::posix_time;

namespace logging	= boost::log;
namespace sinks		= boost::log::sinks;
namespace attrs		= boost::log::attributes;
namespace src		= boost::log::sources;
namespace expr		= boost::log::expressions;
namespace keywords	= boost::log::keywords;

using namespace logenum;

#define DEG2RAD 0.017453293f

class DetThread{

	private:

        src::severity_logger< severity_level > log;			// logger

        boost::thread				*m_thread;				// The thread runs this object

        bool						mustStop;				// Used to stop detection thread

        boost::mutex				mustStopMutex;			// Mutex for access mustStop flag

        int							detMeth;				// Indicates the detection method wanted

        Fifo<Frame>					*framesQueue;			// Shared queue between process that contains grabbed frames

        boost::mutex				*mutexQueue;

        boost::condition_variable	*condQueueFill;

        boost::condition_variable	*condQueueNewElement;

        boost::condition_variable	*condNewElemOn_ListEventToRec;

        boost::mutex				*mutex_listEvToRec;

        vector<RecEvent>            *listEvToRec;

        Mat                         mask;

        int                         imgFormat;

        vector <GlobalEvent>        listGlobalEvents;

        int                         nbDet;

        int                         geMaxDuration;

        int                         geMaxInstance;

        int                         geAfterDuration;

        string                      recordingPath;

        string                      station;

        //bool threadStopped;

        bool                        debug;

        bool                        maskMoon;

        bool                        downsample;

        string                      debugLocation;

        bool                        maskMoonSave;

        Fits fitsHeader;

	public:

        DetThread(   Mat                        maskImg,
                     int                        mth,
                     int                        acqFormatPix,
                     Fifo<Frame>                *queue,
                     boost::mutex               *m_mutex_queue,
                     boost::condition_variable  *m_cond_queue_fill,
                     boost::condition_variable  *m_cond_queue_new_element,
                     boost::condition_variable  *newElem_listEventToRecord,
                     boost::mutex               *mutex_listEventToRecord,
                     vector<RecEvent>           *listEventToRecord,
                     int                        geAfterTime,
                     int                        geMax,
                     int                        geMaxTime,
                     string                     recPath,
                     string                     stationName,
                     bool                       detDebug,
                     string                     debugPath,
                     bool                       detMaskMoon,
                     bool                       saveMaskedMoon,
                     bool                       detDownsample,
                     Fits fitsHead
                 );


		~DetThread( void );

		void join();

		void startDetectionThread();

		void operator()();

		void stopDetectionThread();

};


