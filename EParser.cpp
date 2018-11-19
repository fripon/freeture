/*
                                EParser.cpp

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
*   Last modified:      04/12/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    EParser.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    04/12/2014
* \brief   Parse some parameters in the configuration file with FreeTure's enumerations.
*/

#include "EParser.h"

template<> EParser<CamPixFmt>::EParser() {

    enumMap["MONO8"]   = MONO8;
    enumMap["GREY"]    = GREY;
    enumMap["Y800"]    = Y800;
    enumMap["MONO12"]  = MONO12;
    enumMap["YUYV"]    = YUYV;
    enumMap["UYVY"]    = UYVY;
    enumMap["RGB565"]  = RGB565;
    enumMap["BGR3"]    = BGR3;
    enumMap["RGB3"]    = RGB3;
    enumMap["MONO16"]  = MONO16;

}

template<> EParser<SmtpSecurity>::EParser() {

    enumMap["NO_SECURITY"]  = NO_SECURITY;
    enumMap["USE_TLS"]      = USE_TLS;
    enumMap["USE_SSL"]      = USE_SSL;

}

template<> EParser<InputDeviceType>::EParser() {

    enumMap["CAMERA"]               = CAMERA;
    enumMap["VIDEO"]                = VIDEO;
    enumMap["UNDEFINED_INPUT_TYPE"] = UNDEFINED_INPUT_TYPE;
    enumMap["SINGLE_FITS_FRAME"]    = SINGLE_FITS_FRAME;

}

template<> EParser<StackMeth>::EParser() {

    enumMap["SUM"]      = SUM;
    enumMap["MEAN"]     = MEAN;

}

template<> EParser<DetMeth>::EParser() {

    enumMap["TEMPORAL_MTHD"]    = TEMPORAL_MTHD;
    enumMap["TEMPLATE_MTHD"]    = TEMPLATE_MTHD;

}

template<> EParser<LogSeverityLevel>::EParser() {

    enumMap["normal"]           = normal;
    enumMap["notification"]     = notification;
    enumMap["fail"]             = fail;
    enumMap["warning"]          = warning;
    enumMap["critical"]         = critical;

}

template<> EParser<TimeMode>::EParser() {

    enumMap["DAY"]          = DAY;
    enumMap["NIGHT"]        = NIGHT;
    enumMap["DAYNIGHT"]     = DAYNIGHT;

}

template<> EParser<ImgFormat>::EParser() {

    enumMap["JPEG"]        = JPEG;
    enumMap["FITS"]        = FITS;

}

template<> EParser<CamSdkType>::EParser() {

    enumMap["ARAVIS"]        = ARAVIS;
    enumMap["PYLONGIGE"]     = PYLONGIGE;
    enumMap["TIS"]           = TIS;
    enumMap["VIDEOFILE"]     = VIDEOFILE;
    enumMap["FRAMESDIR"]     = FRAMESDIR;
    enumMap["V4L2"]          = V4L2;
    enumMap["VIDEOINPUT"]    = VIDEOINPUT;
    enumMap["UNKNOWN"]       = UNKNOWN;

}

