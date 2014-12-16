/*
								EnumParser.cpp

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
*	Last modified:		04/12/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
 * @file    EnumParser.cpp
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    04/12/2014
 *
 */

#include "EnumParser.h"

template<> EnumParser<CamBitDepth>::EnumParser(){

    enumMap["MONO_8"]   = MONO_8;
    enumMap["MONO_12"]  = MONO_12;

}

template<> EnumParser<CamType>::EnumParser(){
cout <<"here : " <<DMK << endl;
    enumMap["DMK"]      = DMK;
    enumMap["BASLER"]   = BASLER;
    enumMap["VIDEO"]    = VIDEO;
    enumMap["FRAMES"]   = FRAMES;

}

template<> EnumParser<AstStackMeth>::EnumParser(){

    enumMap["SUM"]      = SUM;
    enumMap["MEAN"]     = MEAN;

}

template<> EnumParser<DetMeth>::EnumParser(){

    enumMap["HOUGH"]    = HOUGH;
    enumMap["LIST"]     = LIST;

}
