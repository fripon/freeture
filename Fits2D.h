/*
                                Fits2D.h

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
*   Last modified:      20/07/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    Fits2D.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    14/06/2015
* \brief   Write/Read fits2D file.
*/

#pragma once

#include "config.h"

#ifdef LINUX
    #define BOOST_LOG_DYN_LINK 1
#endif

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#ifdef CFITSIO_H
  #include CFITSIO_H
#else
  #include "fitsio.h"
#endif
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
#include "TimeDate.h"
#include "EImgBitDepth.h"
#include "Fits.h"
#include "Conversion.h"

using namespace std;
using namespace boost::posix_time;

class Fits2D : public Fits {

    private :

        // Location where to save fits or path of a fits file to read.
        string mFitsPath;

        static boost::log::sources::severity_logger< LogSeverityLevel > logger;

        static class Init {

            public :

                Init() {

                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("Fits2D"));

                }

        }initializer;

    public :

        /**
        * Constructor.
        *
        */
        Fits2D(string path);

        /**
        * Destructor.
        *
        */
        ~Fits2D(void);

        /**
        * Create and write Fits image.
        * @param img Image to save in fits format.
        * @param imgType Format of the image (8 bits signed/unsigned char, 16 bits signed/unsigned char, 32 bits float).
        * @param fileName Optional parameter to specify a name of the fits file.
        * @return Success status to create and write the file.
        *
        */
        bool writeFits(Mat img, ImgBitDepth imgType, string fileName, string compression = "");

        /**
        * Read a Fits file in 32 bits float format.
        * @param img Reference on the container which will contain the read fits.
        *
        */
        bool readFits32F(Mat &img);

        /**
        * Read a Fits file in 16 bits unsigned char format.
        * @param img Reference on the container which will contain the read fits.
        *
        */
        bool readFits16US(Mat &img);

        /**
        * Read a Fits file in 16 bits signed char format.
        * @param img Reference on the container which will contain the read fits.
        *
        */
        bool readFits16S(Mat &img);

        /**
        * Read a Fits file in 8 bits unsigned char format.
        * @param img Reference on the container which will contain the read fits.
        *
        */
        bool readFits8UC(Mat &img);

        /**
        * Read a Fits file in 8 bits signed char format.
        * @param img Reference on the container which will contain the read fits.
        *
        */
        bool readFits8C(Mat &img);

        /**
        * Read a keyword in integer type.
        * @param keyword Keyword name.
        * @param value Reference on the found keyword's value.
        *
        */
        bool readIntKeyword(string keyword, int &value);

        /**
        * Read a keyword in string type.
        * @param keyword Keyword name.
        * @param value Reference on the found keyword's value.
        *
        */
        bool readStringKeyword(string keyword, string &value);

        /**
        * Read a keyword in double type.
        * @param keyword Keyword name.
        * @param value Reference on the found keyword's value.
        *
        */
        bool readDoubleKeyword(string keyword, double &value);

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
        bool writeKeywords(fitsfile *fptr);

};
