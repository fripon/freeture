/*
                            DetectionTemporal.h

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
*   Last modified:      20/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    DetectionTemporal.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Detection method by temporal analysis.
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
#include "ImgProcessing.h"
#include "EStackMeth.h"
#include "ECamPixFmt.h"
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
#include "Mask.h"

using namespace boost::filesystem;
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
using namespace std;
using namespace cv;

class DetectionTemporal : public Detection {

    private :

        static boost::log::sources::severity_logger< LogSeverityLevel > logger;

        static class Init {

            public :

                Init() {

                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("DetectionTemporal"));

                }

        }initializer;

        vector<GlobalEvent>             mListGlobalEvents;      // List of global events (Events spread on several frames).
        vector<Point>                   mSubdivisionPos;        // Position (origin in top left) of 64 subdivisions.
        vector<Scalar>                  mListColors;            // One color per local event.
        Mat                             mLocalMask;             // Mask used to remove isolated white pixels.
        bool                            mSubdivisionStatus;     // If subdivisions positions have been computed.
        Mat                             mPrevThresholdedMap;
        vector<GlobalEvent>::iterator   mGeToSave;              // Global event to save.
        int                             mRoiSize[2];
        int                             mImgNum;                // Current frame number.
        Mat                             mPrevFrame;             // Previous frame.
        Mat                             mStaticMask;
        string                          mDebugCurrentPath;
        int                             mDataSetCounter;
        bool                            mDebugUpdateMask;
        Mask                            *mMaskManager;
        vector<string>                  debugFiles;
        detectionParam                  mdtp;
        VideoWriter                     mVideoDebugAutoMask;

    public :

        DetectionTemporal(detectionParam dp, CamPixFmt fmt);

        ~DetectionTemporal();

        void initMethod(string cfgPath);

        bool runDetection(Frame &c);

        void saveDetectionInfos(string p, int nbFramesAround);

        void resetDetection(bool loadNewDataSet);

        void resetMask();

        int getEventFirstFrameNb() {return (*mGeToSave).getNumFirstFrame();};

        TimeDate::Date getEventDate() {return (*mGeToSave).getDate();};

        int getEventLastFrameNb() {return (*mGeToSave).getNumLastFrame();};

        vector<string> getDebugFiles();

    private :

        void createDebugDirectories(bool cleanDebugDirectory);

        int selectThreshold(Mat i);

        vector<Scalar> getColorInEventMap(Mat &eventMap, Point roiCenter);

        void colorRoiInBlack(Point p, int h, int w, Mat &region);

        void analyseRegion( Mat &subdivision,
                            Mat &absDiffBinaryMap,
                            Mat &eventMap,
                            Mat &posDiff,
                            int posDiffThreshold,
                            Mat &negDiff,
                            int negDiffThreshold,
                            vector<LocalEvent> &listLE,
                            Point subdivisionPos,
                            int maxNbLE,
                            int numFrame,
                            string &msg,
                            TimeDate::Date cFrameDate);



};

