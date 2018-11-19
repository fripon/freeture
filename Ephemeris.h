/*
                                    Ephemeris.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*   This file is part of:   freeture
*
*   Copyright:      (C) 2014-2015 Lucie Maquet
*                               IMCCE
*                   (C) 2014-2015 Yoan Audureau
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
*   Last modified:      06/07/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    Ephemeris.h
* \author  Lucie Maquet, Yoan Audureau
* \version 1.0
* \date    06/07/2015
*/

#pragma once
#include <math.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include "TimeDate.h"

using namespace std;

class Ephemeris {

    private :

        double mPI;
        int mDay;
        int mMonth;
        int mYear;
        double mSunElevation;
        double mStationLongitude;
        double mStationLatitude;

    public :

        Ephemeris(string date, double sunElevation, double longitude, double latitude);
        ~Ephemeris();
        bool computeEphemeris(int &sunriseHours, int &sunriseMinutes, int &sunsetHours, int &sunsetMinutes);

    private :

        double dateToJulianDate();
        double longitudeSun(double JJ);
        double obliquity(double JJ);

};
