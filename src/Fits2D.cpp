/*
                                Fits2D.cpp

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
* \file    Fits2D.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    01/12/2014
* \brief   Write/Read fits2D file.
*/

#include "Fits2D.h"

boost::log::sources::severity_logger< LogSeverityLevel >  Fits2D::logger;

Fits2D::Init Fits2D::initializer;

Fits2D::~Fits2D(void){}

Fits2D::Fits2D(string path):mFitsPath(path){

    kPROGRAM    = "FreeTure";
    kCREATOR    = "FRIPON";

}

bool Fits2D::writeKeywords(fitsfile *fptr){

    /*

        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% FITS 2D Keywords template %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

        1.  SIMPLE      = T                                         / file does conform to FITS standard
        2.  BITPIX      = 8                                         / number of bits per pixel
        3.  NAXIS       = 2                                         / number of data axes
        4.  NAXIS1      = 1280                                      / length of data axis 1
        5.  NAXIS2      = 960                                       / length of data axis 2
        6.  EXTEND      = T                                         / FITS dataset may contain extensions
        7.  FILENAME    = 'stationOrsay_YYYYMMJJ_HHMMSS_UT.fits'    / name of the fits file
        8.  DATE        = 'YYYY-MM-JJT HH:MM:SS.SS'                 / date of the creation of the fits file
        9.  DATE-OBS    = 'YYYY-MM-JJT HH:MM:SS.SS'                 / acquisition date of the first frame
        10. OBS_MODE    = 'SINGLE'                                  / observation method used to get this fits file 'SINGLE' 'SUM' 'AVERAGE' ('MEDIAN')
        11. ELAPTIME    = 60                                        / end observation date - start observation date (sec.)
        12. EXPOSURE    = 0.033                                     / frame exp (sec). 999999 if different exposure.
        13. ONTIME      = 0.033                                     / total integration time, sum of each frame exp (sec)
        14. FILTER      = "NONE"
        15. TELESCOP    = "<Code station>"                          / station <stationName>
        16. OBSERVER    = "<responsable camera>"
        17. INSTRUME    = 'FRIPON-CAM'
        18. CAMERA      = 'BASLER 1300gm'
        19. FOCAL       = 1.25
        20. APERTURE    = 2.0
        21. SITELONG    = 2.1794397                                 / geodetic WGS84 longitude (DD)
        22. SITELAT     = 48.7063906                                / geodetic WGS84 latitude (DD)
        23. SITEELEV    = 90                                        / geodetic WGS84 elevation (m)
        24. XPIXEL      = 3.75
        25. YPIXEL      = 3.75
        26. GAINDB      = 400                                       / detector gain
        27. SATURATE    = 4095                                      / saturation value
        28. PROGRAM     = 'FreeTure v0.1'                           / name of the acquisition software
        29. CREATOR     = 'FRIPON TEAM'                             / http://fripon.org
        30. BZERO       = 0
        31. BSCALE      = 1
        32. RADESYS     = 'ICRS'
        33. TIMESYS     = 'UTC'
        34. EQUINOX     = 2.000000000000E+03                        / equinox of equatorial coordinates
        35. CTYPE1      = 'RA---ARC'                                / projection and reference system
        36. CTYPE2      = 'DEC--ARC'                                / projection and reference system
        37. TIMEUNIT    = 's '
        38. CD1_1       = 0.0                                       / deg/px
        39. CD1_2       = 0.17                                      / deg/px
        40. CD2_1       = 0.17                                      / deg/pix
        41. CD2_2       = 0.0                                       / deg/pix
        42. CD3_3       = 30                                        / fps
        43. CRPIX1      = 640
        44. CRPIX2      = 480
        45. CRVAL1      =                                           / Sidereal time (decimal degree)
        46. CRVAL2      =                                           / geodetic WGS84 latitude (DD)
        47. K1          =
        48. K2          =

    */

    int status = 0;

    // DELETE DEFAULT COMMENTS.

    if(ffdkey(fptr, "COMMENT", &status))
       printerror( status );

    if(ffdkey(fptr, "COMMENT", &status))
       printerror( status );

    /// 7. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% FILENAME %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * filename = new char[kFILENAME.length()+1];
    strcpy(filename,kFILENAME.c_str());

    char * cfilename = new char[cFILENAME.length()+1];
    strcpy(cfilename,cFILENAME.c_str());

    if(fits_write_key(fptr, TSTRING, "FILENAME", filename, cfilename, &status)){

        delete filename;
        delete cfilename;
        printerror(status, "Error fits_write_key(FILENAME)");
        return false;

    }

    delete cfilename;
    delete filename;

    /// 8. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% DATE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * date = new char[kDATE.length()+1];
    strcpy(date,kDATE.c_str());

    char * cdate = new char[cDATE.length()+1];
    strcpy(cdate,cDATE.c_str());

    if(fits_write_key(fptr,TSTRING,"DATE",date,cdate,&status)){

        delete date;
        delete cdate;
        printerror(status, "Error fits_write_key(DATE)");
        return false;

    }

    delete cdate;
    delete date;

    /// 9. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% DATE-OBS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * dateobs = new char[kDATEOBS.length()+1];
    strcpy(dateobs,kDATEOBS.c_str());

    char * cdateobs = new char[cDATEOBS.length()+1];
    strcpy(cdateobs,cDATEOBS.c_str());

    if(fits_write_key(fptr,TSTRING,"DATE-OBS",dateobs,cdateobs,&status)){

        delete dateobs;
        delete cdateobs;
        printerror(status, "Error fits_write_key(DATE-OBS)");
        return false;

    }

    delete cdateobs;
    delete dateobs;

    /// 10. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% OBS_MODE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cobsmode = new char[cOBSMODE.length()+1];
    strcpy(cobsmode,cOBSMODE.c_str());

    char * obsmode = new char[kOBSMODE.length()+1];
    strcpy(obsmode,kOBSMODE.c_str());

    if(fits_write_key(fptr,TSTRING,"OBS_MODE",obsmode,cobsmode,&status)){

        delete cobsmode;
        delete obsmode;
        printerror(status, "Error fits_write_key(OBS_MODE)");
        return false;

    }

    delete cobsmode;
    delete obsmode;

    /// 11. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% ELAPTIME %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * celaptime = new char[cELAPTIME.length()+1];
    strcpy(celaptime,cELAPTIME.c_str());

    if(fits_write_key(fptr,TDOUBLE,"ELAPTIME",&kELAPTIME,celaptime,&status)){

        delete celaptime;
        printerror(status, "Error fits_write_key(ELAPTIME)");
        return false;

    }

    delete celaptime;

    /// 12. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% EXPOSURE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ceposure = new char[cEXPOSURE.length()+1];
    strcpy(ceposure,cEXPOSURE.c_str());

    if(fits_write_key(fptr,TDOUBLE,"EXPOSURE",&kEXPOSURE,ceposure,&status)){

        delete ceposure;
        printerror(status, "Error fits_write_key(EXPOSURE)");
        return false;

    }

    delete ceposure;

    /// 13. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% ONTIME %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * contime = new char[cONTIME.length()+1];
    strcpy(contime,cONTIME.c_str());

    if(fits_write_key(fptr,TDOUBLE,"ONTIME",&kONTIME,contime,&status)){

        delete contime;
        printerror(status, "Error fits_write_key(ONTIME)");
        return false;

    }

    delete contime;


    /// 14. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% FILTER %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cfilter = new char[cFILTER.length()+1];
    strcpy(cfilter,cFILTER.c_str());

    char * f = new char[kFILTER.length()+1];
    strcpy(f,kFILTER.c_str());

    if(fits_write_key(fptr,TSTRING,"FILTER",f,cfilter,&status)){

        delete cfilter;
        delete f;
        printerror(status, "Error fits_write_key(FILTER)");
        return false;

    }

    delete cfilter;
    delete f;


    /// 15. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% TELESCOP %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ctelescop = new char[cTELESCOP.length()+1];
    strcpy(ctelescop,cTELESCOP.c_str());

    char * t = new char[kTELESCOP.length()+1];
    strcpy(t,kTELESCOP.c_str());

    if(fits_write_key(fptr,TSTRING,"TELESCOP",t,ctelescop,&status)){

        delete ctelescop;
        delete t;
        printerror(status, "Error fits_write_key(TELESCOP)");
        return false;

    }

    delete ctelescop;
    delete t;

    /// 16. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% OBSERVER %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cobserver = new char[cOBSERVER.length()+1];
    strcpy(cobserver,cOBSERVER.c_str());

    char * o = new char[kOBSERVER.length()+1];
    strcpy(o,kOBSERVER.c_str());

    if(fits_write_key(fptr,TSTRING,"OBSERVER",o,cobserver,&status)){

        delete cobserver;
        delete o;
        printerror(status, "Error fits_write_key(OBSERVER)");
        return false;

    }

    delete cobserver;
    delete o;

    /// 17. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% INSTRUME %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cinstrume = new char[cINSTRUME.length()+1];
    strcpy(cinstrume,cINSTRUME.c_str());

    char * i = new char[kINSTRUME.length()+1];
    strcpy(i,kINSTRUME.c_str());

    if(fits_write_key(fptr,TSTRING,"INSTRUME",i,cinstrume,&status)){

        delete cinstrume;
        delete i;
        printerror(status, "Error fits_write_key(OBSERVER)");
        return false;

    }

    delete cinstrume;
    delete i;

    /// 18. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CAMERA %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccamera = new char[cCAMERA.length()+1];
    strcpy(ccamera,cCAMERA.c_str());

    char * cam = new char[kCAMERA.length()+1];
    strcpy(cam,kCAMERA.c_str());

    if(fits_write_key(fptr,TSTRING,"CAMERA",cam,ccamera,&status)){

        delete ccamera;
        delete cam;
        printerror(status, "Error fits_write_key(CAMERA)");
        return false;

    }

    delete ccamera;
    delete cam;

    /// 19. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% FOCAL %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cfocal = new char[cFOCAL.length()+1];
    strcpy(cfocal,cFOCAL.c_str());

    if(fits_write_key(fptr,TDOUBLE,"FOCAL",&kFOCAL,cfocal,&status)){

        delete cfocal;
        printerror(status, "Error fits_write_key(FOCAL)");
        return false;

    }

    delete cfocal;

    /// 20. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% APERTURE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * caperture = new char[cAPERTURE.length()+1];
    strcpy(caperture,cAPERTURE.c_str());

    if(fits_write_key(fptr,TDOUBLE,"APERTURE",&kAPERTURE,caperture,&status)){

        delete caperture;
        printerror(status, "Error fits_write_key(APERTURE)");
        return false;

    }

    delete caperture;

    /// 21. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% SITELONG %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * csitelong = new char[cSITELONG.length()+1];
    strcpy(csitelong,cSITELONG.c_str());

    if(fits_write_key(fptr,TDOUBLE,"SITELONG",&kSITELONG,csitelong,&status)){

        delete csitelong;
        printerror(status, "Error fits_write_key(APERTURE)");
        return false;

    }

    delete csitelong;

    /// 22. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% SITELAT %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * csitelat = new char[cSITELAT.length()+1];
    strcpy(csitelat,cSITELAT.c_str());

    if(fits_write_key(fptr,TDOUBLE,"SITELAT",&kSITELAT,csitelat,&status)){

        delete csitelat;
        printerror(status, "Error fits_write_key(SITELAT)");
        return false;

    }

    delete csitelat;

    /// 23. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% SITEELEV %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * csiteelev = new char[cSITEELEV.length()+1];
    strcpy(csiteelev,cSITEELEV.c_str());

    if(fits_write_key(fptr,TDOUBLE,"SITEELEV",&kSITEELEV,csiteelev,&status)){

        delete csiteelev;
        printerror(status, "Error fits_write_key(SITEELEV)");
        return false;

    }

    delete csiteelev;

    /// 24. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% XPIXEL %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cxpixel = new char[cXPIXEL.length()+1];
    strcpy(cxpixel,cXPIXEL.c_str());

    if(fits_write_key(fptr,TDOUBLE,"XPIXEL",&kXPIXEL,cxpixel,&status)){

        delete cxpixel;
        printerror(status, "Error fits_write_key(XPIXEL)");
        return false;

    }

    delete cxpixel;

    /// 25. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% YPIXEL %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cypixel = new char[cYPIXEL.length()+1];
    strcpy(cypixel,cYPIXEL.c_str());

    if(fits_write_key(fptr,TDOUBLE,"YPIXEL",&kYPIXEL,cypixel,&status)){

        delete cypixel;
        printerror(status, "Error fits_write_key(YPIXEL)");
        return false;

    }

    delete cypixel;

    /// 26. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% GAINDB %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cgaindb = new char[cGAINDB.length()+1];
    strcpy(cgaindb,cGAINDB.c_str());

    if(fits_write_key(fptr,TINT,"GAINDB",&kGAINDB,cgaindb,&status)){

        delete cgaindb;
        printerror(status, "Error fits_write_key(GAINDB)");
        return false;

    }

    delete cgaindb;

    /// 27. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% SATURATE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * csaturate = new char[cSATURATE.length()+1];
    strcpy(csaturate,cSATURATE.c_str());

    if(fits_write_key(fptr,TDOUBLE,"SATURATE",&kSATURATE,csaturate,&status)){

        delete csaturate;
        printerror(status, "Error fits_write_key(SATURATE)");
        return false;

    }

    delete csaturate;

    /// 28. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% PROGRAM %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cprograme = new char[cPROGRAM.length()+1];
    strcpy(cprograme,cPROGRAM.c_str());

    char * p = new char[kPROGRAM.length()+1];
    strcpy(p,kPROGRAM.c_str());

    if(fits_write_key(fptr,TSTRING,"PROGRAM",p,cprograme,&status)){

        delete cprograme;
        delete p;
        printerror(status, "Error fits_write_key(PROGRAM)");
        return false;

    }

    delete cprograme;
    delete p;

    /// 29. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CREATOR %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccreator = new char[cCREATOR.length()+1];
    strcpy(ccreator,cCREATOR.c_str());

    char * c = new char[kCREATOR.length()+1];
    strcpy(c,kCREATOR.c_str());

    if(fits_write_key(fptr,TSTRING,"CREATOR",c,ccreator,&status)){

        delete ccreator;
        delete c;
        printerror(status, "Error fits_write_key(CREATOR)");
        return false;

    }

    delete ccreator;
    delete c;

    /// 30. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% BZERO %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cbzero = new char[cBZERO.length()+1];
    strcpy(cbzero,cBZERO.c_str());

    if(fits_write_key(fptr,TDOUBLE,"BZERO",&kBZERO,cbzero,&status)){

        delete cbzero;
        printerror(status, "Error fits_write_key(BZERO)");
        return false;

    }

    delete cbzero;

    /// 31. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% BSCALE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cbscale = new char[cBSCALE.length()+1];
    strcpy(cbscale,cBSCALE.c_str());

    if(fits_write_key(fptr,TDOUBLE,"BSCALE",&kBSCALE,cbscale,&status)){

        delete cbscale;
        printerror(status, "Error fits_write_key(BSCALE)");
        return false;

    }

    delete cbscale;

    /// 32. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% RADESYS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * radesys = new char[kRADESYS.length()+1];
    strcpy(radesys,kRADESYS.c_str());

    char * cradesys = new char[cRADESYS.length()+1];
    strcpy(cradesys,cRADESYS.c_str());

    if(fits_write_key(fptr,TSTRING,"RADESYS",radesys,cradesys,&status)){

        delete cradesys;
        delete radesys;
        printerror(status, "Error fits_write_key(RADESYS)");
        return false;

    }

    delete cradesys;
    delete radesys;

    /// 33. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% TIMESYS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ctimesys = new char[cTIMESYS.length()+1];
    strcpy(ctimesys,cTIMESYS.c_str());

    char * timesys = new char[kTIMESYS.length()+1];
    strcpy(timesys,kTIMESYS.c_str());

    if(fits_write_key(fptr,TSTRING,"TIMESYS",timesys,ctimesys,&status)){

        delete ctimesys;
        delete timesys;
        printerror(status, "Error fits_write_key(TIMESYS)");
        return false;

    }

    delete ctimesys;
    delete timesys;


    /// 34. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% EQUINOX %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cequinox = new char[cEQUINOX.length()+1];
    strcpy(cequinox,cEQUINOX.c_str());

    if(fits_write_key(fptr,TDOUBLE,"EQUINOX",&kEQUINOX,cequinox,&status)){

        delete cequinox;
        printerror(status, "Error fits_write_key(EQUINOX)");
        return false;

    }

    delete cequinox;

    /// 35. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CTYPE1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ctype1 = new char[cCTYPE1.length()+1];
    strcpy(ctype1,cCTYPE1.c_str());

    char * ktype1 = new char[kCTYPE1.length()+1];
    strcpy(ktype1,kCTYPE1.c_str());

    if(fits_write_key(fptr,TSTRING,"CTYPE1",ktype1,ctype1,&status)){

        delete ctype1;
        delete ktype1;
        printerror(status, "Error fits_write_key(CTYPE1)");
        return false;

    }

    delete ctype1;
    delete ktype1;

    /// 36. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CTYPE2 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ctype2 = new char[cCTYPE2.length()+1];
    strcpy(ctype2,cCTYPE2.c_str());

    char * ktype2 = new char[kCTYPE2.length()+1];
    strcpy(ktype2,kCTYPE2.c_str());

    if(fits_write_key(fptr,TSTRING,"CTYPE2",ktype2,ctype2,&status)){

        delete ctype2;
        delete ktype2;
        printerror(status, "Error fits_write_key(CTYPE2)");
        return false;

    }

    delete ctype2;
    delete ktype2;

    /// 37. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% TIMEUNIT %%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ctimeunit = new char[cTIMEUNIT.length()+1];
    strcpy(ctimeunit,cTIMEUNIT.c_str());

    char * ktimeunit = new char[kTIMEUNIT.length()+1];
    strcpy(ktimeunit,kTIMEUNIT.c_str());

    if(fits_write_key(fptr,TSTRING,"TIMEUNIT",ktimeunit,ctimeunit,&status)){

        delete ctimeunit;
        delete ktimeunit;
        printerror(status, "Error fits_write_key(TIMEUNIT)");
        return false;

    }

    delete ctimeunit;
    delete ktimeunit;

    /// 38. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CD1_1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccd1_1 = new char[cCD1_1.length()+1];
    strcpy(ccd1_1,cCD1_1.c_str());

    if(fits_write_key(fptr,TDOUBLE,"CD1_1",&kCD1_1,ccd1_1,&status)){

        delete ccd1_1;
        printerror(status, "Error fits_write_key(CD1_1)");
        return false;

    }

    delete ccd1_1;

    /// 39. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CD1_2 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccd1_2 = new char[cCD1_2.length()+1];
    strcpy(ccd1_2,cCD1_2.c_str());

    if(fits_write_key(fptr,TDOUBLE,"CD1_2",&kCD1_2,ccd1_2,&status)){

        delete ccd1_2;
        printerror(status, "Error fits_write_key(CD1_2)");
        return false;

    }

    delete ccd1_2;

    /// 40. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CD2_1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccd2_1 = new char[cCD2_1.length()+1];
    strcpy(ccd2_1,cCD2_1.c_str());

    if(fits_write_key(fptr,TDOUBLE,"CD2_1",&kCD2_1,ccd2_1,&status)){

        delete ccd2_1;
        printerror(status, "Error fits_write_key(CD2_1)");
        return false;

    }

    delete ccd2_1;

    /// 41. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CD2_2 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccd2_2 = new char[cCD2_2.length()+1];
    strcpy(ccd2_2,cCD2_2.c_str());

    if(fits_write_key(fptr,TDOUBLE,"CD2_2",&kCD2_2,ccd2_2,&status)){

        delete ccd2_2;
        printerror(status, "Error fits_write_key(CD2_2)");
        return false;

    }

    delete ccd2_2;

    /// 42. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CD3_3 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccd3_3 = new char[cCD3_3.length()+1];
    strcpy(ccd3_3,cCD3_3.c_str());

    if(fits_write_key(fptr,TDOUBLE,"CD3_3",&kCD3_3,ccd3_3,&status)){

        delete ccd3_3;
        printerror(status, "Error fits_write_key(CD3_3)");
        return false;

    }

    delete ccd3_3;

    /// 43. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CRPIX1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccrpix1 = new char[cCRPIX1.length()+1];
    strcpy(ccrpix1,cCRPIX1.c_str());

    if(fits_write_key(fptr,TINT,"CRPIX1",&kCRPIX1,ccrpix1,&status)){

        delete ccrpix1;
        printerror(status, "Error fits_write_key(CRPIX1)");
        return false;

    }

    delete ccrpix1;

    /// 44. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CRPIX2 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccrpix2 = new char[cCRPIX2.length()+1];
    strcpy(ccrpix2,cCRPIX2.c_str());

    if(fits_write_key(fptr,TINT,"CRPIX2",&kCRPIX2,ccrpix2,&status)){

        delete ccrpix2;
        printerror(status, "Error fits_write_key(CRPIX2)");
        return false;

    }

    delete ccrpix2;

    /// 45. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CRVAL1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccrval1 = new char[cCRVAL1.length()+1];
    strcpy(ccrval1,cCRVAL1.c_str());

    if(fits_write_key(fptr,TDOUBLE,"CRVAL1",&kCRVAL1,ccrval1,&status)){

        delete ccrval1;
        printerror(status, "Error fits_write_key(CRVAL1)");
        return false;

    }

    delete ccrval1;

    /// 46. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CRVAL2 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccrval2 = new char[cCRVAL2.length()+1];
    strcpy(ccrval2,cCRVAL2.c_str());

    if(fits_write_key(fptr,TDOUBLE,"CRVAL2",&kSITELAT,ccrval2,&status)){

        delete ccrval2;
        printerror(status, "Error fits_write_key(CRVAL2)");
        return false;

    }

    delete ccrval2;

    /// 47. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% K1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ck1 = new char[cK1.length()+1];
    strcpy(ck1,cK1.c_str());

    if(fits_write_key(fptr,TDOUBLE,"K1",&kK1,ck1,&status)){

        delete ck1;
        printerror(status, "Error fits_write_key(K1)");
        return false;

    }

    delete ck1;

    /// 48. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% K2 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ck2 = new char[cK2.length()+1];
    strcpy(ck2,cK2.c_str());

    if(fits_write_key(fptr,TDOUBLE,"K2",&kK2,ck2,&status)){

        delete ck2;
        printerror(status, "Error fits_write_key(K2)");
        return false;

    }

    delete ck2;

    return true;


}

