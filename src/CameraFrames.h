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
* \file    CameraFrames.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    02/09/2014
* \brief   Acquisition thread with fits individual frames in input.
*/

#pragma once

#include "includes.h"
#include "Conversion.h"
#include "TimeDate.h"
#include "Frame.h"
#include "Fits2D.h"
#include "Fits.h"
#include <list>
#include <boost/filesystem.hpp>
#include "ELogSeverityLevel.h"
#include "EImgBitDepth.h"

#include <boost/circular_buffer.hpp>

using namespace boost::filesystem;

using namespace cv;
using namespace std;

namespace logging	= boost::log;
namespace sinks		= boost::log::sinks;
namespace attrs		= boost::log::attributes;
namespace src		= boost::log::sources;
namespace expr		= boost::log::expressions;
namespace keywords	= boost::log::keywords;

class CameraFrames{

	private:

		src::severity_logger< LogSeverityLevel > log;

		//! Frame's location.
		string dirPath;

		//! Thread.
		boost::thread *thread;

		//! Frame's height.
		int imgH;

		//! Frame's width.
		int imgW;

        //! Separator in the frame's file name.
        int separatorPosition;

        //! Position of the frame's number in the file name.
        string separator;

        //! Frame's bit depth.
        int bitpix;

        Fits fitsHeader;

        boost::circular_buffer<Frame>   *frameBuffer;
        boost::mutex                    *m_frameBuffer;
        boost::condition_variable       *c_newElemFrameBuffer;

        bool                            *newFrameDet;
        boost::mutex                    *m_newFrameDet;
        boost::condition_variable       *c_newFrameDet;

	public:

		CameraFrames(   string                          dir,
                        int                             sepPos,
                        string                          sep,
                        Fits                            fitsHead,
                        int                             bitdepth,
                        boost::circular_buffer<Frame>   *cb,
                        boost::mutex                    *m_cb,
                        boost::condition_variable       *c_newElemCb,
                        bool                            *newFrameForDet,
                        boost::mutex                    *m_newFrameForDet,
                        boost::condition_variable       *c_newFrameForDet);

        //! Destructor
		~CameraFrames(void);

		//! Create thread.
		void startThread();

        //! Thread operations.
		void operator ()();

        //! Wait the end of the thread.
		void join();

		//! Get frame's width.
		int getWidth();

		//! Get frame's height.
		int getHeight();
};

