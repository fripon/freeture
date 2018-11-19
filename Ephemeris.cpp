/*
                                Ephemeris.cpp

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
* \file    Ephemeris.cpp
* \author  Lucie Maquet, Yoan AUdureau
* \version 1.0
* \date    06/07/2015
*/

#include "Ephemeris.h"

Ephemeris::Ephemeris(string date, double sunElevation, double longitude, double latitude) {

    mPI = 4.0 * atan(1.0);

    mYear = atoi(date.substr(0,4).c_str());
    mMonth = atoi(date.substr(4,2).c_str());
    mDay = atoi(date.substr(6,2).c_str());

    mSunElevation = sunElevation;
    mStationLongitude = -longitude;

    if(latitude > -60 && latitude < 60)
        mStationLatitude = latitude;
    else
        throw "Latitude must have a value between -60 and 60 degree.";

}

Ephemeris::~Ephemeris() {

}

bool Ephemeris::computeEphemeris(int &sunriseHours, int &sunriseMinutes, int &sunsetHours, int &sunsetMinutes) {

    double DJJ = dateToJulianDate();

    double LS = longitudeSun(DJJ);

    LS = LS+(-993.0+17.0*cos(3.10+62830.14*((((DJJ-2451545.0)/3652500.0)-2451545.0)/3652500.0)))*pow(10,-7);

    LS = LS+(-834.0*sin(2.18-3375.7*(((DJJ-2451545.0)/3652500.0))+0.36*pow(((DJJ-2451545.0)/3652500.0),2))-64.0*
         sin(3.51-125666.39*((DJJ-2451545.0)/3652500.0)+0.10*pow(((DJJ-2451545.0)/3652500.0),2)))*pow(10,-7);

    double obliq = obliquity(DJJ);

    double alpS = atan2((cos(obliq)*sin(LS)),(cos(LS)));

    double delS = asin(sin(obliq)*sin(LS));

    double height = mSunElevation*mPI/180.0;

    double lat=mStationLatitude*mPI/180.0;

    double HO = acos((sin(height)-sin(lat)*sin(delS))/(cos(lat)*cos(delS)));

    // Check.
    if((((height-sin(lat)*sin(delS))/(cos(lat)*cos(delS))) < -1) || (((height-sin(lat)*sin(delS))/(cos(lat)*cos(delS)))>1)){

        return false;

    }else{

        //!******************************************************!
        //!                 Compute sunset time
        //!******************************************************!

        double TC = alpS + HO;

        // Compute sideral time at 0h

        double DJ0 = DJJ-0.5; // Julian date at 0h TU

        double TS0 = 1.75337+6.2833197*(DJ0-2451545.0)/365.25;

        double TSC = TC-TS0;

        TSC = fmod((fmod(TSC,2*mPI)+2*mPI),2*mPI);

        double longg = mStationLongitude*mPI/180.0;

        double TSC_TU = (TSC+longg)*0.9972696;

        // Sunset time in decimal hours.
        TSC_TU = TSC_TU*12.0/mPI;

        double t1;
        modf(TSC_TU,&t1);

        sunsetHours = int(t1);
        sunsetMinutes = int((TSC_TU - double(t1))*60.0);

        //!******************************************************!
        //!                 Compute sunrise time
        //!******************************************************!

        DJJ = DJJ+1.0;

        //! Compute sun longitude in radians.

        LS = longitudeSun(DJJ);
        LS = LS+(-993.0+17.0*cos(3.10+62830.14*((DJJ-2451545.0)/3652500.0)))*pow(10,-7);
        double  DJJtemp=(DJJ-2451545.0)/3652500.0;
        LS = LS+(-834.0*sin(2.18-3375.7*(DJJtemp)+0.36*pow(DJJtemp,2))-64.0*sin(3.51-125666.39*(DJJtemp)+0.10*pow(DJJtemp,2)))*pow(10,-7);

        //! Compute sun right ascension and delination in radians.

        obliq = obliquity(DJJ);
        alpS = atan2((cos(obliq)*sin(LS)),(cos(LS)));
        delS = asin(sin(obliq)*sin(LS));

        //! Compute sun hour angle.

        height = mSunElevation*mPI/180.0;
        lat = mStationLatitude*mPI/180.0;
        HO = acos((sin(height)-sin(lat)*sin(delS))/(cos(lat)*cos(delS)));

        // Check.
        if((((height-sin(lat)*sin(delS))/(cos(lat)*cos(delS)))<-1)||(((height-sin(lat)*sin(delS))/(cos(lat)*cos(delS)))>1)){

            return false;

        }else {

            double TL = alpS-HO;

            // Compute sideral time at 0h

            DJ0 = DJJ-0.5; // Julian date at 0h TU

            TS0 = 1.75337+6.2833197*(DJ0-2451545.0)/365.25;

            double TSL = TL-TS0;

            TSL = fmod((fmod(TSL,2*mPI)+2*mPI),2*mPI);

            longg = mStationLongitude*mPI/180.0;

            double TSL_TU = (TSL+longg)*0.99726960;

            TSL_TU = TSL_TU*12.0/mPI;

            double t3;
            modf(TSL_TU,&t3);

            sunriseHours = int(t3);
            sunriseMinutes = int((TSL_TU - double(t3))*60.0);

        }

    }

    return true;

}

