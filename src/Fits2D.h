/*
				Fits2D.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau
*                               FRIPON-GEOPS-UPSUD-CNRS
*
*	License:		GNU General Public License
*
*	FreeTure is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*	FreeTure is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*	You should have received a copy of the GNU General Public License
*	along with FreeTure. If not, see <http://www.gnu.org/licenses/>.
*
*	Last modified:		28/11/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    Fits2D.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    01/12/2014
* \brief   Write/Read fits2D file.
*/

#pragma once
#include "config.h"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>

#ifdef LINUX
#define BOOST_LOG_DYN_LINK 1
#endif

#include "fitsio.h"


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

class Fits2D : public Fits{

	private :

		string fitsPath;

		static boost::log::sources::severity_logger< LogSeverityLevel > logger;
		
		static class _Init{

            public:
                _Init()
                {
                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("Fits2D"));
                }
        } _initializer;

	public:



                Fits2D          (string recPath, Fits fits);

                Fits2D          (string recPath);

                Fits2D          ();

                ~Fits2D         (void);

		bool    writeFits       (Mat img, ImgBitDepth imgType, string fileName);

		bool    readFits32F     (Mat &img, string filePath);
		bool    readFits16US    (Mat &img, string filePath);
		bool    readFits16S     (Mat &img, string filePath);
		bool    readFits8UC     (Mat &img, string filePath);
		bool    readFits8C      (Mat &img, string filePath);

		bool    readIntKeyword  (string filePath, string keyword, int &value);

    private:

        bool    printerror      (int status, string errorMsg);
		bool    printerror      (string errorMsg);
		void    printerror      (int status);
		bool    writeKeywords   (fitsfile *fptr);

};