bool Fits2D::writeFits(Mat img, ImgBitDepth imgType, string fileName, string compression) {

    int status = 0;

    long  firstPixel, nbelements;

    // 2-dimensional image.
    long naxis = 2;

    // Image size.
    long naxes[2] = { img.cols, img.rows };

    // First pixel to write.
    firstPixel = 1;

    // Number of pixels to write.
    nbelements = naxes[0] * naxes[1];

    // Fits creation date : 'YYYY-MM-JJTHH:MM:SS.SS'
    boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
    kDATE = to_iso_extended_string(time);

    // Date in the fits filename.
    string dateFileName = TimeDate::getYYYYMMDDThhmmss(to_iso_string(time));

    // Define CRPIX1 and CRPIX2
    kCRPIX1 = (int)naxes[0] / 2;
    kCRPIX2 = (int)naxes[1] / 2;

    fitsfile *fptr;

    const char * filename;
    const char * filename2;


    // Creation of the fits filename.
    string pathAndname = "";

    if(fileName != ""){

        pathAndname = mFitsPath + fileName  + ".fit";
        kFILENAME = fileName + ".fit";

    }else{

        pathAndname = mFitsPath + kTELESCOP + "_" + dateFileName + "_UT.fit";
        kFILENAME = kTELESCOP + "_" +  dateFileName + "_UT.fit";

    }

    filename = pathAndname.c_str();
    pathAndname += compression;
    filename2 = pathAndname.c_str();

    switch(imgType){

        // UC8
        case 0:
        {
            //https://www-n.oca.eu/pichon/Tableau_2D.pdf
            unsigned char ** tab = (unsigned char * *)malloc( img.rows * sizeof(unsigned char *)) ;

            if(tab == NULL){

                BOOST_LOG_SEV(logger, fail) << "Fail to allocate unsigned char** array (NULL).";
                return false;

            }

            tab[0] = (unsigned char  *) malloc( naxes[0] * naxes[1] * sizeof(unsigned char) ) ;

            if(tab[0] == NULL){

                BOOST_LOG_SEV(logger, fail) << "Fail to allocate unsigned char* array (NULL).";
                return false;

            }

            for( int a = 1; a<naxes[1]; a++ ){

                tab[a] = tab[a-1] + naxes[0];
            }

            // Delete old file if it already exists.
            remove(filename);

            // Create new FITS file.
            if(fits_create_file(&fptr, filename2, &status)){

                 printerror(status);
                 free(tab[0]);
                 return false;

            }

            if (fits_create_img(fptr, BYTE_IMG, naxis, naxes, &status)){

                 printerror(status);
                 free(tab[0]);
                 return false;

            }

            // Initialize the values in the fits image with the mat's values.
             for ( int j = 0; j < naxes[1]; j++){

                 unsigned char * matPtr = img.ptr<unsigned char>(j);

                 for ( int i = 0; i < naxes[0]; i++){

                     // Affect a value and inverse the image.
                     tab[img.rows-1-j][i] = (unsigned char)matPtr[i];

                }
            }

            // Write the array of unsigned short to the FITS file.
            if(fits_write_img(fptr, TBYTE, firstPixel, nbelements, tab[0], &status)){

                 printerror(status);
                 free(tab[0]);
                 return false;

            }

            // Free previously allocated memory.
            free(tab[0]);

            break;
        }

        case 1:
        {
            //https://www-n.oca.eu/pichon/Tableau_2D.pdf
            char ** tab = (char * *) malloc( img.rows * sizeof( char * ) ) ;

            if(tab == NULL){

                BOOST_LOG_SEV(logger, fail) << "Fail to allocate char** array (NULL).";
                return false;

            }

            tab[0] = (char *) malloc( naxes[0] * naxes[1] * sizeof(char) ) ;

            if(tab[0] == NULL){

                BOOST_LOG_SEV(logger, fail) << "Fail to allocate char* array (NULL).";
                return false;

            }

            for( int a = 1; a<naxes[1]; a++ ){

                tab[a] = tab[a-1] + naxes[0];
            }

            // Delete old file if it already exists.
            remove(filename);

            // Create new FITS file.
            if(fits_create_file(&fptr, filename2, &status)){

                 printerror(status);
                 free(tab[0]);
                 return false;

            }


            if(fits_create_img(fptr,  SBYTE_IMG, naxis, naxes, &status)){

                 printerror(status);
                 free(tab[0]);
                 return false;

            }

            // Initialize the values in the fits image with the mat's values.
             for( int j = 0; j < naxes[1]; j++){

                 char * matPtr = img.ptr<char>(j);

                 for(int i = 0; i < naxes[0]; i++){

                     // Affect a value and inverse the image.
                     tab[img.rows-1-j][i] = (char)matPtr[i];

                }
            }

            // Write the array of unsigned short to the FITS file.
             if(fits_write_img(fptr, TSBYTE, firstPixel, nbelements, tab[0], &status)){

                 printerror(status);
                 free(tab[0]);
                 return false;

            }

            // Free previously allocated memory.
            free(tab[0]);

            break;
        }

        case 2 :
        {

            //https://www-n.oca.eu/pichon/Tableau_2D.pdf
            unsigned short ** tab = (unsigned short * *) malloc( img.rows * sizeof( unsigned short * ) ) ;

            if(tab == NULL){

                BOOST_LOG_SEV(logger, fail) << "Fail to allocate unsigned short** array (NULL).";
                return false;

            }

            tab[0] = (unsigned short  *) malloc( naxes[0] * naxes[1] * sizeof(unsigned short) ) ;

            if(tab[0] == NULL){

                BOOST_LOG_SEV(logger, fail) << "Fail to allocate unsigned short* array (NULL).";
                return false;

            }

            for( int a = 1; a<naxes[1]; a++ ){

              tab[a] = tab[a-1] + naxes[0];
            }

            // Delete old file if it already exists.
            remove(filename);

            // Create new FITS file.
            if (fits_create_file(&fptr, filename2, &status)){

                 printerror(status);
                 free( *tab);
                 free( tab );
                 return false;

            }

            if ( fits_create_img(fptr,  USHORT_IMG, naxis, naxes, &status)){

                 printerror(status);
                 free( *tab);
                 free( tab );
                 return false;
            }

            // Initialize the values in the fits image with the mat's values.
            for ( int j = 0; j < naxes[1]; j++){

                 unsigned short * matPtr = img.ptr<unsigned short>(j);

                 for ( int i = 0; i < naxes[0]; i++){

                     // Affect a value and inverse the image.
                     tab[img.rows-1-j][i] = (unsigned short)matPtr[i];
                }
            }

            // write the array of unsigned short to the FITS file
            if ( fits_write_img(fptr, TUSHORT, firstPixel, nbelements, tab[0], &status)){

                 printerror(status);
                 free( *tab);
                 free( tab );
                 return false;
            }

            // Free previously allocated memory.
            free( *tab);
            free( tab );

            break;

        }

        case 3 :
        {

            Mat newMat;

            if(img.type() == CV_16UC1) {

                // Convert unsigned short type image in short type image.
                newMat = Mat(img.rows, img.cols, CV_16SC1, Scalar(0));

                // Set bzero and bscale for print unsigned short value in soft visualization.
                kBZERO = 32768;
                kBSCALE = 1;

                unsigned short * ptr;
                short * ptr2;

                for(int i = 0; i < img.rows; i++){

                    ptr = img.ptr<unsigned short>(i);
                    ptr2 = newMat.ptr<short>(i);

                    for(int j = 0; j < img.cols; j++){

                        if(ptr[j] - 32768 > 32767){

                            ptr2[j] = 32767;

                        }else{

                            ptr2[j] = ptr[j] - 32768;
                        }
                    }
                }

            }else{

                img.copyTo(newMat);

            }

            //https://www-n.oca.eu/pichon/Tableau_2D.pdf
            short ** tab = (short * *) malloc( img.rows * sizeof( short * ) ) ;

            if(tab == NULL){

                BOOST_LOG_SEV(logger, fail) << "Fail to allocate short** array (NULL).";
                return false;

            }

            tab[0] = (short  *) malloc( naxes[0] * naxes[1] * sizeof(short) ) ;

            if(tab[0] == NULL){

                BOOST_LOG_SEV(logger, fail) << "Fail to allocate short* array (NULL).";
                return false;

            }

            for( int a = 1; a<naxes[1]; a++ ){

              tab[a] = tab[a-1] + naxes[0];
            }

            // Delete old file if it already exists.
            remove(filename);

            // Create new FITS file.
            if (fits_create_file(&fptr, filename2, &status)){

                 printerror(status);
                 free( *tab);
                 free( tab );
                 return false;
            }


            if ( fits_create_img(fptr,  SHORT_IMG, naxis, naxes, &status)){

                 printerror(status);
                 free( *tab);
                 free( tab );
                 return false;
            }

            // Initialize the values in the fits image with the mat's values.
            for ( int j = 0; j < naxes[1]; j++){

                 short * matPtr = newMat.ptr<short>(j);

                 for ( int i = 0; i < naxes[0]; i++){

                     // Affect a value and inverse the image.
                     tab[newMat.rows-1-j][i] = (short)matPtr[i];
                }
            }

            // write the array of signed short to the FITS file
            if ( fits_write_img(fptr, TSHORT, firstPixel, nbelements, tab[0], &status)){

                 printerror(status);
                 free( *tab);
                 free( tab );
                 return false;
            }

            // Free previously allocated memory.
            free( *tab);
            free( tab );

            break;

        }

        case 4 :
        {

            //https://www-n.oca.eu/pichon/Tableau_2D.pdf
            float ** tab = (float * *) malloc( img.rows * sizeof( float * ) ) ;

            if(tab == NULL){

                BOOST_LOG_SEV(logger, fail) << "Fail to allocate float** array (NULL).";
                return false;
            }

            tab[0] = (float *) malloc( naxes[0] * naxes[1] * sizeof(float) ) ;

            if(tab[0] == NULL){

                BOOST_LOG_SEV(logger, fail) << "Fail to allocate float* array (NULL).";
                return false;
            }

            for( int a = 1; a<naxes[1]; a++ ){

                tab[a] = tab[a-1] + naxes[0];
            }

            // Delete old file if it already exists.
            remove(filename);

            // Create new FITS file
            if(fits_create_file(&fptr, filename, &status)){

                 printerror(status);
                 free( *tab);
                 free( tab );
                 return false;
            }

            if( fits_create_img(fptr,  FLOAT_IMG, naxis, naxes, &status)){

                 printerror(status);
                 free( *tab);
                 free( tab );
                 return false;
            }

            // Initialize the values in the fits image with the mat's values.
            for( int j = 0; j < naxes[1]; j++){

                 float * matPtr = img.ptr<float>(j);

                 for ( int i = 0; i < naxes[0]; i++){

                     // Affect a value and inversed the image.
                     tab[img.rows-1-j][i] = (float)matPtr[i];
                 }
            }

            // Write the array of unsigned short to the FITS file.
            if( fits_write_img(fptr, TFLOAT, firstPixel, nbelements, tab[0], &status)){

                 printerror(status);
                 free( *tab);
                 free( tab );
                 return false;
            }

            // Free previously allocated memory.
            free(*tab);
            free(tab);


            break;
        }

    }

    if(!writeKeywords(fptr)){

        if(fits_close_file(fptr, &status)){

             printerror(status);
        }

        return false;
    }

    if(fits_close_file(fptr, &status)){

        printerror(status);
        return false;
    }

    return true;
}

