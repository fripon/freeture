/*
				Conversion.h

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
*	Last modified:		20/10/2014
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

#include "ECamBitDepth.h"

using namespace std;
using namespace cv;

class Conversion{

    public:

        Conversion();

        static string matTypeToString(int type);

        static string intToString(int nb);

        static string floatToString(float nb);

        static void stringTok(list<string>  &container, string const &in, const char * const delimiters);

        static double roundToNearest(double num);

        static Mat convertTo8UC1(Mat &img);

        static string numbering(int totalDigit, int n);

		static void intBitDepth_To_CamBitDepth(int acqFormat, CamBitDepth &camFormat);

		static int countNumberDigit(int n);

		// Examples :
		// 153 -> 200
		// 103 -> 200
		// 1025 -> 2000
		// 4095 -> 5000
		// 64000 -> 70000
		static int roundToUpperRange(int n);

};

