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
 * @file    Frame.cpp
 * @author  Yoan Audureau
 * @version 1.0
 * @date    19/06/2014
 */

#include "Frame.h"

Frame::Frame(Mat capImg, int g, int e, string acquisitionDate){

	exp         = e;
	gain        = g;
    pathOnDisk  = "";
    fileName    = "";
    rawDate     = acquisitionDate;
    capImg.copyTo(img);
    frameRemaining = 0;

	//acqDate = nowtime.localFormattedDatetime(second_clock::universal_time(),"%Y-%m-%dT %H:%M:%S");

    //vector <string> dateValues;

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


}

Frame::Frame(vector<int>  acquisitionDate, int g, int e){

    exp         = e;
	gain        = g;
	pathOnDisk  = "";
	fileName    = "";
	frameRemaining = 0;

	/*vector <string> dateValues;

    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(":");
    tokenizer tokens(acquisitionDate, sep);

    for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
        dateValues.push_back(*tok_iter);
    }

    for(int i = 0 ; i< dateValues.size();i++){
       date.push_back(atoi(dateValues.at(i).c_str()));
    }*/

    date = acquisitionDate;

    //FORMAT -> %Y-%m-%dT%H:%M:%S
    acqDate = Conversion::intToString(acquisitionDate.at(0))+"-"+
    Conversion::intToString(acquisitionDate.at(1))+"-"+
    Conversion::intToString(acquisitionDate.at(2))+"T"+
    Conversion::intToString(acquisitionDate.at(3))+":"+Conversion::intToString(acquisitionDate.at(4))+":"+Conversion::intToString(acquisitionDate.at(5));

    threadReadingStatus["img"] = false;
    threadReadingStatus["ast"] = false;
    threadReadingStatus["det"] = false;

}

Frame::Frame(string  acquisitionDate, int g, int e){

    exp         = e;
	gain        = g;
	pathOnDisk  = "";
	fileName    = "";

	vector <string> dateValues;

    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(":");
    tokenizer tokens(acquisitionDate, sep);

    for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
        dateValues.push_back(*tok_iter);
    }

    for(int i = 0 ; i< dateValues.size();i++){
       date.push_back(atoi(dateValues.at(i).c_str()));
    }

   // date = acquisitionDate;

    //FORMAT -> %Y-%m-%dT%H:%M:%S
    acqDate = dateValues.at(0)+"-"+dateValues.at(1)+"-"+dateValues.at(2)+"T"+dateValues.at(3)+":"+dateValues.at(4)+":"+dateValues.at(5);

    threadReadingStatus["img"] = false;
    threadReadingStatus["ast"] = false;
    threadReadingStatus["det"] = false;
    frameRemaining = 0;

}

Frame::Frame(){

    exp         = 0;
	gain        = 0;
    pathOnDisk  = "";
    fileName    = "";
    acqDate     = "";
    frameRemaining = 0;

}

Frame::~Frame(void){

}

bool Frame::copyFrame(Frame*& frameToCopy, Mat mask){

    exp         = frameToCopy->getExposure();
	gain        = frameToCopy->getGain();
    pathOnDisk  = frameToCopy->getPath();
    fileName    = frameToCopy->getFileName();
    acqDate     = frameToCopy->getAcqDate();
    date        = frameToCopy->getDate();
    dateString  = frameToCopy->getDateString();

    frameToCopy->getImg().copyTo(img,mask);

    return true;

}

bool Frame::copyFrame(Frame*& frameToCopy){

    exp         = frameToCopy->getExposure();
	gain        = frameToCopy->getGain();
    pathOnDisk  = frameToCopy->getPath();
    fileName    = frameToCopy->getFileName();
    acqDate     = frameToCopy->getAcqDate();
    date        = frameToCopy->getDate();
    dateString  = frameToCopy->getDateString();

    frameToCopy->getImg().copyTo(img);

    frameRemaining = 0;
    frameNumber = 0;

    return true;

}

int Frame::getNumFrame(){

	return frameNumber;

}

void Frame::setNumFrame(int n){

	frameNumber = n;

}


int Frame::getFrameRemaining(){

    return frameRemaining;

}

void Frame::setFrameRemaining(int val){

    frameRemaining = val;

}

string  Frame::getRawDate() {return rawDate;}
void    Frame::setRawDate(string d) {rawDate = d;}



Mat Frame::getImg(){

	return img;

}

void Frame::setImg(Mat i){

	i.copyTo(img);

}


void    Frame::useMask(Mat mask){

    Mat temp;
    img.copyTo(temp, mask);
    temp.copyTo(img);

}



int Frame::getExposure(){

    return exp;

}

int Frame::getGain(){

    return gain;

}

void Frame::setExposure(int val){

    exp = val;

}

void Frame::setGain(int val){

   gain = val;

}

string Frame::getAcqDate (){

    return acqDate;

}

void Frame::setAcqDate (string val){

    acqDate = val;

}

vector <int> Frame::getDate(){

    return date;

}

vector <string> Frame::getDateString(){

    return dateString;

}

void Frame::setDate(vector <int>  val){

    date = val;

}

void Frame::setPath(string  p){

    pathOnDisk = p;

}

string Frame::getPath(){

    return pathOnDisk;

}

string Frame::getFileName(){

    return fileName;

}

void Frame::setFileName(string val){

    fileName = val;

}

void Frame::setThreadReadingStatus(string threadName, bool status){

    if(threadReadingStatus.find(threadName) != threadReadingStatus.end()){

        threadReadingStatus.at(threadName) = status;

    }

}

bool Frame::getThreadReadingStatus (string threadName){

    if(threadReadingStatus.find(threadName) != threadReadingStatus.end()){

        return threadReadingStatus.at(threadName);

    }

    return false;

}
