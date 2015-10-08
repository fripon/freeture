/*
                            CameraFrames.h

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
* \file    CameraFrames.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    02/09/2014
* \brief   Fits frames in input of acquisition thread.
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

class CameraFrames: public Camera {

    private:

        static boost::log::sources::severity_logger< LogSeverityLevel > logger;

        static class Init {

            public:

                Init() {

                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("CameraFrames"));

                }

        } initializer;

        bool searchMinMaxFramesNumber(string location);

        vector<string> mFramesDir;  // List of frames directories to process.
        int mNumFramePos;           // Position of the frame number in its filename.
        int mFirstFrameNum;         // First frame number in a directory.
        int mLastFrameNum;          // Last frame number in a directory.
        bool mReadDataStatus;       // Signal the end of reading data in a directory.
        int mCurrDirId;             // Id of the directory to use.
        string mCurrDir;            // Path of the directory to use.

    public:

        /**
         * Constructor.
         *
         * @param locationList Directories which contain set of frames in fits format.
         * @param numPos Position of the frame's number in its filename.
         */
        CameraFrames(vector<string>	locationList, int numPos);

        /**
         * Destructor.
         */
        ~CameraFrames();

        bool createDevice(int id) { return true;};

        bool listCameras() {return true;};

        /**
        * Prepare acquisition on the first directory of fits frames.
        *
        * @return Success status to prepare acquisition.
        */
        bool grabInitialization();

        /**
        * Read the next fits frame in the current directory.
        *
        * @param newFrame New frame's container object.
        * @return Success status to get a frame.
        */
        bool grabImage(Frame &img);

        /**
        * Get status : End to read fits frames in the current directory.
        *
        * @return Reading end status.
        */
        bool getStopStatus();

        /**
        * Load next directory of fits frames if there is.
        *
        * @return Success status to load next data set.
        */
        bool loadNextDataSet(string &location);

        /**
        * Get data status : Is there another directory to use in input ?
        *
        * @return If there is still a directory to load in input.
        */
        bool getDataSetStatus();

        bool getFPS(double &value);

        bool setExposureTime(double exp){cout << "Fake exposure time." << endl; return true;};

        bool setGain(int gain) {cout << "Fake gain." << endl; return true;};

        bool setFPS(double fps){cout << "Fake fps." << endl;return true;};

        bool setPixelFormat(CamBitDepth format){cout << "Fake format." << endl;return true;};

};

