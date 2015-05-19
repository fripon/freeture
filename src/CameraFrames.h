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
#include "config.h"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>


#ifdef LINUX
#define BOOST_LOG_DYN_LINK 1
#endif

#include <boost/log/sources/logger.hpp>
#include "ELogSeverityLevel.h"
#include "Conversion.h"
#include "TimeDate.h"
#include "Frame.h"
#include "Fits2D.h"
#include "Fits.h"
#include <list>
#include <iterator>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>

#include "Camera.h"

using namespace boost::posix_time;
using namespace cv;
using namespace std;

class CameraFrames: public Camera{

	private:

		static boost::log::sources::severity_logger< LogSeverityLevel > logger;

		static class _Init{

            public:
                _Init()
                {
                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("CameraFrames"));
                }
        } _initializer;

		//! Frame's location.
		vector<string> framesDir;

        //! Separator in the frame's file name.
        int numPosInName;

		int firstNumFrame;
		int lastNumFrame;

		bool endReadDataStatus;


		int framesetID;

        bool extractFrameNumbers(string location);

        string currentFramesFir;

	public:

		CameraFrames(vector<string>	dir, int nbPos);

		~CameraFrames();

		bool grabStart();

		bool grabImage(Frame &img);

		bool getStopStatus();

		bool loadData();

		bool getDataStatus();


};

