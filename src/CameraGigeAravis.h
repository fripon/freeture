/*                      CameraGigeAravis.h

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
*   Last modified:      21/01/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    CameraGigeAravis.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    21/01/2015
* \brief   Use Aravis library to pilot GigE Cameras.
*          https://wiki.gnome.org/action/show/Projects/Aravis?action=show&redirect=Aravis
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
    #include "arv.h"
    #include "arvinterface.h"
    #include <time.h>

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

    class CameraGigeAravis: public Camera{

        private:

            static boost::log::sources::severity_logger< LogSeverityLevel > logger;

            static class Init{

                public:

                    Init(){

                        logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("CameraGigeAravis"));

                    }

            }initializer;

            ArvCamera       *camera;                // Camera to control.
            ArvPixelFormat  pixFormat;              // Image format.
            ArvStream       *stream;                // Object for video stream reception.
            int             width;                  // Camera region's width.
            int             height;                 // Camera region's height.
            double          fps;                    // Camera acquisition frequency.
            double          gainMin;                // Camera minimum gain.
            double          gainMax;                // Camera maximum gain.
            unsigned int    payload;                // Width x height.
            double          exposureMin;            // Camera's minimum exposure time.
            double          exposureMax;            // Camera's maximum exposure time.
            const char      *capsString;
            int             gain;                   // Camera's gain.
            double          exp;                    // Camera's exposure time.
            bool            shiftBitsImage;         // For example : bits are shifted for dmk's frames.
            guint64         nbCompletedBuffers;     // Number of frames successfully received.
            guint64         nbFailures;             // Number of frames failed to be received.
            guint64         nbUnderruns;
            int             frameCounter;           // Counter of success received frames.

        public :

            /**
             * Constructor.
             *
             * @param shift Indicates if frame's pixel bits have to be shifted.
             */
            CameraGigeAravis(bool shift);

            /**
             * Constructor.
             *
             */
            CameraGigeAravis();

            /**
             * Destructor.
             *
             */
            ~CameraGigeAravis();

            /**
            * List connected GigE devices.
            *
            */
            bool listGigeCameras();

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

            /**
            * Save genicam file from camera.
            *
            * @param format New format.
            * @return Success status to set format.
            */
            void saveGenicamXml(string p);


    };

#endif
