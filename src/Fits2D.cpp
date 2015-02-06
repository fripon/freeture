/*
								Fits2D.cpp

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
*	Last modified:		01/12/2014
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

src::severity_logger< LogSeverityLevel >  Fits2D::logger;
Fits2D::_Init Fits2D::_initializer;

Fits2D::Fits2D(){}

Fits2D::~Fits2D(void){}

Fits2D::Fits2D(string recPath, Fits fits){

    fitsPath = recPath;

    kFILTER     = fits.getFilter();
    kTELESCOP   = fits.getTelescop();
    kOBSERVER   = fits.getObserver();
    kINSTRUME   = fits.getInstrument();
    kCAMERA     = fits.getCamera();
    kFOCAL      = fits.getFocal();
    kAPERTURE   = fits.getAperture();
    kSITELONG   = fits.getSitelong();
    kSITELAT    = fits.getSitelat();
    kSITEELEV   = fits.getSiteelev();
    kK1         = fits.getK1();
    kK2         = fits.getK2();
    kCOMMENT    = fits.getComment();
    kPROGRAM    = fits.getProgram();
    kCREATOR    = fits.getCreator();
    kCD1_1      = fits.getCd1_1();
    kCD1_2      = fits.getCd1_2();
    kCD2_1      = fits.getCd2_1();
    kCD2_2      = fits.getCd2_2();
    kCRPIX1     = fits.getCrpix1();
    kCRPIX2     = fits.getCrpix2();
    kXPIXEL     = fits.getXpixel();
    kYPIXEL     = fits.getYpixel();

}

Fits2D::Fits2D(string recPath){

    fitsPath = recPath;

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
        12. EXPOSURE    = 0.033                                     / integration time : 1/fps * nb_frames (sec.)
        13. ONTIME      = 0.033                                     / frame exposure time (sec.)
        14. FILTER      = "NONE"
        15. TELESCOP    = "<Code station>"                          / station <stationName>
        16. OBSERVER    = "<responsable camera>"
        17. INSTRUME    = 'FRIPON-CAM'
        18. CAMERA      = 'BASLER 1300gm'
        19. FOCAL       = 1.25
        20. APERTURE    = 2.0
        21. SITELONG    = 2.1794397                                 / longitude observatory
        22. SITELAT     = 48.7063906                                / latitude observatory
        23. SITEELEV    = 90                                        / observatory elevation (meter)
        24. XPIXEL      = 3.75
        25. YPIXEL      = 3.75
        26. GAINDB      = 400                                       / detector gain
        27. SATURATE    = 4095                                      / saturation value or max value (not saturated) in case where OBS_MODE = SUM
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
        46. CRVAL2      =                                           / latitude observatory (decimal degree)
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
        return printerror(status, "Error fits_write_key(FILENAME)");

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
        return printerror(status, "Error fits_write_key(DATE)");

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
        return printerror(status, "Error fits_write_key(DATE-OBS)");

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
        return printerror(status, "Error fits_write_key(OBS_MODE)");

    }

    delete cobsmode;
    delete obsmode;

    /// 11. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% ELAPTIME %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * celaptime = new char[cELAPTIME.length()+1];
    strcpy(celaptime,cELAPTIME.c_str());

    if(fits_write_key(fptr,TDOUBLE,"ELAPTIME",&kELAPTIME,celaptime,&status)){

        delete celaptime;
        return printerror(status, "Error fits_write_key(ELAPTIME)");

    }

    delete celaptime;

    /// 12. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% EXPOSURE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ceposure = new char[cEXPOSURE.length()+1];
    strcpy(ceposure,cEXPOSURE.c_str());

    if(fits_write_key(fptr,TDOUBLE,"EXPOSURE",&kEXPOSURE,ceposure,&status)){

        delete ceposure;
        return printerror(status, "Error fits_write_key(EXPOSURE)");

    }

    delete ceposure;

    /// 13. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% ONTIME %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * contime = new char[cONTIME.length()+1];
    strcpy(contime,cONTIME.c_str());

    if(fits_write_key(fptr,TDOUBLE,"ONTIME",&kONTIME,contime,&status)){

        delete contime;
        return printerror(status, "Error fits_write_key(ONTIME)");

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
        return printerror(status, "Error fits_write_key(FILTER)");

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
        return printerror(status, "Error fits_write_key(TELESCOP)");

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
        return printerror(status, "Error fits_write_key(OBSERVER)");

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
        return printerror(status, "Error fits_write_key(OBSERVER)");

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
        return printerror(status, "Error fits_write_key(CAMERA)");

    }

    delete ccamera;
    delete cam;

    /// 19. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% FOCAL %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cfocal = new char[cFOCAL.length()+1];
    strcpy(cfocal,cFOCAL.c_str());

    if(fits_write_key(fptr,TDOUBLE,"FOCAL",&kFOCAL,cfocal,&status)){

        delete cfocal;
        return printerror(status, "Error fits_write_key(FOCAL)");

    }

    delete cfocal;

    /// 20. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% APERTURE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * caperture = new char[cAPERTURE.length()+1];
    strcpy(caperture,cAPERTURE.c_str());

    if(fits_write_key(fptr,TDOUBLE,"APERTURE",&kAPERTURE,caperture,&status)){

        delete caperture;
        return printerror(status, "Error fits_write_key(APERTURE)");

    }

    delete caperture;

    /// 21. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% SITELONG %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * csitelong = new char[cSITELONG.length()+1];
    strcpy(csitelong,cSITELONG.c_str());

    if(fits_write_key(fptr,TDOUBLE,"SITELONG",&kSITELONG,csitelong,&status)){

        delete csitelong;
        return printerror(status, "Error fits_write_key(APERTURE)");

    }

    delete csitelong;

    /// 22. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% SITELAT %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * csitelat = new char[cSITELAT.length()+1];
    strcpy(csitelat,cSITELAT.c_str());

    if(fits_write_key(fptr,TDOUBLE,"SITELAT",&kSITELAT,csitelat,&status)){

        delete csitelat;
        return printerror(status, "Error fits_write_key(SITELAT)");

    }

    delete csitelat;

    /// 23. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% SITEELEV %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * csiteelev = new char[cSITEELEV.length()+1];
    strcpy(csiteelev,cSITEELEV.c_str());

    if(fits_write_key(fptr,TDOUBLE,"SITEELEV",&kSITEELEV,csiteelev,&status)){

        delete csiteelev;
        return printerror(status, "Error fits_write_key(SITEELEV)");

    }

    delete csiteelev;

    /// 24. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% XPIXEL %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cxpixel = new char[cXPIXEL.length()+1];
    strcpy(cxpixel,cXPIXEL.c_str());

    if(fits_write_key(fptr,TDOUBLE,"XPIXEL",&kXPIXEL,cxpixel,&status)){

        delete cxpixel;
        return printerror(status, "Error fits_write_key(XPIXEL)");

    }

    delete cxpixel;

    /// 25. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% YPIXEL %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cypixel = new char[cYPIXEL.length()+1];
    strcpy(cypixel,cYPIXEL.c_str());

    if(fits_write_key(fptr,TDOUBLE,"YPIXEL",&kYPIXEL,cypixel,&status)){

        delete cypixel;
        return printerror(status, "Error fits_write_key(YPIXEL)");

    }

    delete cypixel;

    /// 26. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% GAINDB %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cgaindb = new char[cGAINDB.length()+1];
    strcpy(cgaindb,cGAINDB.c_str());

    if(fits_write_key(fptr,TINT,"GAINDB",&kGAINDB,cgaindb,&status)){

        delete cgaindb;
        return printerror(status, "Error fits_write_key(GAINDB)");

    }

    delete cgaindb;

    /// 27. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% SATURATE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * csaturate = new char[cSATURATE.length()+1];
    strcpy(csaturate,cSATURATE.c_str());

    if(fits_write_key(fptr,TDOUBLE,"SATURATE",&kSATURATE,csaturate,&status)){

        delete csaturate;
        return printerror(status, "Error fits_write_key(SATURATE)");

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
        return printerror(status, "Error fits_write_key(PROGRAM)");

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
        return printerror(status, "Error fits_write_key(CREATOR)");

    }

    delete ccreator;
    delete c;

    /// 30. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% BZERO %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cbzero = new char[cBZERO.length()+1];
    strcpy(cbzero,cBZERO.c_str());

    if(fits_write_key(fptr,TDOUBLE,"BZERO",&kBZERO,cbzero,&status)){

        delete cbzero;
        return printerror(status, "Error fits_write_key(BZERO)");

    }

    delete cbzero;

    /// 31. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% BSCALE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cbscale = new char[cBSCALE.length()+1];
    strcpy(cbscale,cBSCALE.c_str());

    if(fits_write_key(fptr,TDOUBLE,"BSCALE",&kBSCALE,cbscale,&status)){

        delete cbscale;
        return printerror(status, "Error fits_write_key(BSCALE)");

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
        return printerror(status, "Error fits_write_key(RADESYS)");

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
        return printerror(status, "Error fits_write_key(TIMESYS)");

    }

    delete ctimesys;
    delete timesys;


    /// 34. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% EQUINOX %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * cequinox = new char[cEQUINOX.length()+1];
    strcpy(cequinox,cEQUINOX.c_str());

    if(fits_write_key(fptr,TDOUBLE,"EQUINOX",&kEQUINOX,cequinox,&status)){

        delete cequinox;
        return printerror(status, "Error fits_write_key(EQUINOX)");

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
        return printerror(status, "Error fits_write_key(CTYPE1)");

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
        return printerror(status, "Error fits_write_key(CTYPE2)");

    }

    delete ctype2;
    delete ktype2;

    /// 37. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% TIMEUNIT %%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ctimeunit = new char[cTIMEUNIT.length()+1];
    strcpy(ctimeunit,cTIMEUNIT.c_str());

    char * ktimeunit = new char[kTIMEUNIT.length()+1];
    strcpy(ktimeunit,kTIMEUNIT.c_str());

    if(fits_write_key(fptr,TSTRING,"TIMEUNIT",ktimeunit,ctype2,&status)){

        delete ctimeunit;
        delete ktimeunit;
        return printerror(status, "Error fits_write_key(TIMEUNIT)");

    }

    delete ctimeunit;
    delete ktimeunit;

    /// 38. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CD1_1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccd1_1 = new char[cCD1_1.length()+1];
    strcpy(ccd1_1,cCD1_1.c_str());

    if(fits_write_key(fptr,TDOUBLE,"CD1_1",&kCD1_1,ccd1_1,&status)){

        delete ccd1_1;
        return printerror(status, "Error fits_write_key(CD1_1)");

    }

    delete ccd1_1;

    /// 39. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CD1_2 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccd1_2 = new char[cCD1_2.length()+1];
    strcpy(ccd1_2,cCD1_2.c_str());

    if(fits_write_key(fptr,TDOUBLE,"CD1_2",&kCD1_2,ccd1_2,&status)){

        delete ccd1_2;
        return printerror(status, "Error fits_write_key(CD1_2)");

    }

    delete ccd1_2;

    /// 40. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CD2_1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccd2_1 = new char[cCD2_1.length()+1];
    strcpy(ccd2_1,cCD2_1.c_str());

    if(fits_write_key(fptr,TDOUBLE,"CD2_1",&kCD2_1,ccd2_1,&status)){

        delete ccd2_1;
        return printerror(status, "Error fits_write_key(CD2_1)");

    }

    delete ccd2_1;

    /// 41. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CD2_2 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccd2_2 = new char[cCD2_2.length()+1];
    strcpy(ccd2_2,cCD2_2.c_str());

    if(fits_write_key(fptr,TDOUBLE,"CD2_2",&kCD2_2,ccd2_2,&status)){

        delete ccd2_2;
        return printerror(status, "Error fits_write_key(CD2_2)");

    }

    delete ccd2_2;

    /// 42. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CD3_3 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccd3_3 = new char[cCD3_3.length()+1];
    strcpy(ccd3_3,cCD3_3.c_str());

    if(fits_write_key(fptr,TDOUBLE,"CD3_3",&kCD3_3,ccd3_3,&status)){

        delete ccd3_3;
        return printerror(status, "Error fits_write_key(CD3_3)");

    }

    delete ccd3_3;

    /// 43. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CRPIX1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccrpix1 = new char[cCRPIX1.length()+1];
    strcpy(ccrpix1,cCRPIX1.c_str());

    if(fits_write_key(fptr,TINT,"CRPIX1",&kCRPIX1,ccrpix1,&status)){

        delete ccrpix1;
        return printerror(status, "Error fits_write_key(CRPIX1)");

    }

    delete ccrpix1;

    /// 44. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CRPIX2 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccrpix2 = new char[cCRPIX2.length()+1];
    strcpy(ccrpix2,cCRPIX2.c_str());

    if(fits_write_key(fptr,TINT,"CRPIX2",&kCRPIX2,ccrpix2,&status)){

        delete ccrpix2;
        return printerror(status, "Error fits_write_key(CRPIX2)");

    }

    delete ccrpix2;

    /// 45. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CRVAL1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccrval1 = new char[cCRVAL1.length()+1];
    strcpy(ccrval1,cCRVAL1.c_str());

    if(fits_write_key(fptr,TDOUBLE,"CRVAL1",&kCRVAL1,ccrval1,&status)){

        delete ccrval1;
        return printerror(status, "Error fits_write_key(CRVAL1)");

    }

    delete ccrval1;

    /// 46. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% CRVAL2 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ccrval2 = new char[cCRVAL2.length()+1];
    strcpy(ccrval2,cCRVAL2.c_str());

    if(fits_write_key(fptr,TDOUBLE,"CRVAL2",&kSITELAT,ccrval2,&status)){

        delete ccrval2;
        return printerror(status, "Error fits_write_key(CRVAL2)");

    }

    delete ccrval2;

    /// 47. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% K1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ck1 = new char[cK1.length()+1];
    strcpy(ck1,cK1.c_str());

    if(fits_write_key(fptr,TDOUBLE,"K1",&kK1,ck1,&status)){

        delete ck1;
        return printerror(status, "Error fits_write_key(K1)");

    }

    delete ck1;

    /// 48. %%%%%%%%%%%%%%%%%%%%%%%%%%%%% K2 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    char * ck2 = new char[cK2.length()+1];
    strcpy(ck2,cK2.c_str());

    if(fits_write_key(fptr,TDOUBLE,"K2",&kK2,ck2,&status)){

        delete ck2;
        return printerror(status, "Error fits_write_key(K2)");

    }

    delete ck2;

    return true;


}

/******************************************************/
/* Create a FITS primary array containing a 2-D image */
/******************************************************/

