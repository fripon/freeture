/*
								Fits3D.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
*	Last modified:		20/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
 * @file    Fits3D.cpp
 * @author  Yoan Audureau
 * @version 1.0
 * @date    01/12/2014
 * @section DESCRIPTION
 *
 * Class used to write fits cube with 8 bits unsigned char or 16 bits unsigned short values.
 * Keywords for fits cube have to be defined before to write the fits cube.
 * See settable keywords in the fits class.
 */

#pragma once

#include "includes.h"

#ifdef CFITSIO_H
  #include CFITSIO_H
#else
  #include "fitsio.h"
#endif

#include "Configuration.h"
#include "TimeDate.h"
#include "EnumLog.h"
#include "Fits.h"

using namespace cv;
using namespace std;

using namespace boost::posix_time;

namespace logging	= boost::log;
namespace sinks		= boost::log::sinks;
namespace attrs		= boost::log::attributes;
namespace src		= boost::log::sources;
namespace expr		= boost::log::expressions;
namespace keywords	= boost::log::keywords;

using namespace logenum;

class Fits3D : public Fits{

    private:

		src::severity_logger< severity_level > log;

		int imgW;
        int imgH;
        int imgT;

        fitsfile *fptr;
        const char * filename;

        vector <Mat> *buffer;

    public:

        Fits3D(vector <Mat> *frames);
        ~Fits3D();

        bool writeFits3d8uc     (string file);
        bool writeFits3d16us    (string file);

    private:

        bool    printerror      (int status, string errorMsg);
		bool    printerror      (string errorMsg);
		void    printerror      (int status);
		bool    writeKeywords   ();

};

