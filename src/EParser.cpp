/*
								EParser.cpp

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
*	Last modified:		04/12/2014
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

template<> EParser<CamBitDepth>::EParser(){

    enumMap["MONO_8"]   = MONO_8;
    enumMap["MONO_12"]  = MONO_12;

}

template<> EParser<CamType>::EParser(){

    enumMap["DMK_GIGE"]		= DMK_GIGE;
    enumMap["BASLER_GIGE"]		= BASLER_GIGE;
	enumMap["TYTEA_USB"]		= TYTEA_USB;
    enumMap["VIDEO"]			= VIDEO;
    enumMap["FRAMES"]			= FRAMES;

}

template<> EParser<StackMeth>::EParser(){

    enumMap["SUM"]      = SUM;
    enumMap["MEAN"]     = MEAN;

}

template<> EParser<DetMeth>::EParser(){

    enumMap["HOUGH_MTHD"]       = HOUGH_MTHD;
    enumMap["TEMPORAL_MTHD"]    = TEMPORAL_MTHD;
    enumMap["TEMPORAL_MTHD_"]    = TEMPORAL_MTHD_;
    enumMap["DAYTIME_MTHD"]     = DAYTIME_MTHD;

}

template<> EParser<LogSeverityLevel>::EParser(){

    enumMap["normal"]           = normal;
    enumMap["notification"]     = notification;
    enumMap["fail"]             = fail;
    enumMap["warning"]          = warning;
    enumMap["critical"]         = critical;

}

