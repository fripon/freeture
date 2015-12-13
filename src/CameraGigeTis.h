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
    #include "EParser.h"
    #include "ECamPixFmt.h"
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
            double mExposure;
            double mFPS;
            CamPixFmt mImgDepth;
            int mSaturateVal;
            int mGainMin;
            int mGainMax;
            int mExposureMin;
            int mExposureMax;

        public:

            CameraGigeTis();

            ~CameraGigeTis();

            vector<pair<int,string>> getCamerasList();

            bool grabSingleImage(Frame &frame, int camID);

            bool createDevice(int id);

            bool setPixelFormat(CamPixFmt format);

            void getExposureBounds(double &eMin, double &eMax);

            void getGainBounds(int &gMin, int &gMax);

            bool getFPS(double &value);

            bool setExposureTime(double value);

            bool setGain(int value);

            bool setFPS(double value);

            bool setFpsToLowerValue();

            bool grabInitialization();

            bool acqStart();

            bool grabImage(Frame &newFrame);

            void acqStop();

            void grabCleanse();

            bool getPixelFormat(CamPixFmt &format);

            double getExposureTime();

            bool setSize(int width, int height, bool customSize);

            void getAvailablePixelFormats();

    };

#endif
