/*                      CameraV4l2.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*   This file is part of:   freeture
*
*   Copyright:      (C) 2014-2015 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
*   Last modified:      17/08/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    CameraV4l2.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    17/08/2015
*/

#pragma once

#include "config.h"

#ifdef LINUX

    #include "opencv2/highgui/highgui.hpp"
    #include <opencv2/imgproc/imgproc.hpp>

    #include <iostream>
    #include <string>
    #include "Frame.h"
    #include "TimeDate.h"
    #include "Camera.h"
    #include <time.h>

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <assert.h>


    #include <fcntl.h>              /* low-level i/o */
    #include <unistd.h>
    #include <errno.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <sys/time.h>
    #include <sys/mman.h>
    #include <sys/ioctl.h>

    #include <linux/videodev2.h>

    #define BOOST_LOG_DYN_LINK 1

    #include "EParser.h"
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
    #include "PixFmtConv.h"
    #include <algorithm>

    using namespace cv;
    using namespace std;

    class CameraV4l2: public Camera {

        private:

            static boost::log::sources::severity_logger< LogSeverityLevel > logger;

            static class Init{

                public:

                    Init(){

                        logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("CameraV4l2"));

                    }

            }initializer;

            const char* mDeviceName;
            int fd;
            double expMin, expMax, exp;
            int gainMin, gainMax, gain;
            int mWidth, mHeight;
            int mFrameCounter;
            struct v4l2_format mFormat;
            bool mCustomSize;

        public :

             void init_userp (unsigned int buffer_size);
             void init_mmap (void);
             void init_read (unsigned int buffer_size);
             int read_frame (void);
             void errno_exit (const char *s);
             int xioctl (int fh, int request, void *arg);


            CameraV4l2();

            ~CameraV4l2();

            bool getInfos();

            vector<pair<int,string>> getCamerasList();

            bool listCameras();

            bool createDevice(int id);

            bool setSize(int width, int height, bool customSize);

            bool grabInitialization();

            void grabCleanse();

            bool acqStart();

            void acqStop();

            bool grabImage(Frame& newFrame);

            bool grabSingleImage(Frame &frame, int camID);

            bool getDeviceNameById(int id, string &device);

            bool getCameraName();

            void getExposureBounds(double &eMin, double &eMax);

            void getGainBounds(int &gMin, int &gMax);

            bool getPixelFormat(CamPixFmt &format);

            bool getFrameSize(int &w, int &h);

            bool getFrameSizeEnum();

            bool getFPS(double &value);

            bool getFpsEnum(vector<double> &values);

            string getModelName();

            double getExposureTime();

            bool setExposureTime(double exp);

            bool setGain(int gain);

            bool setFPS(double fps);

            bool setPixelFormat(CamPixFmt depth);

            void getAvailablePixelFormats();


        private :

            bool convertImage(unsigned char* buffer, Mat &image);

            bool setSize();

    };

#endif