//Float 32bits float -1.18*10-38~3.40*10-38
bool Fits2D::readFits32F(Mat &img){

    float * ptr = NULL;
    float  * ptr1 = NULL;

    int status = 0,  nfound, anynull;
    long naxes[2], fpixel, nbuffer, npixels;

    float  nullval;


    fitsfile *fptr;

    const char * filename;

    filename = mFitsPath.c_str();

    if(fits_open_file(&fptr, filename, READONLY, &status)){

        printerror(status);
        return false;

    }

    // Read the NAXIS1 and NAXIS2 keyword to get image size.
    if(fits_read_keys_lng(fptr, "NAXIS", 1, 2, naxes, &nfound, &status)){

        printerror(status);
        return false;

    }

    Mat image = Mat::zeros( naxes[1],naxes[0], CV_32FC1);

    npixels  = naxes[0] * naxes[1];         // number of pixels in the image
    fpixel   = 1;                           // first pixel
    nullval  = 0;                           // don't check for null values in the image

    nbuffer = npixels;

    //float  buffer[npixels];
    float* buffer = new float[npixels];

    if(fits_read_img(fptr, TFLOAT, fpixel, nbuffer, &nullval,buffer, &anynull, &status)){

        printerror(status);
        delete buffer;
        return false;

    }

    memcpy(image.ptr(), buffer, npixels * 4);

    Mat loadImg = Mat::zeros( naxes[1],naxes[0], CV_32FC1 );

    // y
    for(int i = 0; i < naxes[1]; i++){

        float * ptr = image.ptr<float>(i);
        float * ptr1 = loadImg.ptr<float >(naxes[1] - 1 - i);

        // x
        for(int j = 0; j < naxes[0]; j++){

            ptr1[j] = ptr[j];

        }
    }

    loadImg.copyTo(img);
    delete buffer;

    if(fits_close_file(fptr, &status)){

        printerror(status);
        return false;

    }

    return true;

}

