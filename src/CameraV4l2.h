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

    #include "ECamBitDepth.h"
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

    using namespace cv;
    using namespace std;

   // #define CLEAR(x) memset(&(x), 0, sizeof(x))



    class CameraV4l2: public Camera {

        private:

            static boost::log::sources::severity_logger< LogSeverityLevel > logger;

            static class Init{

                public:

                    Init(){

                        logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("CameraV4l2"));

                    }

            }initializer;

            const char* deviceName;
            int fd;
            int expMin, expMax;
            int gainMin, gainMax;
        public :

             void init_userp (unsigned int buffer_size);
             void init_mmap (void);
             void init_read (unsigned int buffer_size);
             int read_frame (void);
             void errno_exit (const char *s);
             int xioctl (int fh, int request, void *arg);

            /**
             * Constructor.
             *
             */
            CameraV4l2();

            /**
             * Destructor.
             *
             */
            ~CameraV4l2();

            /**
            * Get device's informations.
            *
            */
            bool getInfos();

            /**
            * List connected GigE devices.
            *
            */
            bool listCameras();

            /**
            * Open/create a device.
            *
            * @param id Identification number of the device to create.
            */
            bool createDevice(int id);

            /**
            * Prepare device to grab frames.
            *
            * @return Success status to prepare camera.
            */
            bool grabInitialization();

            /**
            * Close a device and clean resources.
            *
            */
            void grabCleanse();

            /**
            * Run acquisition.
            *
            */
            void acqStart();

            /**
            * Stop acquisition.
            *
            */
            void acqStop();

            /**
            * Get a frame from continuous acquisition.
            *
            * @param newFrame New frame's container object.
            * @return Success status to grab a frame.
            */
            bool grabImage(Frame& newFrame);

            /**
            * Configure the correct camera to use and grab a frame by single acquisition.
            *
            * @param frame It contains the single frame grabbed by a camera in case of success.
            * @param camID ID of the camera to use.
            * @return The success status of grabbing a single frame.
            */
            bool grabSingleImage(Frame &frame, int camID);

            /**
            * Get camera name from its ID.
            *
            * @param id Identification number of the camera from which the name is required.
            * @param device The camera's name found.
            * @return Success status to find camera's name.
            */
            bool getDeviceNameById(int id, string &device);

            /**
            * Get device's exposure time bounds.
            *
            * @param eMin Return minimum exposure time value.
            * @param eMax Return maximum exposure time value.
            */
            void getExposureBounds(int &eMin, int &eMax);

            /**
            * Get device's gain bounds.
            *
            * @param gMin Return minimum gain value.
            * @param gMax Return maximum gain value.
            */
            void getGainBounds(int &gMin, int &gMax);

            /**
            * Get device's image format.
            *
            * @param format Return image format.
            * @return Success status to get format.
            */
            bool getPixelFormat(CamBitDepth &format);

            /**
            * Get device's frame width.
            *
            * @return Frame's width.
            */
            int getFrameWidth();

            /**
            * Get device's frame height.
            *
            * @return Frame's height.
            */
            int getFrameHeight();

            /**
            * Get device's acquisition frequency.
            *
            * @return Device's fps.
            */
            int getFPS();

            /**
            * Get device's model name.
            *
            * @return Device's model name.
            */
            string getModelName();

            /**
            * Get device's exposure time value.
            *
            * @return Device's exposure time.
            */
            int getExposureTime();

            /**
            * Set device's exposure time value.
            *
            * @param value New exposure time value (us).
            * @return Success status to set new exposure time.
            */
            bool setExposureTime(int exp);

            /**
            * Get device's gain value.
            *
            * @return Device's gain.
            */
            bool setGain(int gain);

            /**
            * Set device's acquisition frequency.
            *
            * @param value New fps value.
            * @return Success status to set fps.
            */
            bool setFPS(int fps);

            /**
            * Set device's format.
            *
            * @param format New format.
            * @return Success status to set format.
            */
            bool setPixelFormat(CamBitDepth depth);

    };

#endif
