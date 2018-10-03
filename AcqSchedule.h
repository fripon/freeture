/*
                                AcqSchedule.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*   This file is part of:   freeture
*
*   Copyright:      (C) 2014-2015 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
*   Last modified:      20/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    AcqSchedule.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    19/06/2014
* \brief
*/

#pragma once

#include <stdio.h>
#include <iostream>

using namespace std;

class AcqSchedule{

    private:

        int     mH;     // Hours
        int     mM;     // Minutes
        int     mS;     // Seconds
        int     mE;     // Exposure time
        int     mG;     // Gain
        int     mN;     // Repetition number
        int     mF;     // Format
        string  mDate;

    public:

        /**
        * Constructor.
        *
        * @param H Hour.
        * @param M Minutes.
        * @param S Seconds.
        * @param E Exposure time.
        * @param G Gain.
        * @param F Format.
        * @param N Repetition number.
        */
        AcqSchedule(int H, int M, int S, int E, int G, int F, int N);

        /**
        * Constructor.
        *
        */
        AcqSchedule();

        /**
        * Destructor.
        *
        */
        ~AcqSchedule();

        /**
        * Get acquisition hours.
        *
        * @return Hours.
        */
        int getH() {return mH;};

        /**
        * Get acquisition minutes.
        *
        * @return Minutes.
        */
        int getM() {return mM;};

        /**
        * Get acquisition seconds.
        *
        * @return Seconds.
        */
        int getS() {return mS;};

        /**
        * Get acquisition exposure time value.
        *
        * @return Exposure time.
        */
        int getE() {return mE;};

        /**
        * Get acquisition gain.
        *
        * @return Gain.
        */
        int getG() {return mG;};

        /**
        * Get acquisition format.
        *
        * @return Format : 8 or 12.
        */
        int getF() {return mF;};

        /**
        * Get acquisition repetition number.
        *
        * @return Repetition number.
        */
        int getN() {return mN;};

        /**
        * Set acquisition date.
        *
        * @param Date : YYYY-MM-DDTHH:MM:SS,fffffffff
        */
        void setDate(string date) {mDate = date;};

        /**
        * Get acquisition date.
        *
        * @return Date : YYYY-MM-DDTHH:MM:SS,fffffffff
        */
        string getDate() {return mDate;};

};
