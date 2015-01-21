/*
								DetThread.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
#include "ELogSeverityLevel.h"

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
#include "ECamBitDepth.h"
#include "EDetMeth.h"
#include <boost/circular_buffer.hpp>

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

#define DEG2RAD 0.017453293f

class DetThread{

	private:

        src::severity_logger< LogSeverityLevel > log;			// logger

        boost::thread				*m_thread;				// The thread runs this object

        bool						mustStop;				// Used to stop detection thread

        boost::mutex				mustStopMutex;			// Mutex for access mustStop flag

        int							detMeth;				// Indicates the detection method wanted

        Mat                         mask;

        CamBitDepth                 imgFormat;

        vector<GlobalEvent>         listGlobalEvents;

        int                         nbDet;

        int                         timeMax;

        int                         nbGE;

        int                         timeAfter;
        int                         frameBufferMaxSize;

        string                      recordingPath;

        string                      station;

        bool                        debug;

        bool                        downsample;

        string                      debugLocation;

        Fits fitsHeader;

        Mat prevthresh;

        DetMeth mthd;

        boost::circular_buffer<Frame>   *frameBuffer;
        boost::mutex                    *m_frameBuffer;
        boost::condition_variable       *c_newElemFrameBuffer;

        bool                            *newFrameDet;
        boost::mutex                    *m_newFrameDet;
        boost::condition_variable       *c_newFrameDet;

        RecEvent                        *eventToRec;

	public:

        DetThread(   Mat                            maskImg,
                     int                            mth,
                     CamBitDepth                    acqFormatPix,
                     DetMeth                        detMthd,
                     int                            geAfterTime,
                     int                            bufferSize,
                     int                            geMax,
                     int                            geMaxTime,
                     string                         recPath,
                     string                         stationName,
                     bool                           detDebug,
                     string                         debugPath,
                     bool                           detDownsample,
                     Fits                           fitsHead,
                     boost::circular_buffer<Frame>  *cb,
                     boost::mutex                   *m_cb,
                     boost::condition_variable      *c_newElemCb,
                     bool                           *newFrameForDet,
                     boost::mutex                   *m_newFrameForDet,
                     boost::condition_variable      *c_newFrameForDet,
                     RecEvent                       *recEvent);


		~DetThread();

		void operator()();

        void startDetectionThread();
		void stopDetectionThread();
		void join();

};


