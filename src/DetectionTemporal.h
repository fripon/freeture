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
* \brief   Detection method by temporal movement.
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
        vector<Scalar>                  mListColors;            // Each color (B,G,R) is an attribute of a local event.
        Mat                             mLocalMask;             // Mask used to remove single white pixels.
        bool                            mSubdivisionStatus;     // It subdivisions positions have been computed.
        Mat                             mPrevThresholdedMap;
        vector<GlobalEvent>::iterator   mGeToSave;              // Global event to save.
        bool                            mDownsampleEnabled;     // Use downsampling or not      (parameter from configuration file).
        bool                            mSaveGeMap;             // Save GE map                  (parameter from configuration file).
        bool                            mSavePos;               // Save GE positions            (parameter from configuration file).
        bool                            mDebugEnabled;          // Enable or disable debugging  (parameter from configuration file).
        bool                            mSaveDirMap;            // Save GE direction map        (parameter from configuration file).
        bool                            mSaveGeInfos;           // Save GE informations         (parameter from configuration file).
        string                          mDebugPath;             // Debug location data          (parameter from configuration file).
        bool                            mDebugVideo;            // Create a video for debugging (parameter from configuration file).
        string                          mMaskPath;              // Location of the mask to use  (parameter from configuration file).
        bool                            mMaskEnabled;
        VideoWriter                     mVideoDebug;            // Video debug container.
        int                             mRoiSize[2];
        int                             mImgNum;                // Current frame number.
        Mat                             mPrevFrame;             // Previous frame.
        Mat                             mStaticMask;
        Mat                             mOriginalMask;
        Mat                             mHighIntensityMap;      // Map of pixel with high intensity.
        Mat                             mMask;                  // Mask applied to frames.
        bool                            mMaskToCreate;          // Mask must be created.
        string                          mDebugCurrentPath;
        int                             mDataSetCounter;
        bool                            mUpdateMask;
        bool                            mDebugUpdateMask;
        double                          mTimeBeforeEvent;       // Time to keep before an event     (parameter from configuration file).

        boost::circular_buffer<Mat>     mCapBuffer;

        //boost::circular_buffer<Frame> frameBuffer(ACQ_BUFFER_SIZE * ACQ_FPS);
        int                             mCapCounter;
        VideoWriter mVideoDebugAutoMask;

    public :

        /**
        * Constructor.
        *
        */
        DetectionTemporal(double timeBefore, string cfgPath);

        /**
        * Destructor.
        *
        */
        ~DetectionTemporal();

        /**
        * Initialize detection method.
        *
        * @param cfgPath Configuration file path.
        */
        void initMethod(string cfgPath);

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
        int getNumFirstEventFrame() {return (*mGeToSave).getNumFirstFrame();};

        /**
        * Get date of the detected event.
        *
        * @return Date of the event : YYYY-MM-DDTHH:MM:SS,fffffffff
        */
        TimeDate::Date getDateEvent() {return (*mGeToSave).getDate();};

        /**
        * Get frame's number (in frame buffer) of the last frame which belongs to the detected event.
        *
        * @return Frame number.
        */
        int getNumLastEventFrame() {return (*mGeToSave).getNumLastFrame();};

    private :

        /**
        * Initialize debug.
        *
        */
        void createDebugDirectories(bool cleanDebugDirectory);

        /**
        * Select a threshold.
        *
        * @param i Opencv mat image.
        * @return Threshold.
        */
        int selectThreshold(Mat i);

        /**
        * Compute subdivisions position in a frame.
        *
        * @param sub Subdivisions positions container.
        * @param n Number of expected subdivisions.
        * @param imgH Frame's height.
        * @param imgW Frame's width.
        */
        void subdivideFrame(vector<Point> &sub, int n, int imgH, int imgW);

        /**
        * Extract color at given position.
        *
        * @param eventMap Image source where to extract color.
        * @param roiCenter Position where to read color.
        * @return Extracted BGR color.
        */
        vector<Scalar> getColorInEventMap(Mat &eventMap, Point roiCenter);

        /**
        * Color an image region in black.
        *
        * @param p Center of the black region.
        * @param h Height of the black region.
        * @param w Width of the black region.
        * @param region Black region's destination.
        */
        void colorRoiInBlack(Point p, int h, int w, Mat &region);

        /**
        * Create loca events or attach ROI to existing local events.
        *
        * @param region Subdivision where to search.
        * @param frame Thresholded frame.
        * @param eventMap Map of local events.
        * @param listLE List of local events.
        * @param regionPosInFrame Subdivision position in frame.
        * @param maxNbLE Maximum number of local event.
        */
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
                            string &msg);

};