//Unsigned 16bits ushort 0~65535
bool Fits2D::readFits16US(Mat &img){

    unsigned short * ptr = NULL;
    unsigned short  * ptr1 = NULL;

    int status = 0,  nfound, anynull;
    long naxes[2], fpixel, nbuffer, npixels;

    float  nullval;


    fitsfile *fptr;

    const char * filename;

    filename = mFitsPath.c_str();

    if(fits_open_file(&fptr, filename, READONLY, &status)){

        printerror(status);
        return false;

    }

    // Read the NAXIS1 and NAXIS2 keyword to get image size.
    if(fits_read_keys_lng(fptr, "NAXIS", 1, 2, naxes, &nfound, &status)){

        printerror(status);
        return false;

    }

    Mat image = Mat::zeros( naxes[1],naxes[0], CV_16UC1);

    npixels  = naxes[0] * naxes[1];         // number of pixels in the image
    fpixel   = 1;                           // first pixel
    nullval  = 0;                           // don't check for null values in the image

    nbuffer = npixels;

    //unsigned short  buffer[npixels];
	unsigned short* buffer = new unsigned short[npixels];
    if(fits_read_img(fptr, TUSHORT, fpixel, nbuffer, &nullval,buffer, &anynull, &status)){

        printerror(status);
        delete buffer;
        return false;

    }

    memcpy(image.ptr(), buffer, npixels * 2);

    Mat loadImg = Mat::zeros( naxes[1],naxes[0], CV_16UC1 );

    // y
    for(int i = 0; i < naxes[1]; i++){

        unsigned short * ptr = image.ptr<unsigned short>(i);
        unsigned short * ptr1 = loadImg.ptr<unsigned short >(naxes[1] - 1 - i);

        // x
        for(int j = 0; j < naxes[0]; j++){

            ptr1[j] = ptr[j];

        }
    }

    loadImg.copyTo(img);
	delete buffer;

    if(fits_close_file(fptr, &status)){

        printerror(status);
        return false;

    }

    return true;

}

