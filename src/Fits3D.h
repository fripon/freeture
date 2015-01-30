/*
				Fits3D.h

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
*	Last modified:		20/10/2014
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

#include "includes.h"

#ifdef CFITSIO_H
  #include CFITSIO_H
#else
  #include "fitsio.h"
#endif

#include "Configuration.h"
#include "TimeDate.h"
#include "ELogSeverityLevel.h"
#include "Fits.h"
#include "ECamBitDepth.h"

using namespace cv;
using namespace std;

class Fits3D : public Fits{

    private:

        fitsfile        *fptr;
        const char      *filename;
        int             status;
        long            naxis;
        long            naxes[3];
        int             size3d;
        long            fpixel[3];
        int             imgSize;
        CamBitDepth     imgDepth;
        int             n;
        unsigned char   *array3D_MONO_8;
        unsigned short  *array3D_MONO_12;

        bool    printerror      (int status, string errorMsg);
		bool    printerror      (string errorMsg);
		void    printerror      (int status);
		bool    writeKeywords   ();

    public:

        Fits3D  (){};
        ~Fits3D (){};
        Fits3D  (CamBitDepth depth, int imgHeight, int imgWidth, int imgNum, Fits fits);

        void addImageToFits3D   (Mat frame);
        bool writeFits3D        (string file);

};