bool Fits2D::writeFits(Mat img, ImgBitDepth imgType, vector<string> date, bool fileNameWithDate, string fileName){

    BOOST_LOG_SEV(log, normal) << " Start write Fits.";

    int status = 0;

    long  firstPixel, nbelements;

    // 2-dimensional image.
    long naxis = 2;

    // Image size.
    long naxes[2] = { img.cols, img.rows };
    BOOST_LOG_SEV(log, normal) << " Fits size :" << img.cols << "x" << img.rows;

    // First pixel to write.
	firstPixel = 1;

	// Number of pixels to write.
    nbelements = naxes[0] * naxes[1];

    // Get current date.
    string dateFile = "";
    vector<string> dateString;
    string creationDate = TimeDate::localDateTime(second_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(":");
    tokenizer tokens(creationDate, sep);
    for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
        dateString.push_back(*tok_iter);
    }
    kDATE = dateString.at(0) + "-" + dateString.at(1) + "-" + dateString.at(2) + "T" + dateString.at(3) + ":" + dateString.at(4) + ":" + dateString.at(5);

    if(date.size() == 6){

        dateFile	= date.at(0) + date.at(1) + date.at(2) + "_" + date.at(3) + date.at(4) + date.at(5);

    }

    fitsfile *fptr;

    const char * filename;

    // Creation of the fits filename.
    string pathAndname = "";

    if(fileNameWithDate){

        pathAndname = fitsPath + kTELESCOP + "_" + dateFile + "_UT.fit";
        kFILENAME = kTELESCOP + "_" + dateFile + "_UT.fit";

    }else{

        pathAndname = fitsPath + fileName + ".fit";
        kFILENAME = fileName + ".fit";
    }

    filename = pathAndname.c_str();
    BOOST_LOG_SEV(log, normal) << " Fits name : " << pathAndname;

    switch(imgType){

        case 0:
        {
            //https://www-n.oca.eu/pichon/Tableau_2D.pdf
            unsigned char ** tab = (unsigned char * *)malloc( img.rows * sizeof(unsigned char *)) ;

            if(tab == NULL){

                return printerror("Fits2D::writeimage() case 8 bits -> tab == NULL");
            }

            tab[0] = (unsigned char  *) malloc( naxes[0] * naxes[1] * sizeof(unsigned char) ) ;

            if(tab[0] == NULL){

                return printerror("Fits2D::writeimage() case 8 bits -> tab[0] == NULL");
            }

            for( int a = 1; a<naxes[1]; a++ ){

                tab[a] = tab[a-1] + naxes[0];
            }

            // Delete old file if it already exists.
            remove(filename);

            // Create new FITS file.
            if (fits_create_file(&fptr, filename, &status)){

                 return printerror( status, "Fits2D::writeimage() case 8 bits -> fits_create_file() failed" );
            }


            if (fits_create_img(fptr, BYTE_IMG, naxis, naxes, &status)){

                 return printerror( status, "Fits2D::writeimage() case 8 bits -> fits_create_img() failed" );
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

                 return printerror( status, "Fits2D::writeimage() case 8 bits -> fits_write_img() failed" );
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

                return printerror("Fits2D::writeimage() case 8 bits -> tab == NULL");
            }

            tab[0] = (char *) malloc( naxes[0] * naxes[1] * sizeof(char) ) ;

            if(tab[0] == NULL){

                return printerror("Fits2D::writeimage() case 8 bits -> tab[0] == NULL");
            }

            for( int a = 1; a<naxes[1]; a++ ){

                tab[a] = tab[a-1] + naxes[0];
            }

            // Delete old file if it already exists.
            remove(filename);

            // Create new FITS file.
            if (fits_create_file(&fptr, filename, &status)){

                 return printerror( status, "Fits2D::writeimage() case 8 signed bits -> fits_create_file() failed" );
            }


            if (fits_create_img(fptr,  SBYTE_IMG, naxis, naxes, &status)){

                 return printerror( status, "Fits2D::writeimage() case 8 signed bits -> fits_create_img() failed" );
            }

            // Initialize the values in the fits image with the mat's values.
             for ( int j = 0; j < naxes[1]; j++){

                 char * matPtr = img.ptr<char>(j);

                 for ( int i = 0; i < naxes[0]; i++){

                     // Affect a value and inverse the image.
                     tab[img.rows-1-j][i] = (char)matPtr[i];

                }
            }

            // Write the array of unsigned short to the FITS file.
             if(fits_write_img(fptr, TSBYTE, firstPixel, nbelements, tab[0], &status)){

                 return printerror( status, "Fits2D::writeimage() case 8 signed bits -> fits_write_img() failed" );
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

                return printerror("Fits2D::writeimage() case 16 bits -> tab == NULL");
            }

            tab[0] = (unsigned short  *) malloc( naxes[0] * naxes[1] * sizeof(unsigned short) ) ;

            if(tab[0] == NULL){

                return printerror("Fits2D::writeimage() case 16 bits -> tab[0] == NULL");
            }

            for( int a = 1; a<naxes[1]; a++ ){

              tab[a] = tab[a-1] + naxes[0];
            }

            // Delete old file if it already exists.
            remove(filename);

            // Create new FITS file.
            if (fits_create_file(&fptr, filename, &status)){

                 return printerror( status, "Fits2D::writeimage() case 16 bits -> fits_create_file() failed" );
            }

           // fits_set_compression_type(fptr, GZIP_1, &status);



            if ( fits_create_img(fptr,  USHORT_IMG, naxis, naxes, &status)){

                 return printerror( status, "Fits2D::writeimage() case 16 bits -> fits_create_img() failed" );
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

                 return printerror( status, "Fits2D::writeimage() case 16 bits -> fits_write_img() failed" );
            }

            // Free previously allocated memory.
            free( *tab);
            free( tab );

            break;

        }

        case 3 :
        {

            //https://www-n.oca.eu/pichon/Tableau_2D.pdf
            short ** tab = (short * *) malloc( img.rows * sizeof( short * ) ) ;

            if(tab == NULL){

                return printerror("Fits2D::writeimage() case 16 signed bits -> tab == NULL");
            }

            tab[0] = (short  *) malloc( naxes[0] * naxes[1] * sizeof(short) ) ;

            if(tab[0] == NULL){

                return printerror("Fits2D::writeimage() case 16 signed bits -> tab[0] == NULL");
            }

            for( int a = 1; a<naxes[1]; a++ ){

              tab[a] = tab[a-1] + naxes[0];
            }

            // Delete old file if it already exists.
            remove(filename);

            // Create new FITS file.
            if (fits_create_file(&fptr, filename, &status)){

                 return printerror( status, "Fits2D::writeimage() case 16 signed bits -> fits_create_file() failed" );
            }


            if ( fits_create_img(fptr,  SHORT_IMG, naxis, naxes, &status)){

                 return printerror( status, "Fits2D::writeimage() case 16 signed bits -> fits_create_img() failed" );
            }

            // Initialize the values in the fits image with the mat's values.
            for ( int j = 0; j < naxes[1]; j++){

                 short * matPtr = img.ptr<short>(j);

                 for ( int i = 0; i < naxes[0]; i++){

                     // Affect a value and inverse the image.
                     tab[img.rows-1-j][i] = (short)matPtr[i];
                }
            }

            // write the array of unsigned short to the FITS file
            if ( fits_write_img(fptr, TSHORT, firstPixel, nbelements, tab[0], &status)){

                 return printerror( status, "Fits2D::writeimage() case 16 signed bits -> fits_write_img() failed" );
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

                return printerror("Fits2D::writeimage() case 32 bits -> tab == NULL");
            }

            tab[0] = (float *) malloc( naxes[0] * naxes[1] * sizeof(float) ) ;

            if(tab[0] == NULL){

                return printerror("Fits2D::writeimage() case 32 bits -> tab[0] == NULL");
            }

            for( int a = 1; a<naxes[1]; a++ ){

                tab[a] = tab[a-1] + naxes[0];
            }

            // Delete old file if it already exists.
            remove(filename);

            // Create new FITS file
            if (fits_create_file(&fptr, filename, &status)){

                 return printerror( status, "Fits2D::writeimage() case 32 bits -> fits_create_file() failed" );
            }

            if ( fits_create_img(fptr,  FLOAT_IMG, naxis, naxes, &status)){

                 return printerror( status, "Fits2D::writeimage() case 32 bits -> fits_create_img() failed" );
            }

            // Initialize the values in the fits image with the mat's values.
            for ( int j = 0; j < naxes[1]; j++){

                 float * matPtr = img.ptr<float>(j);

                 for ( int i = 0; i < naxes[0]; i++){

                     // Affect a value and inversed the image.
                     tab[img.rows-1-j][i] = (float)matPtr[i];
                 }
            }

            // Write the array of unsigned short to the FITS file.
            if ( fits_write_img(fptr, TFLOAT, firstPixel, nbelements, tab[0], &status)){

                 return printerror( status, "Fits2D::writeimage() case 32 bits -> fits_write_img() failed" );
            }

            // Free previously allocated memory.
            free(*tab);
            free(tab);


            break;
        }

        default :

            return printerror("Fits2D::writeimage() -> bitDepth doesn't exists");

            break;

    }

    if(!writeKeywords(fptr)){

        if( fits_close_file(fptr, &status)){

             return printerror( status, "Fits2D::writeimage() -> fits_close_file() failed" );
        }

        return false;
    }

    if( fits_close_file(fptr, &status)){

        return printerror( status, "Fits2D::writeimage() -> fits_close_file() failed" );
    }

    return true;
}