//Signed 16bits short -32768~32767
bool Fits2D::readFits16S(Mat &img){

    short * ptr = NULL;
    unsigned short  * ptr1 = NULL;

    int status = 0,  nfound, anynull;
    long naxes[2], fpixel, nbuffer, npixels;

    float  nullval;

     fitsfile *fptr;

    const char * filename;

    filename = mFitsPath.c_str();

    if(fits_open_file(&fptr, filename, READONLY, &status)){

        printerror(status);
        return false;
    }

    // Read the NAXIS1 and NAXIS2 keyword to get image size.
    if(fits_read_keys_lng(fptr, "NAXIS", 1, 2, naxes, &nfound, &status)){

        printerror(status);
        return false;
    }

    Mat image = Mat::zeros( naxes[1],naxes[0], CV_16SC1);

    npixels  = naxes[0] * naxes[1];         // number of pixels in the image
    fpixel   = 1;                           // first pixel
    nullval  = 0;                           // don't check for null values in the image

    nbuffer = npixels;

   // short  buffer[npixels];
    short* buffer = new short[npixels];
    if(fits_read_img(fptr, TSHORT, fpixel, nbuffer, &nullval,buffer, &anynull, &status)){

        printerror(status);
        delete buffer;
        return false;
    }

    memcpy(image.ptr(), buffer, npixels * 2);

    Mat loadImg = Mat::zeros( naxes[1],naxes[0], CV_16UC1 );

    // y
    for(int i = 0; i < naxes[1]; i++){

        short * ptr = image.ptr<short>(i);
        unsigned short * ptr1 = loadImg.ptr<unsigned short >(naxes[1] - 1 - i);

        // x
        for(int j = 0; j < naxes[0]; j++){

            ptr1[j] = ptr[j] + 32768;

        }
    }

    loadImg.copyTo(img);
    delete buffer;
    if(fits_close_file(fptr, &status)){

        printerror(status);
        return false;
    }


    return true;

}

