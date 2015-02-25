/*
								Conversion.cpp

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
* \file    Conversion.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    13/06/2014
* \brief   Various conversion tools.
*/

#include "Conversion.h"

Conversion::Conversion(){
    //ctor
}

string Conversion::matTypeToString(int type) {

  string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {

    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;

  }

  r += "C";
  r += (chans+'0');

  return r;

}

string Conversion::intToString(int nb){

	ostringstream oss;
	oss << nb;
	string result = oss.str();
	return result;

}

double Conversion::roundToNearest(double num){

    return (num > 0.0) ? floor(num + 0.5) : ceil(num - 0.5);

}

//string with delimiters -> list of string
template <typename Container> void Conversion::stringTok(Container &container, string const &in, const char * const delimiters  = "_"){

    const string::size_type len = in.length();
    string::size_type i = 0;

    while (i < len){

        // Eat leading whitespace
        i = in.find_first_not_of(delimiters, i);

        if (i == string::npos)
            return;   // Nothing left but white space

        // Find the end of the token
        string::size_type j = in.find_first_of(delimiters, i);

        // Push token
        if (j == string::npos){

            container.push_back(in.substr(i));
            return;

        }else

            container.push_back(in.substr(i, j-i));

        // Set up for next loop
        i = j + 1;

    }
}

Mat Conversion::convertTo8UC1(Mat &img){

    Mat tmp;
    img.copyTo(tmp);
    double min, max;
    minMaxLoc(tmp, &min, &max);
    tmp.convertTo(tmp, CV_8UC1, 255.0/(max - min));

    return tmp;

}

string Conversion::numbering(int totalDigit, int n){

    int cpt = 0;

    int nbZeroToAdd = 0;

    string ch = "";

    if(n<10){

        nbZeroToAdd = totalDigit - 1;

        for(int i = 0; i < nbZeroToAdd; i++){

            ch += "0";

        }

    }else{

        while(n > 0){

            n/=10;
            cpt ++;

        }

        nbZeroToAdd = totalDigit - cpt;

        for(int i = 0; i < nbZeroToAdd; i++){

            ch += "0";

        }

    }

    return ch;
}

void Conversion::intBitDepth_To_CamBitDepth(int acqFormat, CamBitDepth &camFormat){

    switch(acqFormat){

        case 8 :

            {
                camFormat = MONO_8;

            }

            break;

        case 12 :

            {
                camFormat = MONO_12;
            }

            break;

        default :

            throw "> The specified bitdepth is not allowed.";

            break;

    }

}
