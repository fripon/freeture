/*
                                    Conversion.h

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
*   Last modified:      20/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    Conversion.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    13/06/2014
* \brief   Various conversion tools.
*/

#pragma once

#include <iostream>
#include <list>
#include <string>
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

class Conversion {

    public :

        /**
        * Get type of opencv mat object.
        *
        * @param type mat.type()
        * @return Type in string.
        */
        static string matTypeToString(int type);

        /**
        * Convert an int value to string.
        *
        * @param nb Integer value.
        * @return String value.
        */
        static string intToString(int nb);

        /**
        * Convert a float value to string.
        *
        * @param nb Float value.
        * @return String value.
        */
        static string floatToString(float nb);

        static string doubleToString(double nb);

        /**
        * Extract data from a string according to a delimiter.
        *
        * @param container List of extracted data.
        * @param in String to analyse.
        * @param delimiters
        */
        static void stringTok(list<string>  &container, string const &in, const char * const delimiters);

        /**
        * Convert an opencv image to 8 bits.
        *
        * @param img Opencv image to convert.
        * @return 8 bits opencv mat.
        */
        static Mat convertTo8UC1(Mat &img);

        /**
        * Determine the number of "0" required.
        *
        * @param totalDigit Maximum number of digits.
        * @param n Integer value.
        * @return Number of 0 to add to reach maximum of available digits.
        */
        static string numbering(int totalDigit, int n);

        /**
        * Count number of digit in a value.
        *
        * @param n Integer value.
        * @return Number of digits.
        */
        static int countNumberDigit(int n);

        /**
        * Round an integer to upper range.
        *
        * @param n Integer value. Example : 103, 1025.
        * @return Rounded value. Example : 200, 2000.
        */
        static int roundToUpperRange(int n);

        static float roundToNearest(float value, float precision);

};