//Unsigned 8bits uchar 0~255
bool Fits2D::readFits8UC(Mat &img){

    unsigned char * ptr = NULL;
    unsigned char  * ptr1 = NULL;

    int status = 0,  nfound, anynull;
    long naxes[2], fpixel, nbuffer, npixels;

    float  nullval;

    fitsfile *fptr;

    const char * filename;

    filename = mFitsPath.c_str();

    if(fits_open_file(&fptr, filename, READONLY, &status)){

        printerror(status);
        return false;
    }

    // Read the NAXIS1 and NAXIS2 keyword to get image size.
    if(fits_read_keys_lng(fptr, "NAXIS", 1, 2, naxes, &nfound, &status)){

        printerror(status);
        return false;
    }

    Mat image = Mat::zeros( naxes[1],naxes[0], CV_8UC1);

    npixels  = naxes[0] * naxes[1];         // number of pixels in the image
    fpixel   = 1;                           // first pixel
    nullval  = 0;                           // don't check for null values in the image

    nbuffer = npixels;

    //unsigned char  buffer[npixels];
    unsigned char* buffer = new unsigned char[npixels];

    if(fits_read_img(fptr, TBYTE, fpixel, nbuffer, &nullval,buffer, &anynull, &status)){

        printerror(status);
        delete buffer;
        return false;
    }

    memcpy(image.ptr(), buffer, npixels * 2);

    Mat loadImg = Mat::zeros( naxes[1],naxes[0], CV_8UC1 );

    // y
    for(int i = 0; i < naxes[1]; i++){

        unsigned char * ptr = image.ptr<unsigned char>(i);
        unsigned char * ptr1 = loadImg.ptr<unsigned char >(naxes[1] - 1 - i);

        // x
        for(int j = 0; j < naxes[0]; j++){

            ptr1[j] = ptr[j];

        }
    }

    loadImg.copyTo(img);

    delete buffer;

    if(fits_close_file(fptr, &status)){

        printerror(status);
        return false;
    }

    return true;

}

