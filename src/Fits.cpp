/*
                                Fits.cpp

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
*   Last modified:      28/11/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    Fits.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    04/12/2014
* \brief   Parent class of fits file with the keywords and their comments.
*/

#include "Fits.h"

Fits::Fits() {

    STATION     = "station";

    kFILENAME   = "";
    kDATE       = "";
    kDATEOBS    = "";
    kOBSMODE    = "SINGLE";
    kELAPTIME   = 0;
    kEXPOSURE   = 0.0;
    kONTIME     = 0.0;
    kFILTER     = "";
    kTELESCOP   = "";
    kOBSERVER   = "";
    kINSTRUME   = "";
    kCAMERA     = "";
    kFOCAL      = 0.0;
    kAPERTURE   = 0.0;
    kSITELONG   = 0.0;
    kSITELAT    = 0.0;
    kSITEELEV   = 0.0;
    kXPIXEL     = 0.0;
    kYPIXEL     = 0.0;
    kGAINDB     = 0;
    kSATURATE   = 0.0;
    kPROGRAM    = "";
    kCREATOR    = "" ;
    kBZERO      = 0.0;
    kBSCALE     = 1.0;
    kRADESYS    = "ICRS";
    kTIMESYS    = "UTC";
    kEQUINOX    = 0.0;
    kCTYPE1     = "";
    kCTYPE2     = "";
    kCTYPE3     = "UTC";
    kTIMEUNIT   = "s";
    kCD1_1      = 0.0;
    kCD1_2      = 0.0;
    kCD2_1      = 0.0;
    kCD2_2      = 0.0;
    kCD3_3      = 0.0;
    kCD1_3      = 0.0;
    kCD2_3      = 0.0;
    kCD3_1      = 0.0;
    kCD3_2      = 0.0;
    kCRPIX1     = 0;
    kCRPIX2     = 0;
    kCRPIX3     = 0;
    kCRVAL1     = 0.0;
    kCRVAL2     = 0.0;
    kK1         = 0.0;
    kK2         = 0.0;
    kCOMMENT    = "";

    cFILENAME   = "name of the fits file";
    cDATE       = "date of the creation of the fits file";
    cDATEOBS    = "acquisition date of the first frame";
    cOBSMODE    = "'SINGLE' 'SUM' 'AVERAGE' ('MEDIAN')";
    cELAPTIME   = "end obs. date - start obs. date (sec.)";
    cEXPOSURE   = "frame exp (sec). 999999 if different exposure.";
    cONTIME     = "total integration time, sum of each frame exp (sec)";
    cFILTER     = "";
    cTELESCOP   = "station " + STATION;
    cOBSERVER   = "";
    cINSTRUME   = "";
    cCAMERA     = "";
    cFOCAL      = "";
    cAPERTURE   = "";
    cSITELONG   = "geodetic WGS84 longitude (DD)";
    cSITELAT    = "geodetic WGS84 latitude (DD)";
    cSITEELEV   = "geodetic WGS84 elevation (m)";
    cXPIXEL     = "";
    cYPIXEL     = "";
    cGAINDB     = "detector gain";
    cSATURATE   = "saturated value";
    cPROGRAM    = "name of the acquisition software";
    cCREATOR    = "http://fripon.org";
    cBZERO      = "";
    cBSCALE     = "";
    cRADESYS    = "";
    cTIMESYS    = "";
    cEQUINOX    = "equinox of equatorial coordinates";
    cCTYPE1     = "projection and reference system";
    cCTYPE2     = "projection and reference system";
    cCTYPE3     = "time reference system ";
    cTIMEUNIT   = "";
    cCD1_1      = "deg/px";
    cCD1_2      = "deg/px";
    cCD2_1      = "deg/px";
    cCD2_2      = "deg/px";
    cCD3_3      = "fps";
    cCD1_3      = "";
    cCD2_3      = "";
    cCD3_1      = "";
    cCD3_2      = "";
    cCRPIX1     = "";
    cCRPIX2     = "";
    cCRPIX3     = "";
    cCRVAL1     = "sidereal time (decimal degree)";
    cCRVAL2     = "geodetic WGS84 latitude (DD)";
    cK1         = "R = K1 * f * sin(theta/K2)";
    cK2         = "R = K1 * f * sin(theta/K2)";
    cCOMMENT    = "";

}

Fits::~Fits(){
    //dtor
}

void Fits::loadKeys(fitskeysParam fkp, stationParam sp) {

    STATION = sp.STATION_NAME;
    kTELESCOP = sp.TELESCOP;
    kOBSERVER = sp.OBSERVER;
    kINSTRUME = sp.INSTRUME;
    kCAMERA = sp.CAMERA;
    kFOCAL = sp.FOCAL;
    kAPERTURE = sp.APERTURE;
    kSITELONG = sp.SITELONG;
    kSITELAT = sp.SITELAT;
    kSITEELEV = sp.SITEELEV;
    kK1 = fkp.K1;
    kK2 = fkp.K2;
    kFILTER = fkp.FILTER;
    kCD1_1 = fkp.CD1_1;
    kCD1_2 = fkp.CD1_2;
    kCD2_1 = fkp.CD2_1;
    kCD2_2 = fkp.CD2_2;
    kXPIXEL = fkp.XPIXEL;
    kYPIXEL = fkp.YPIXEL;
    kCOMMENT = fkp.COMMENT;

}
