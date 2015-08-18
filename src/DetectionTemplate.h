/*
                            DetectionTemplate.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*   This file is part of:   freeture
*
*   Copyright:      (C) 2014-2015 Yoan Audureau
*                               FRIPON-GEOPS-UPSUD-CNRS
*
*   License:        GNU General Public License
*
*   FreeTure is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*   FreeTure is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*   You should have received a copy of the GNU General Public License
*   along with FreeTure. If not, see <http://www.gnu.org/licenses/>.
*
*   Last modified:      03/03/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    DetectionTemplate.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/03/2015
*/

#pragma once

#include "config.h"

#ifdef LINUX
    #define BOOST_LOG_DYN_LINK 1
#endif

#include <boost/circular_buffer.hpp>
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
#include <boost/tokenizer.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/core.hpp>
#include "ELogSeverityLevel.h"
#include "TimeDate.h"
#include "Fits2D.h"
#include "Fits.h"
#include "Frame.h"
#include "EStackMeth.h"
#include "ECamBitDepth.h"
#include "GlobalEvent.h"
#include "LocalEvent.h"
#include "Detection.h"
#include "EParser.h"
#include "SaveImg.h"
#include <vector>
#include <utility>
#include <iterator>
#include <algorithm>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
using namespace std;
using namespace cv;

class DetectionTemplate : public Detection {

    private :

        static boost::log::sources::severity_logger< LogSeverityLevel > logger;

        static class Init {

            public :

                Init() {

                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("DetectionTemplate"));

                }

        }initializer;

        bool                            mDownsampleEnabled;     // Use downsampling or not      (parameter from configuration file).
        bool                            mSavePos;               // Save GE positions            (parameter from configuration file).
        bool                            mDebugEnabled;          // Enable or disable debugging  (parameter from configuration file).
        string                          mDebugPath;             // Debug location data          (parameter from configuration file).
        bool                            mDebugVideo;            // Create a video for debugging (parameter from configuration file).
        string                          mMaskPath;              // Location of the mask to use  (parameter from configuration file).
        bool                            mMaskEnabled;
        VideoWriter                     mVideoDebug;            // Video debug container.
        int                             mImgNum;                // Current frame number.
        Mat                             mPrevFrame;             // Previous frame.
        Mat                             mMask;                  // Mask applied to frames.
        string                          mDebugCurrentPath;
        int                             mDataSetCounter;
        Mat                             mOriginalMask;
        bool                            mMaskToCreate;

    public :

        /**
        * Constructor.
        *
        */
        DetectionTemplate();

        /**
        * Destructor.
        *
        */
        ~DetectionTemplate();

        /**
        * Initialize detection method.
        *
        * @param cfgPath Configuration file path.
        * @return Success to initialize.
        */
        bool initMethod(string cfgPath);

        /**
        * Run meteor detection.
        *
        * @param c Current frame.
        * @return Success to analysis.
        */
        bool run(Frame &c);

        /**
        * Save infos on the detected event.
        *
        */
        void saveDetectionInfos(string p);

        /**
        * Reset detection method.
        *
        */
        void resetDetection(bool loadNewDataSet);

        /**
        * Reset mask.
        *
        */
        void resetMask();

        /**
        * Get frame's number (in frame buffer) of the first frame which belongs to the detected event.
        *
        * @return Frame number.
        */
        int getNumFirstEventFrame();

        /**
        * Get date of the detected event.
        *
        * @return Date of the event : YYYY-MM-DDTHH:MM:SS,fffffffff
        */
        TimeDate::Date getDateEvent();

        /**
        * Get frame's number (in frame buffer) of the last frame which belongs to the detected event.
        *
        * @return Frame number.
        */
        int getNumLastEventFrame();

    private :

        /**
        * Initialize debug.
        *
        */
        void createDebugDirectories(bool cleanDebugDirectory);

};



