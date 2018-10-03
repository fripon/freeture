/*
                            CameraVideo.h

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
* \file    CameraVideo.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Acquisition thread with video in input.
*/

#pragma once
#include "config.h"

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>

#ifdef LINUX
#define BOOST_LOG_DYN_LINK 1
#endif

#include "Frame.h"
#include "SaveImg.h"
#include "TimeDate.h"
#include "Conversion.h"
#include "Camera.h"
#include <string>
#include <vector>
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
#include <boost/filesystem.hpp>
#include <boost/circular_buffer.hpp>

using namespace boost::filesystem;
using namespace cv;
using namespace std;

class CameraVideo : public Camera{

    private:

        static boost::log::sources::severity_logger< LogSeverityLevel > logger;

        static class Init{

            public:

                Init(){

                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("CameraVideo"));

                }

        }initializer;

        int                 mFrameWidth;
        int                 mFrameHeight;
        VideoCapture        mCap;
        bool                mReadDataStatus;
        int                 mVideoID;
        vector<string>      mVideoList;

    public:

        CameraVideo(vector<string> videoList, bool verbose);

        ~CameraVideo(void);

        bool createDevice(int id);

        bool acqStart() {return true;};

        bool listCameras() {return true;};

        bool grabImage(Frame &img);

        bool grabInitialization();

        bool getStopStatus();

        /**
        * Get data status : Is there another video to use in input ?
        *
        * @return If there is still a video to load in input.
        */
        bool getDataSetStatus();

        /**
        * Load next video if there is.
        *
        * @return Success status to load next data set.
        */
        bool loadNextDataSet(string &location);

        bool getFPS(double &value) {value = 0; return false;};

        bool setExposureTime(double exp){return true;};

        bool setGain(int gain) {return true;};

        bool setFPS(double fps){return true;};

        bool setPixelFormat(CamPixFmt format){return true;};

        bool setSize(int width, int height, bool customSize) {return true;};

};

