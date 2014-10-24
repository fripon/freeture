/*
								Fits.cpp

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

#include "Fits.h"

Fits::Fits(){

    kFILENAME   = "" ;
    kEXPOSURE   = 0.0;
    kTELESCOP   = "";
    kOBSERVER   = "";
    kINSTRUME   = "";
    kCAMERA     = "";
    kFOCAL      = 0.0;
    kFILTER     = "";
    kPROGRAM    = "";
    kCREATOR    = "";
    kAPERTURE   = 0.0;
    kXPIXEL     = 0.0;
    kYPIXEL     = 0.0;
    kSITELONG   = 0.0;
    kSITELAT    = 0.0;
    kSITEELEV   = 0.0;
    kGAINDB     = 0;
    kCENTAZ     = "";
    kCENTALT    = "";
    kCENTOR     = "";
    kCRPIX1     = 0;
    kCRPIX2     = 0;
    kK1         = 0.0;
    kK2         = 0.0;
    kCOMMENT    = "";
    kCD1_1      = 0.0;
    kCD1_2      = 0.0;
    kCD2_1      = 0.0;
    kCD2_2      = 0.0;
    kOBSMODE    = 0;
    kDATEOBS    = "";
    kSATURATE   = 0.0;
    kONTIME     = 0.0;
    kRADESYS    = "";
    kEQUINOX    = 0.0;
    kCTYPE1     = "";
    kCTYPE2     = "";
    kELAPTIME   = 0;
    kCRVAL2     = 0.0;
    kCRVAL1     = 0.0;

}

Fits::~Fits(){
    //dtor
}

bool Fits::loadKeywordsFromConfigFile(string configFile){

    namespace fs = boost::filesystem;

    path p(configFile);

    if(fs::exists(p)){

        Configuration getFitsKeys;
        getFitsKeys.Load(configFile);

        getFitsKeys.Get("TELESCOP", kTELESCOP);
        getFitsKeys.Get("OBSERVER", kOBSERVER);
        getFitsKeys.Get("INSTRUME", kINSTRUME);
        getFitsKeys.Get("CAMERA",   kCAMERA);
        getFitsKeys.Get("FOCAL",    kFOCAL);
        getFitsKeys.Get("APERTURE", kAPERTURE);
        getFitsKeys.Get("SITELONG", kSITELONG);
        getFitsKeys.Get("SITELAT",  kSITELAT);
        getFitsKeys.Get("SITEELEV", kSITEELEV);
        getFitsKeys.Get("CENTAZ",   kCENTAZ);
        getFitsKeys.Get("CENTALT",  kCENTALT);
        getFitsKeys.Get("CENTOR",   kCENTOR);
        getFitsKeys.Get("CRPIX1",   kCRPIX1);
        getFitsKeys.Get("CRPIX2",   kCRPIX2);
        getFitsKeys.Get("K1",       kK1);
        getFitsKeys.Get("K2",       kK2);
        getFitsKeys.Get("PROGRAM",  kPROGRAM);
        getFitsKeys.Get("FILTER",   kFILTER);
        getFitsKeys.Get("CREATOR",  kCREATOR);
        getFitsKeys.Get("CD1_1",    kCD1_1);
        getFitsKeys.Get("CD1_2",    kCD1_2);
        getFitsKeys.Get("CD2_1",    kCD2_1);
        getFitsKeys.Get("CD2_2",    kCD2_2);
        getFitsKeys.Get("XPIXEL",   kXPIXEL);
        getFitsKeys.Get("YPIXEL",   kYPIXEL);
        getFitsKeys.Get("COMMENT",  kCOMMENT);

        return true;

    }else{

        cout << " The path of the configuration file doesn't exists" << endl;

        return false;

    }

}

//%%%%%%%%%%%%%%%%%%% GETTER %%%%%%%%%%%%%%%%%%%%%%%%%%%%

string Fits::getFilename(){

    return kFILENAME;

}

double Fits::getExposure(){

    return kEXPOSURE;

}

string Fits::getTelescop(){

    return kTELESCOP;

}

string Fits::getObserver(){

    return kOBSERVER;

}

string Fits::getInstrument(){

    return kINSTRUME;

}

string Fits::getCamera(){

    return kCAMERA;

}

double Fits::getFocal(){

    return kFOCAL;

}

string Fits::getFilter(){

    return kFILTER;

}

string Fits::getProgram(){

    return kPROGRAM;

}

string Fits::getCreator(){

    return kCREATOR;

}

double Fits::getAperture(){

    return kAPERTURE;

}

double Fits::getXpixel(){

    return kXPIXEL;

}

double Fits::getYpixel(){

    return kYPIXEL;

}

double Fits::getSitelong(){

    return kSITELONG;

}

double Fits::getSitelat(){

    return kSITELAT;

}

double Fits::getSiteelev(){

    return kSITEELEV;

}

int Fits::getGaindb(){

    return kGAINDB;

}

string Fits::getCentaz(){

    return kCENTAZ;

}

string Fits::getCentalt(){

    return kCENTALT;

}

string Fits::getCentor(){

    return kCENTOR;

}

int Fits::getCrpix1(){

    return kCRPIX1;

}

int Fits::getCrpix2(){

    return kCRPIX2;

}

double Fits::getK1(){

    return kK1;

}

double Fits::getK2(){

    return kK2;

}

string Fits::getComment(){

    return kCOMMENT;

}

double Fits::getCd1_1(){

    return kCD1_1;

}

double Fits::getCd1_2(){

    return kCD1_2;

}

double Fits::getCd2_1(){

    return kCD2_1;

}

double Fits::getCd2_2(){

    return kCD2_2;

}

int Fits::getObsmode(){

    return kOBSMODE;

}

string Fits::getDateobs(){

    return kDATEOBS;

}

double Fits::getSaturate(){

    return kSATURATE;

}

double Fits::getOntime(){

    return kONTIME;

}

string Fits::getRadesys(){

    return kRADESYS;

}

double Fits::getEquinox(){

    return kEQUINOX;

}

string Fits::getCtype1(){

    return kCTYPE1;

}

string Fits::getCtype2(){

    return kCTYPE2;

}

int Fits::getElaptime(){

    return kELAPTIME;

}

double Fits::getCrval2(){

    return kCRVAL2;

}

double Fits::getCrval1(){

    return kCRVAL1;

}

//%%%%%%%%%%%%%%%%%%% SETTER %%%%%%%%%%%%%%%%%%%%%%%%%%%%

void Fits::setFilename(string filename){

    kFILENAME = filename;

}

void Fits::setExposure(double exposure){

    kEXPOSURE = exposure;

}

void Fits::setTelescop(string telescope){

    kTELESCOP = telescope;

}

void Fits::setObserver(string observer){

    kOBSERVER = observer;

}

void Fits::setInstrument(string instrument){

    kINSTRUME = instrument;

}

void Fits::setCamera(string camera){

    kCAMERA = camera;

}

void Fits::setFocal(double focal){

    kFOCAL = focal;

}

void Fits::setFilter(string filter){

    kFILTER = filter;

}

void Fits::setProgram(string program){

    kPROGRAM = program;

}

void Fits::setCreator(string creator){

    kCREATOR = creator;

}

void Fits::setAperture(double aperture){

    kAPERTURE = aperture;

}

void Fits::setXpixel(double xpixel){

    kXPIXEL = xpixel;

}

void Fits::setYpixel(double ypixel){

    kYPIXEL = ypixel;

}

void Fits::setSitelong(double sitelong){

    kSITELONG = sitelong;

}

void Fits::setSitelat(double sitelat){

    kSITELAT = sitelat;

}

void Fits::setSiteelev(double siteelev){

    kSITEELEV = siteelev;

}

void Fits::setGaindb(int gaindb){

    kGAINDB = gaindb;

}

void Fits::setCentaz(string centaz){

    kCENTAZ = centaz;

}

void Fits::setCentalt(string centalt){

    kCENTALT = centalt;

}

void Fits::setCentor(string centor){

    kCENTOR = centor;

}

void Fits::setCrpix1(int crpix1){

    kCRPIX1 = crpix1;

}

void Fits::setCrpix2(int crpix2){

    kCRPIX2 = crpix2;

}

void Fits::setK1(double k1){

    kK1 = k1;

}

void Fits::setK2(double k2){

    kK2 = k2;

}

void Fits::setComment(string comment){

    kCOMMENT = comment;

}

void Fits::setCd1_1(double cd1_1){

    kCD1_1 = cd1_1;

}

void Fits::setCd1_2(double cd1_2){

    kCD1_2 = cd1_2;

}

void Fits::setCd2_1(double cd2_1){

    kCD2_1 = cd2_1;

}

void Fits::setCd2_2(double cd2_2){

    kCD2_2 = cd2_2;

}

void Fits::setObsmode(int obsmode){

    kOBSMODE = obsmode;

}

void Fits::setDateobs(string dateobs){

     kDATEOBS = dateobs;

}

void Fits::setSaturate(double saturate){

    kSATURATE = saturate;

}

void Fits::setOntime(double ontime){

    kONTIME = ontime;

}

void Fits::setRadesys(string radesys){

    kRADESYS = radesys;

}

void Fits::setEquinox(double equinox){

    kEQUINOX = equinox;

}

void Fits::setCtype1(string ctype1){

    kCTYPE1 = ctype1;

}

void Fits::setCtype2(string ctype2){

    kCTYPE2 = ctype2;

}

void Fits::setElaptime(int elaptime){

    kELAPTIME = elaptime;

}

void Fits::setCrval2(double crval2){

    kCRVAL2 = crval2;

}

void Fits::setCrval1(double crval1){

    kCRVAL1 = crval1;

}

