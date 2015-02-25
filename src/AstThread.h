/*
								AstThread.h

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
* \file    AstThread.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    21/01/2015
* \brief   Produce a thread for stacking frames. Save stacks which will be
*          use to make astrometry.
*/

#pragma once

#include "config.h"

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>

#ifdef LINUX
#define BOOST_LOG_DYN_LINK 1
#endif

#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/record_ordering.hpp>
#include <boost/log/core.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <iostream>
#include "ELogSeverityLevel.h"
#include "EImgBitDepth.h"
#include "ECamBitDepth.h"
#include "EStackMeth.h"

#include "Frame.h"
#include "Fits.h"
#include "Fits2D.h"
#include "TimeDate.h"
#include "ImgReduction.h"
#include "StackedFrames.h"

#include <boost/filesystem.hpp>
#include <boost/circular_buffer.hpp>

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

class AstThread{

	private:

		src::severity_logger< LogSeverityLevel > log;

		//! Thread declaration.
		boost::thread *thread;

		//! Location where to save stacks.
		string path_;

		bool threadStopped;

		//! Interval between two stacks (In number of frames).
		int capInterval;

		//! Time of stacking frames (In number of frames to stack).
		double exposureTime;

		//! Flag used to stop the thread.
		bool mustStop;

		//! Mutex on the stop flag.
		boost::mutex mustStopMutex;

		//! Bit depth of frames.
		CamBitDepth camFormat ;

		//! Location of the configuration file.
		string configFile;

		//! Method to use to get the final stacked frame.
		StackMeth stackMthd;

		//! Longitude of the station's position.
		double longitude;

        //! Name of the station.
		string stationName;

        //! Contain fits keywords defined in the configuration file.
		Fits fitsHeader;

        //! Camera's fps.
		int camFPS;

        //! If it's disabled, the final stacked frame will be saved in 32 bits.
        //! If it's enabled, it will be saved in 16 bits.
		bool stackReduction;

		boost::circular_buffer<StackedFrames>   *stackedFramesBuffer;
        boost::mutex                            *m_stackedFramesBuffer;
        boost::condition_variable               *c_newElemStackedFramesBuffer;

	public:

        AstThread(  string                                  recPath,
                    string                                  station,
                    StackMeth                               astMeth,
                    string                                  configurationFilePath,
                    double                                  expTime,
                    CamBitDepth                             acqFormat,
                    double                                  longi,
                    Fits                                    fitsHead,
                    int                                     fps,
                    bool                                    reduction,
                    boost::circular_buffer<StackedFrames>   *stackedFb,
                    boost::mutex                            *m_stackedFb,
                    boost::condition_variable               *c_newElemStackedFb);

        ~AstThread(void);

        //! Create a thread.
		void startThread();

		//! Stop the thread.
		void stopThread();

        //! Thread operations.
		void operator()();

};
