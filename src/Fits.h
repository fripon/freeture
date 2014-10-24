/*
								Fits.h

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
*	Last modified:		21/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
 * @file    Fits.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    21/10/2014
 */

#pragma once

#include "includes.h"
#include "Configuration.h"

#include <boost/filesystem.hpp>

using namespace boost::filesystem;
using namespace std;

class Fits{

    public:

        Fits();
        ~Fits();

        bool loadKeywordsFromConfigFile(string configFile);

        string  getCamera();
        string  getFilename();
        double  getExposure();
        string  getTelescop();
        string  getObserver();
        string  getInstrument();
        double  getFocal();
        string  getFilter();
        string  getProgram();
        string  getCreator();
        double  getAperture();
        double  getXpixel();
        double  getYpixel();
        double  getSitelong();
        double  getSitelat();
        double  getSiteelev();
        int     getGaindb();
        string  getCentaz();
        string  getCentalt();
        string  getCentor();
        int     getCrpix1();
        int     getCrpix2();
        double  getK1();
        double  getK2();
        string  getComment();
        double  getCd1_1();
        double  getCd1_2();
        double  getCd2_1();
        double  getCd2_2();
        int     getObsmode();
        string  getDateobs();
        double  getSaturate();
        double  getOntime();
        string  getRadesys();
        double  getEquinox();
        string  getCtype1();
        string  getCtype2();
        int     getElaptime();
        double  getCrval2();
        double  getCrval1();

        void setFilename(string filename);
        void setExposure(double exposure);
        void setTelescop(string telescope);
        void setObserver(string observer);
        void setInstrument(string instrument);
        void setCamera(string camera);
        void setFocal(double focal);
        void setFilter(string filter);
        void setProgram(string program);
        void setCreator(string creator);
        void setAperture(double aperture);
        void setXpixel(double xpixel);
        void setYpixel(double ypixel);
        void setSitelong(double sitelong);
        void setSitelat(double sitelat);
        void setSiteelev(double siteelev);
        void setGaindb(int gaindb);
        void setCentaz(string centaz);
        void setCentalt(string centalt);
        void setCentor(string centor);
        void setCrpix1(int crpix1);
        void setCrpix2(int crpix2);
        void setK1(double k1);
        void setK2(double k2);
        void setComment(string comment);
        void setCd1_1(double cd1_1);
        void setCd1_2(double cd1_2);
        void setCd2_1(double cd2_1);
        void setCd2_2(double cd2_2);
        void setObsmode(int obsmode);
        void setDateobs(string dateobs);
        void setSaturate(double saturate);
        void setOntime(double ontime);
        void setRadesys(string radesys);
        void setEquinox(double equinox);
        void setCtype1(string ctype1);
        void setCtype2(string ctype2);
        void setElaptime(int elaptime);
        void setCrval2(double crval2);
        void setCrval1(double crval1);

    protected:

        //Fits Header Keywords
		string  kFILENAME;
		double  kEXPOSURE;
		string  kTELESCOP;
		string  kOBSERVER;
		string  kINSTRUME;
		string  kCAMERA;
		double  kFOCAL;
		string  kFILTER;
		string  kPROGRAM;
		string  kCREATOR;
		double  kAPERTURE;
		double  kXPIXEL;
		double  kYPIXEL;
		double  kSITELONG;
		double  kSITELAT;
		double  kSITEELEV;
		int     kGAINDB;
		string  kCENTAZ;
		string  kCENTALT;
		string  kCENTOR;
		int     kCRPIX1;
		int     kCRPIX2;
		double  kK1;
		double  kK2;
		string  kCOMMENT;
		double  kCD1_1;
		double  kCD1_2;
		double  kCD2_1;
		double  kCD2_2;
		int     kOBSMODE;
		string  kDATEOBS;
		double  kSATURATE;
		double  kONTIME;
		string  kRADESYS;
		double  kEQUINOX;
		string  kCTYPE1;
		string  kCTYPE2;
		int     kELAPTIME;
		double  kCRVAL2;
		double  kCRVAL1;


};