//Signed 8bits char -128~127
bool Fits2D::readFits8C(Mat &img){

    char * ptr = NULL;
    char  * ptr1 = NULL;

    int status = 0,  nfound, anynull;
    long naxes[2], fpixel, nbuffer, npixels;

    float  nullval;

    fitsfile *fptr;

    const char * filename;

    filename = mFitsPath.c_str();

    if(fits_open_file(&fptr, filename, READONLY, &status)){

        printerror(status);
        return false;
    }

    // Read the NAXIS1 and NAXIS2 keyword to get image size.
    if(fits_read_keys_lng(fptr, "NAXIS", 1, 2, naxes, &nfound, &status)){

        printerror(status);
        return false;
    }

    Mat image = Mat::zeros( naxes[1],naxes[0], CV_8SC1);

    npixels  = naxes[0] * naxes[1];         // number of pixels in the image
    fpixel   = 1;                           // first pixel
    nullval  = 0;                           // don't check for null values in the image

    nbuffer = npixels;

//    char  buffer[npixels];
    char* buffer = new char[npixels];

    if(fits_read_img(fptr, TSBYTE, fpixel, nbuffer, &nullval,buffer, &anynull, &status)){

        printerror(status);
        delete buffer;
        return false;
    }

    memcpy(image.ptr(), buffer, npixels * 2);

    Mat loadImg = Mat::zeros( naxes[1],naxes[0], CV_8SC1 );

    // y
    for(int i = 0; i < naxes[1]; i++){

        char * ptr = image.ptr<char>(i);
        char * ptr1 = loadImg.ptr<char >(naxes[1] - 1 - i);

        // x
        for(int j = 0; j < naxes[0]; j++){

            ptr1[j] = ptr[j];

        }
    }

    loadImg.copyTo(img);
    delete buffer;

    if(fits_close_file(fptr, &status)){

        printerror(status);
        return false;
    }

    return true;

}