//Float 32bits float -1.18*10-38~3.40*10-38
bool Fits2D::readFits32F(Mat &img, string filePath){

    float * ptr = NULL;
    float  * ptr1 = NULL;

    int status = 0,  nfound, anynull;
    long naxes[2], fpixel, nbuffer, npixels;

    float  nullval;


    fitsfile *fptr;

    const char * filename;

    filename = filePath.c_str();

    if(fits_open_file(&fptr, filename, READONLY, &status)){

        return printerror( status, "Fits2D::readFits32F() -> fits_open_file() failed" );
    }

    // Read the NAXIS1 and NAXIS2 keyword to get image size.
    if(fits_read_keys_lng(fptr, "NAXIS", 1, 2, naxes, &nfound, &status)){

        return printerror( status, "Fits2D::readFits32F() -> fits_read_keys_lng() failed" );
    }

    Mat image = Mat::zeros( naxes[1],naxes[0], CV_32FC1);

    npixels  = naxes[0] * naxes[1];         // number of pixels in the image
    fpixel   = 1;                           // first pixel
    nullval  = 0;                           // don't check for null values in the image

    nbuffer = npixels;

    float  buffer[npixels];

    if(fits_read_img(fptr, TFLOAT, fpixel, nbuffer, &nullval,buffer, &anynull, &status)){

        return printerror( status, "Fits2D::readFits32F() -> fits_read_img() failed" );
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

    if(fits_close_file(fptr, &status)){

        return printerror( status, "Fits2D::readFits32F() -> fits_close_file() failed" );
    }

    return true;

}

//Unsigned 16bits ushort 0~65535
bool Fits2D::readFits16US(Mat &img, string filePath){

    unsigned short * ptr = NULL;
    unsigned short  * ptr1 = NULL;

    int status = 0,  nfound, anynull;
    long naxes[2], fpixel, nbuffer, npixels;

    float  nullval;


    fitsfile *fptr;

    const char * filename;

    filename = filePath.c_str();

    if(fits_open_file(&fptr, filename, READONLY, &status)){

        return printerror( status, "Fits2D::readFits16US() -> fits_open_file() failed" );
    }

    // Read the NAXIS1 and NAXIS2 keyword to get image size.
    if(fits_read_keys_lng(fptr, "NAXIS", 1, 2, naxes, &nfound, &status)){

        return printerror( status, "Fits2D::readFits16US() -> fits_read_keys_lng() failed" );
    }

    Mat image = Mat::zeros( naxes[1],naxes[0], CV_16UC1);

    npixels  = naxes[0] * naxes[1];         // number of pixels in the image
    fpixel   = 1;                           // first pixel
    nullval  = 0;                           // don't check for null values in the image

    nbuffer = npixels;

    unsigned short  buffer[npixels];

    if(fits_read_img(fptr, TUSHORT, fpixel, nbuffer, &nullval,buffer, &anynull, &status)){

        return printerror( status, "Fits2D::readFits16US() -> fits_read_img() failed" );
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

    if(fits_close_file(fptr, &status)){

        return printerror( status, "Fits2D::readFits16US() -> fits_close_file() failed" );
    }

    return true;

}

//Signed 16bits short -32768~32767
bool Fits2D::readFits16S(Mat &img, string filePath){

    short * ptr = NULL;
    unsigned short  * ptr1 = NULL;

    int status = 0,  nfound, anynull;
    long naxes[2], fpixel, nbuffer, npixels;

    float  nullval;

     fitsfile *fptr;

    const char * filename;

    filename = filePath.c_str();

    if(fits_open_file(&fptr, filename, READONLY, &status)){

        return printerror( status, "Fits2D::readFits16S() -> fits_open_file() failed" );
    }

    // Read the NAXIS1 and NAXIS2 keyword to get image size.
    if(fits_read_keys_lng(fptr, "NAXIS", 1, 2, naxes, &nfound, &status)){

        return printerror( status, "Fits2D::readFits16S() -> fits_read_keys_lng() failed" );
    }

    Mat image = Mat::zeros( naxes[1],naxes[0], CV_16SC1);

    npixels  = naxes[0] * naxes[1];         // number of pixels in the image
    fpixel   = 1;                           // first pixel
    nullval  = 0;                           // don't check for null values in the image

    nbuffer = npixels;

    short  buffer[npixels];

    if(fits_read_img(fptr, TSHORT, fpixel, nbuffer, &nullval,buffer, &anynull, &status)){

        return printerror( status, "Fits2D::readFits16S() -> fits_read_img() failed" );
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

    if(fits_close_file(fptr, &status)){

        return printerror( status, "Fits2D::readFits16S() -> fits_close_file() failed" );
    }


    return true;

}

//Unsigned 8bits uchar 0~255
bool Fits2D::readFits8UC(Mat &img, string filePath){

    unsigned char * ptr = NULL;
    unsigned char  * ptr1 = NULL;

    int status = 0,  nfound, anynull;
    long naxes[2], fpixel, nbuffer, npixels;

    float  nullval;

    fitsfile *fptr;

    const char * filename;

    filename = filePath.c_str();

    if(fits_open_file(&fptr, filename, READONLY, &status)){

        return printerror( status, "Fits2D::readFits8UC() -> fits_open_file() failed" );
    }

    // Read the NAXIS1 and NAXIS2 keyword to get image size.
    if(fits_read_keys_lng(fptr, "NAXIS", 1, 2, naxes, &nfound, &status)){

        return printerror( status, "Fits2D::readFits8UC() -> fits_read_keys_lng() failed" );
    }

    Mat image = Mat::zeros( naxes[1],naxes[0], CV_8UC1);

    npixels  = naxes[0] * naxes[1];         // number of pixels in the image
    fpixel   = 1;                           // first pixel
    nullval  = 0;                           // don't check for null values in the image

    nbuffer = npixels;

    unsigned char  buffer[npixels];

    if(fits_read_img(fptr, TBYTE, fpixel, nbuffer, &nullval,buffer, &anynull, &status)){

        return printerror( status, "Fits2D::readFits8UC() -> fits_read_img() failed" );
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

    if(fits_close_file(fptr, &status)){

        return printerror( status, "Fits2D::readFits8UC() -> fits_close_file() failed" );
    }

    return true;

}

//Signed 8bits char -128~127
bool Fits2D::readFits8C(Mat &img, string filePath){

    char * ptr = NULL;
    char  * ptr1 = NULL;

    int status = 0,  nfound, anynull;
    long naxes[2], fpixel, nbuffer, npixels;

    float  nullval;

    fitsfile *fptr;

    const char * filename;

    filename = filePath.c_str();

    if(fits_open_file(&fptr, filename, READONLY, &status)){

        return printerror( status, "Fits2D::readFits8C() -> fits_open_file() failed" );
    }

    // Read the NAXIS1 and NAXIS2 keyword to get image size.
    if(fits_read_keys_lng(fptr, "NAXIS", 1, 2, naxes, &nfound, &status)){

        return printerror( status, "Fits2D::readFits8C() -> fits_read_keys_lng() failed" );
    }

    Mat image = Mat::zeros( naxes[1],naxes[0], CV_8SC1);

    npixels  = naxes[0] * naxes[1];         // number of pixels in the image
    fpixel   = 1;                           // first pixel
    nullval  = 0;                           // don't check for null values in the image

    nbuffer = npixels;

    char  buffer[npixels];

    if(fits_read_img(fptr, TSBYTE, fpixel, nbuffer, &nullval,buffer, &anynull, &status)){

        return printerror( status, "Fits2D::readFits8C() -> fits_read_img() failed" );
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

    if(fits_close_file(fptr, &status)){

        return printerror( status, "Fits2D::readFits8C() -> fits_close_file() failed" );
    }

    return true;

}

bool Fits2D::readIntKeyword(string filePath, string keyword, int &value){

    char *ptr = NULL;

    int status = 0;

    fitsfile *fptr;

    const char * filename;

    filename = filePath.c_str();

    if(fits_open_file(&fptr, filename, READONLY, &status)){

        return printerror( status, "> Failed to open the fits file." );
    }

    char * key = new char[keyword.length()+1];
    strcpy(key,keyword.c_str());

    if(fits_read_key(fptr, TINT, key, &value, NULL, &status)){

        return printerror( status, "> Failed to read the fits keyword." );
    }

    delete key;

    return true;
}


bool Fits2D::printerror( int status, string errorMsg){

    if (status){

        fits_report_error(stderr, status);

        cout << stderr << endl;
        BOOST_LOG_SEV(log, normal) << stderr;

    }

    if(errorMsg != ""){

        cout << errorMsg << endl;
        BOOST_LOG_SEV(log, normal) << errorMsg;

    }

    return false;
}

void Fits2D::printerror( int status ){

    if (status){

        fits_report_error(stderr, status);


    }
}

bool Fits2D::printerror( string errorMsg){

    if(errorMsg != ""){

        cout << errorMsg << endl;
        BOOST_LOG_SEV(log, normal) << errorMsg;

    }

    return false;
}