double Ephemeris::dateToJulianDate(){

    double HD = 12.0;

    double Date = double(mYear) + double(mMonth)/12.0 + double(mDay)/365.25;
    double Ye, Mo;

    if(mMonth <3){

        Ye = double(mYear) - 1.0;
        Mo = double(mMonth) + 12.0;

    }else {

        Ye = double(mYear);
        Mo = double(mMonth);

    }

    double Si;
    modf(Ye/100.0,&Si);
    double temp;
    modf(Si/4.0,&temp);
    double B = 2.0 - Si + temp;
    double Dd = double(mDay) + HD/24.0;
    double temp1, temp2;
    modf(365.25 * (Ye+4716.0),&temp1);
    modf(30.60010*(Mo+1.0),&temp2);
    double JJ = temp1 + temp2 + Dd + B - 1524.0;
    return JJ;

}

double Ephemeris::longitudeSun(double JJ) {

    double ll[50] = {403406.0,195207.0,119433.0,112392.0,3891.0,2819.0,1721.0,660.0,
                    350.0,334.0,314.0,268.0,242.0,234.0,158.0,132.0,129.0,114.0,99.0,
                    93.0,86.0,78.0,72.0,68.0,64.0,46.0,38.0,37.0,32.0,29.0,28.0,27.0,
                    27.0,25.0,24.0,21.0,21.0,20.0,18.0,17.0,14.0,13.0,13.0,13.0,12.0,
                    10.0,10.0,10.0,10.0};

    double al[50] = {4.7219640, 5.9374580,1.1155890,5.7816160,5.5474000,1.5120000,4.1897000,
                    5.4150000,4.3150000,4.553000,5.198000,5.989000,2.911000,1.423000,0.061000,
                    2.317000,3.193000,2.828000,0.520000,4.650000,4.350000,2.750000,4.500000,
                    3.230000,1.220000,0.140000,3.440000,4.370000,1.140000,2.840000,5.960000,
                    5.090000,1.720000,2.560000,1.920000,0.090000,5.980000,4.030000,4.270000,0.790000,
                    4.240000,2.010000,2.650000,4.980000,0.930000,2.210000,3.590000,1.500000,2.550000};

    double ml[50] = {1.6210430,62830.3480670,62830.8215240,62829.6343020,125660.5691000,125660.9845,
                    62832.4766,125659.31,57533.85,-33.931,777137.715,78604.191,5.412,39302.098,-34.861,
                    115067.698,15774.337,5296.67,58849.27,5296.11,-3980.7,52237.69,55076.47,261.08,15773.85,
                    188491.03,-7756.55,264.89,117906.27,55075.75,-7961.39,188489.81,2132.19,109771.03,54868.56,
                    25443.93,55731.43,60697.74,2132.79,109771.63,-7752.82,188491.91,207.81,29424.63,-7.99,
                    46941.14,-68.29,21463.25,157208.4};


    double longs = 4.9353929+62833.196168*((JJ-2451545.0)/3652500.0);

    for(int i = 0; i < 50; i++)
        longs = longs+pow(10,-7)*ll[i]*sin(al[i]+ml[i]*((JJ-2451545.0)/3652500.0));

    return longs;

}

double Ephemeris::obliquity(double JJ) {

    double T = (JJ-2451545.0)/3652500.0;
    double earth_ob = 0.4090928+(-226938*T-75*(pow(T,2))+96926*(pow(T,3))-2491*(pow(T,4))-12104*(pow(T,5)))*pow(10,-7);
    return earth_ob;

}
