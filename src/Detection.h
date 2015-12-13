/*
                                Detection.h

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
* \file    Detection.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Detection interface.
*/

#pragma once

#include "config.h"

#ifdef LINUX
    #define BOOST_LOG_DYN_LINK 1
#endif

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
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
#include <boost/tokenizer.hpp>
#include <vector>
#include <iterator>
#include <algorithm>
#include "TimeDate.h"
#include "Fits2D.h"
#include "Fits.h"
#include "Frame.h"
#include "EStackMeth.h"
#include "ECamPixFmt.h"
#include <boost/filesystem.hpp>
#include "SParam.h"

using namespace boost::filesystem;
using namespace std;
using namespace cv;

class Detection {

    private :

        static boost::log::sources::severity_logger< LogSeverityLevel > logger;

        static class Init {

            public:

                Init() {

                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("Detection"));

                }

        }initializer;

    public:

        virtual ~Detection() {};

        /**
        * Run meteor detection method.
        *
        * @param c Current frame.
        * @return Success to perform analysis.
        */
        virtual bool runDetection(Frame &c) {

            return false;

        };

        /**
        * Get the number of the first detected event's frame.
        *
        * @return Frame number.
        */
        virtual int getEventFirstFrameNb() {

            return 0;

        };

        /**
        * Get the number of the last detected event's frame.
        *
        * @return Frame number.
        */
        virtual int getEventLastFrameNb() {
            
            return 0;
        
        };
        
        /**
        * Get date of the detected event.
        *
        * @return Date of the event : YYYY-MM-DDTHH:MM:SS,fffffffff
        */
        virtual TimeDate::Date getEventDate() {

            TimeDate::Date date;
            return date;

        };

        /**
        * Reset detection method.
        *
        */
        virtual void resetDetection(bool loadNewDataSet){};

        /**
        * Reset mask.
        *
        */
        virtual void resetMask(){};

        /**
        * Save infos on the detected event.
        *
        */
        virtual void saveDetectionInfos(string p, int nbFramesAround){};

        virtual vector<string> getDebugFiles() {vector<string> files; return files;};

};

