/*
								Frame.cpp

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
* \file    Frame.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    19/06/2014
* \brief   Frame grabbed from a camera or other input video source.
*/

#include "Frame.h"

Frame::Frame(Mat capImg, int g, int e, string acquisitionDate){

	exp         = e;
	gain        = g;
    fileName    = "";
    rawDate     = acquisitionDate;
    capImg.copyTo(img);
    frameRemaining = 0;
    dateSeconds = 0.0;

    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(":");
    tokenizer tokens(acquisitionDate, sep);
    for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
        dateString.push_back(*tok_iter);
    }

    acqDate = dateString.at(0)+"-"+dateString.at(1)+"-"+dateString.at(2)+"T"+dateString.at(3)+":"+dateString.at(4)+":"+dateString.at(5);

    //dateString = dateValues;

    for(int i = 0 ; i< dateString.size();i++){
       date.push_back(atoi(dateString.at(i).c_str()));
    }

    // Find saturated value.
    double minVal, maxVal;
    minMaxLoc(capImg, &minVal, &maxVal);
    saturatedValue = maxVal;

}

Frame::Frame(){

    exp         = 0;
	gain        = 0;

    fileName    = "";
    acqDate     = "";
    frameRemaining = 0;

}

Frame::~Frame(void){

}

void Frame::setAcqDateMicro(string date){

    acqDateInMicrosec = date;
    vector<string> temp;
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(":");
    tokenizer tokens(date, sep);
    for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
        temp.push_back(*tok_iter);
    }

    dateSeconds = atof(temp.back().c_str());
}

