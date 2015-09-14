/*
                            CameraGigePylon.h

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
 * \file    CameraGigePylon.cpp
 * \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * \version 1.0
 * \date    03/07/2014
 * \brief   Use Pylon library to pilot GigE Cameras.
 */

#pragma once

#include "config.h"

#ifdef USE_PYLON

    #include "Frame.h"
    #include "TimeDate.h"
    #include "Conversion.h"
    #include "SaveImg.h"
    #include "Camera.h"
    #include <boost/log/common.hpp>
    #include <boost/log/attributes.hpp>
    #include <boost/log/sources/logger.hpp>
    #include <boost/log/core.hpp>
    #include "ELogSeverityLevel.h"

    #include <pylon/PylonIncludes.h>
    #include <pylon/gige/BaslerGigEInstantCamera.h>
    #include <pylon/gige/BaslerGigECamera.h>

    using namespace Pylon;
    using namespace GenApi;
    using namespace cv;
    using namespace std;
    using namespace Basler_GigECameraParams;

    static const uint32_t nbBuffers = 20; // Buffer's number used for grabbing

    class CameraGigePylon : public Camera {

        private :

            static boost::log::sources::severity_logger< LogSeverityLevel > logger;

            static class Init {

                public :

                    Init() {

                        logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("CameraGigePylon"));

                    }

            } initializer;

            Pylon::PylonAutoInitTerm                autoInitTerm;
            uint8_t*                                ppBuffersUC[nbBuffers];         // Buffer for the grabbed images in 8 bits format.
            uint16_t*                               ppBuffersUS[nbBuffers];         // Buffer for the grabbed images in 8 bits format.
            StreamBufferHandle                      handles[nbBuffers];
            CTlFactory                              *pTlFactory;                    // Pointer on the transport layer.
            CBaslerGigECamera                       *pCamera;                       // Pointer on basler camera.
            IPylonDevice                            *pDevice;                       // Pointer on device.
            DeviceInfoList_t                        devices;
            CBaslerGigECamera::EventGrabber_t       *pEventGrabber;
            IEventAdapter                           *pEventAdapter;
            CBaslerGigECamera::StreamGrabber_t      *pStreamGrabber;
            int                                     nbEventBuffers;
            GrabResult                              result;
            bool                                    connectionStatus;
            int                                     mFrameCounter;

        public:

            /**
            * Constructor.
            *
            */
            CameraGigePylon();

            /**
            * Destructor.
            *
            */
            ~CameraGigePylon(void);

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
            * Get camera name from its ID.
            *
            * @param id Identification number of the camera from which the name is required.
            * @param device The camera's name found.
            * @return Success status to find camera's name.
            */
            bool getDeviceNameById(int id, string &device);

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
            bool getFPS(double &value);

            /**
            * Get device's model name.
            *
            * @return Device's model name.
            */
            string getModelName();

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
            * @param fps New fps value.
            * @return Success status to set fps.
            */
            bool setFPS(int fps);

            /**
            * Set device's format.
            *
            * @param format New format.
            * @return Success status to set format.
            */
            bool setPixelFormat(CamBitDepth format);

            /**
            * Get device's exposure time value.
            *
            * @return Device's exposure time.
            */
            int getExposureTime();

    };

#endif
