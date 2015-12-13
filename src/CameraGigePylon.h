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
    #include "ECamPixFmt.h"
    #include "EParser.h"
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

            // Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
            // is initialized during the lifetime of this object.
            Pylon::PylonAutoInitTerm                autoInitTerm;

            uint8_t*                                ppBuffersUC[nbBuffers];         // Buffer for the grabbed images in 8 bits format.
            uint16_t*                               ppBuffersUS[nbBuffers];         // Buffer for the grabbed images in 16 bits format.
            StreamBufferHandle                      handles[nbBuffers];
            CTlFactory                              *pTlFactory;
            ITransportLayer                         *pTl;                    // Pointer on the transport layer.
            CBaslerGigECamera                       *pCamera;                       // Pointer on basler camera.
            CBaslerGigECamera::StreamGrabber_t      *pStreamGrabber;
            DeviceInfoList_t                        devices;
            GrabResult                              result;
            bool                                    connectionStatus;
            int                                     mFrameCounter;

        public:

            CameraGigePylon();

            ~CameraGigePylon(void);

            vector<pair<int,string>> getCamerasList();

            bool listCameras();

            bool createDevice(int id);

            bool getDeviceNameById(int id, string &device);

            bool grabInitialization();

            void grabCleanse();

            bool acqStart();

            void acqStop();

            bool grabImage(Frame& newFrame);

            bool grabSingleImage(Frame &frame, int camID);

            void getExposureBounds(double &eMin, double &eMax);

            void getGainBounds(int &gMin, int &gMax);

            bool getPixelFormat(CamPixFmt &format);

            bool getFrameSize(int &w, int &h);

            bool getFPS(double &value);

            string getModelName();

            bool setExposureTime(double exp);

            bool setGain(int gain);

            bool setFPS(double fps);

            bool setPixelFormat(CamPixFmt format);

            double getExposureTime();

            bool setSize(int width, int height, bool customSize);

            void getAvailablePixelFormats();

    };

#endif