bool Fits2D::readIntKeyword(string keyword, int &value){

    char *ptr = NULL;

    int status = 0;

    fitsfile *fptr;

    const char * filename;

    filename = mFitsPath.c_str();

    if(fits_open_file(&fptr, filename, READONLY, &status)){

        printerror(status);
        return false;
    }

    char * key = new char[keyword.length()+1];
    strcpy(key,keyword.c_str());

    if(fits_read_key(fptr, TINT, key, &value, NULL, &status)){

        printerror(status);
        delete key;
        return false;
    }

    delete key;

    if(fits_close_file(fptr, &status)){

        printerror(status);
        return false;
    }

    return true;
}

bool Fits2D::readStringKeyword(string keyword, string &value){

    char *ptr = NULL;

    int status = 0;

    fitsfile *fptr;

    const char * filename;

    char v[40];

    filename = mFitsPath.c_str();


    if(fits_open_file(&fptr, filename, READONLY, &status)){

        printerror(status);
        return false;

    }

    char * key = new char[keyword.length()+1];
    strcpy(key,keyword.c_str());

    if(fits_read_key(fptr, TSTRING, key, v, NULL, &status)){


        printerror(status);
        delete key;
        return false;

    }

    value = string(v);

    delete key;

    if(fits_close_file(fptr, &status)){

        printerror(status);
        return false;

    }

    return true;
}

bool Fits2D::readDoubleKeyword(string keyword, double &value){

    char *ptr = NULL;

    int status = 0;

    fitsfile *fptr;

    const char * filename;

    filename = mFitsPath.c_str();

    if(fits_open_file(&fptr, filename, READONLY, &status)){

        printerror(status);
        return false;
    }

    char * key = new char[keyword.length()+1];
    strcpy(key,keyword.c_str());

    if(fits_read_key(fptr, TDOUBLE, key, &value, NULL, &status)){

        printerror(status);
        delete key;
        return false;
    }

    delete key;

    if(fits_close_file(fptr, &status)){

        printerror(status);
        return false;

    }

    return true;
}

void Fits2D::printerror(int status, string errorMsg){

    if(status){

        char status_str[200];
        fits_get_errstatus(status, status_str);

        BOOST_LOG_SEV(logger, fail) << errorMsg;
        cout << errorMsg << endl;
        std::string str(status_str);
        BOOST_LOG_SEV(logger, fail) << "CFITSIO ERROR : " << status << " -> " << str;
        cout << "CFITSIO ERROR : " << status << " -> " << str << endl;

    }

}

void Fits2D::printerror(int status){

    if(status){

        char status_str[200];
        fits_get_errstatus(status, status_str);
        std::string str(status_str);
        BOOST_LOG_SEV(logger, fail) << "CFITSIO ERROR : " << status << " -> " << str;
        cout << "CFITSIO ERROR : " << status << " -> " << str << endl;

    }
}
