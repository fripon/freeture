/*                          CameraGigeTis.h

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
* \file    CameraGigeTis.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    21/01/2015
* \brief   Use Imaging source sdk to pilot GigE Cameras.
*/

#pragma once

#include "config.h"

#ifdef WINDOWS

    #include "opencv2/highgui/highgui.hpp"
    #include <opencv2/imgproc/imgproc.hpp>
    #include <iostream>
    #include <string>
    #include "Frame.h"
    #include "TimeDate.h"
    #include "Camera.h"
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
    #include "tisudshl.h"
    #include <algorithm>

    #define NUMBER_OF_BUFFERS 1

    using namespace cv;
    using namespace std;

    class CameraGigeTis: public Camera {

        private:

            static boost::log::sources::severity_logger< LogSeverityLevel > logger;

            static class Init {

                public:

                    Init() {

                        logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("CameraGigeTis"));

                    }

            } initializer;

            DShowLib::Grabber::tVidCapDevListPtr pVidCapDevList;
            DShowLib::tIVCDRangePropertyPtr getPropertyRangeInterface(_DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr& pItems, const GUID& id);
            bool propertyIsAvailable(const GUID& id, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer);
            long getPropertyValue(const GUID& id, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer);
            void setPropertyValue(const GUID& id, long val, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer);
            long getPropertyRangeMin(const GUID& id, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer);
            long getPropertyRangeMax(const GUID& id, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer);

            DShowLib::Grabber* m_pGrabber;
            DShowLib::tFrameHandlerSinkPtr pSink;
            DShowLib::Grabber::tMemBufferCollectionPtr pCollection;
            BYTE* pBuf[NUMBER_OF_BUFFERS];

            int mFrameCounter;
            int mGain;
            int mExposure;
            int mFPS;
            CamBitDepth mImgDepth;
            int mSaturateVal;
            int mGainMin;
            int mGainMax;
            int mExposureMin;
            int mExposureMax;

        public:

            /**
            * Constructor.
            *
            */
            CameraGigeTis();

            /**
            * Destructor.
            *
            */
            ~CameraGigeTis();

            /**
            * List connected GigE devices.
            *
            */
            bool listGigeCameras();

            /**
            * Configure the correct camera to use and grab a frame by single acquisition.
            *
            * @param frame It contains the single frame grabbed by a camera in case of success.
            * @param camID ID of the camera to use.
            * @return The success status of grabbing a single frame.
            */
            bool grabSingleImage(Frame &frame, int camID);

            /**
            * Open/create a device.
            *
            * @param id Identification number of the camera to create.
            */
            bool createDevice(int id);

            /**
            * Set device's format.
            *
            * @param format New format.
            * @return Success status to set format.
            */
            bool setPixelFormat(CamBitDepth format);

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
            * Get device's acquisition frequency.
            *
            * @return Device's fps.
            */
            int getFPS();

            /**
            * Set device's exposure time value.
            *
            * @param value New exposure time value (us).
            * @return Success status to set new exposure time.
            */
            bool setExposureTime(int value);

            /**
            * Set device's gain value.
            *
            * @param value New gain value.
            * @return Success status to set new gain.
            */
            bool setGain(int value);

            /**
            * Set device's acquisition frequency.
            *
            * @param value New fps value.
            * @return Success status to set fps.
            */
            bool setFPS(int value);

            /**
            * Prepare device to grab frames.
            *
            * @return Success status to prepare camera.
            */
            bool grabInitialization();

            /**
            * Run acquisition.
            *
            */
            void acqStart();

            /**
            * Get a frame from continuous acquisition.
            *
            * @param newFrame New frame's container object.
            * @return Success status to grab a frame.
            */
            bool grabImage(Frame &newFrame);

            /**
            * Stop acquisition.
            *
            */
            void acqStop();

            /**
            * Close a device and clean resources.
            *
            */
            void grabCleanse();

            /**
            * Get device's image format.
            *
            * @param format Return image format.
            * @return Success status to get format.
            */
            bool getPixelFormat(CamBitDepth &format);

            /**
            * Get device's exposure time value.
            *
            * @return Device's exposure time.
            */
            int getExposureTime();

    };

#endif
