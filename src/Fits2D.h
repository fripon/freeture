/*
								Fits2D.h

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
 * @file    Fits2D.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    13/06/2014
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
//using namespace Pylon;
//using namespace GenApi;
using namespace cv;
using namespace std;
//using namespace Basler_GigECameraParams;
using namespace boost::posix_time;

namespace logging	= boost::log;
namespace sinks		= boost::log::sinks;
namespace attrs		= boost::log::attributes;
namespace src		= boost::log::sources;
namespace expr		= boost::log::expressions;
namespace keywords	= boost::log::keywords;

using namespace logenum;

class Fits2D{

	private :

		src::severity_logger< severity_level > log;

		string savedFitsPath;

		//Keywords values
		string vFILENAME;
		double vEXPOSURE;
		string vTELESCOP;
		string vOBSERVER;
		string vINSTRUME;
		string vCAMERA;
		double vFOCAL;
		string vFILTER;
		string vPROGRAM;
		string vCREATOR;
		double vAPERTURE;
		double vXPIXEL;
		double vYPIXEL;
		double vSITELONG;
		double vSITELAT;
		double vSITEELEV;
		int    vGAINDB;
		string vCENTAZ;
		string vCENTALT;
		string vCENTOR;
		int    vCRPIX1;
		int    vCRPIX2;
		double vK1;
		double vK2;
		string vCOMMENT;
		double vCD1_1;
		double vCD1_2;
		double vCD2_1;
		double vCD2_2;
		int    vOBSMODE;
		string vDATEOBS;
		double vSATURATE;
		double vONTIME;
		string vRADESYS;
		double vEQUINOX;
		string vCTYPE1;
		string vCTYPE2;
		int vELAPTIME;
		double CRVAL2;
		double vCRVAL1;

	public:

		            Fits2D                      (string recPath,
                                                 int    ontime,
                                                 string dateObs,
                                                 int    elaptime,
                                                 int    fps,
                                                 double saturate,
                                                 double exposure,
                                                 int    gain,
                                                 double sideralTime);

                    Fits2D();

		            ~Fits2D                     (void);

		bool        writeimage                  (Mat img,
                                                 int bitDepth, string nb, bool dtANDstation);

		void        printerror                  (int status);

		bool        loadKeywordsFromConfigFile  (string configFile);

		bool        readFitsToMat(Mat &img, string filePath);

};


