/*
                                TimeDate.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
* \file    TimeDate.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    13/06/2014
* \brief   Manage time.
*/

#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <boost/tokenizer.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

using namespace std;
using namespace boost::posix_time;

class TimeDate {

    public :

        static string localDateTime(::boost::posix_time::ptime pt, string format);

        static double gregorianToJulian_1(vector<int> date);

        static double gregorianToJulian_2(vector<int> date);

        static double julianCentury(double julianDate);

        static double hmsToHdecimal(int H, int M, int S);

        static double degreeToHdecimal(int val);

        static vector<int> HdecimalToHMS(double val);

        static double julianDateFromPreviousMidnightUT(int gregorianH, int gregorianMin, int gregorianS, double JD0);

        static double localSideralTime_2(double julianCentury, int gregorianH, int gregorianMin, int gregorianS, int longitude);

        static double localSideralTime_2(double julianCentury, int gregorianH, int gregorianMin, float gregorianS, float longitude);

        static double localSideralTime_1(double JD0, int gregorianH, int gregorianMin, int gregorianS);

        static vector<string> splitStringToStringVector(string str);

        static vector<int> splitStringToIntVector(string str);

        // Date input format : YYYY-MM-DDTHH:MM:SS.fffffffff
        static string get_YYYYMMDD_fromDateString(string date);

        // Date input format : YYYY-MM-DDTHH:MM:SS.fffffffff
        static vector<int> getIntVectorFromDateString(string date);

        // Date input format	: YYYYMMJJTHHMMSS.fffffffff
        // Date output format	: YYYYMMJJTHHMMSS
        static string get_YYYYMMDDThhmmss(string date);

};

