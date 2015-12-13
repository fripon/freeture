/*
                                Fits3D.h

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
* \file    Fits3D.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    01/12/2014
* \brief   Write fits3D file.
*/

#pragma once

#include "config.h"

#ifdef LINUX
#define BOOST_LOG_DYN_LINK 1
#endif

#ifdef CFITSIO_H
  #include CFITSIO_H
#else
  #include "fitsio.h"
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
#include "CfgLoader.h"
#include "TimeDate.h"
#include "Fits.h"
#include "ECamPixFmt.h"

using namespace cv;
using namespace std;

class Fits3D : public Fits {

    private :

        static boost::log::sources::severity_logger< LogSeverityLevel > logger;

        static class Init {

            public :

                Init() {

                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("Fits3D"));

                }

        }initializer;

        fitsfile        *fptr;
        const char      *mFileName;
        int             status;
        long            naxis;
        long            naxes[3];
        int             size3d;
        long            fpixel[3];
        int             imgSize;
        CamPixFmt       imgDepth;
        int             n;                  // Index for the number of images
        unsigned char   *array3D_MONO_8;
        unsigned short  *array3D_MONO_12;

    public :

        /**
        * Constructor.
        * @param depth Image format.
        * @param imgHeight Height of images.
        * @param imgWidth Width of images.
        * @param numberOfImages Number of images to add in the fits cube.
        * @param fileName Name od the fits cube.
        *
        */
        Fits3D(CamPixFmt depth, int imgHeight, int imgWidth, int numberOfImages, string fileName);

        /**
        * Constructor.
        *
        */
        Fits3D():
        fptr(NULL), mFileName("noFileName"), status(0), naxis(3), size3d(0), imgSize(0),
        imgDepth(MONO8), n(0), array3D_MONO_12(NULL), array3D_MONO_8(NULL) {

        };

        /**
        * Destructor.
        *
        */
        ~Fits3D(){};

        void addImageToFits3D(Mat frame);

        /**
        * Create and write fits 3D.
        * @param status Error cfitsio status.
        *
        */
        bool writeFits3D();

    private :

        /**
        * Helper function to get cfitsio error.
        * @param status Error cfitsio status.
        * @param errorMsg Additional information about where the error occured.
        *
        */
        void printerror(int status, string errorMsg);

        /**
        * Helper function to get cfitsio error.
        * @param status Error cfitsio status.
        *
        */
        void printerror(int status);

        /**
        * Write keywords in fits.
        * @param fptr Pointer on the fits.
        * @return Success to write keywords.
        *
        */
        bool writeKeywords();


};

