/*
								GlobalEvent.cpp

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
*	Last modified:		20/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
 * @file    GlobalEvent.cpp
 * @author  Yoan Audureau
 * @version 1.0
 * @date    03/06/2014
*/

#include "GlobalEvent.h"

GlobalEvent::GlobalEvent(vector<string> date, int imgH, int imgW, bool downsample){

    age             = 0;
    ageLastElem     = 0;

    frameDownSampled = downsample;

    dateEvent       = date;
    nbPt = 3;
    LELinked = false;

    dirMask = Mat(imgH,imgW, CV_8UC1,Scalar(255));
    mapEvent = Mat(imgH,imgW, CV_8UC1,Scalar(0));

    if(downsample)
        dirMap = Mat(imgH*2,imgW*2, CV_8UC3,Scalar(0,0,0));
    else
        dirMap = Mat(imgH,imgW, CV_8UC3,Scalar(0,0,0));

    cptt = 0;

}

GlobalEvent::~GlobalEvent(){

    //dtor
}

/*void GlobalEvent::incrementLocalEventsPosition(){

    vector<LocalEvent>::iterator it;

    for (it=listLocalEvent.begin(); it!=listLocalEvent.end(); ++it){

       // cout << "Pos before : "<< (*it).getFramePosition() <<endl;
        (*it).setFramePosition((*it).getFramePosition()+1);
       // cout << "Pos after : "<< (*it).getFramePosition() <<endl;

    }
}*/

RecEvent GlobalEvent::extractEventRecInfos(){

    RecEvent rec;

    vector<Point>   eventXYPosition;
    vector<int>     eventBufferPosition;

    vector<LocalEvent>::iterator it;

    for (it=listLocalEvent.begin(); it!=listLocalEvent.end(); ++it){

        eventXYPosition.push_back((*it).centerOfMass);
        eventBufferPosition.push_back((*it).getFramePosition());

    }

    rec.setPrevBuffer(eventPrevBuffer);
    rec.setBuffer(eventBuffer);                        // Copy the event's buffer
    rec.setListMetPos(eventXYPosition);             // XY positions of the event in the frame
    rec.setPositionInBuffer(eventBufferPosition);   // Frame numbers of event positions
    rec.setMapEvent(mapEvent);                      // GEMAP
    rec.setDateEvent(dateEvent);                    // Date of the first event's frame
    rec.setDirMap(dirMap);

    return rec;

}

vector<LocalEvent> * GlobalEvent::getListLocalEvent(){

    vector<LocalEvent> *liste = &listLocalEvent;

    return liste;

}

Mat GlobalEvent::getMapEvent(){

    return mapEvent;

}

void GlobalEvent::setMapEvent(Mat m){

    m.copyTo(mapEvent);

}

int GlobalEvent::getAge(){

    return age;

}

int GlobalEvent::getAgeLastElem(){

    return ageLastElem;

}

vector<string> GlobalEvent::getDate(){

    return dateEvent;

}

void GlobalEvent::setAge(int a){

    age = a;

}

void GlobalEvent::setAgeLastElem(int a){

    ageLastElem = a;

}

bool GlobalEvent::getLELinked(){

    return LELinked;

}

void GlobalEvent::setLELinked(bool state){

    LELinked = state;

}

bool GlobalEvent::addLE(LocalEvent le){

    Point center =  le.centerOfMass;

    if(frameDownSampled){
        center.x = center.x*2;
        center.y = center.y*2;
    }

    if((listLocalEvent.size() + 1)%nbPt == 0){

        float sumX = 0, sumY =0, avgX = 0, avgY = 0;

        for(int i = 0; i<nbPt-1; i++){

            sumX+= listLocalEvent.at(listLocalEvent.size()- nbPt+1).centerOfMass.x;
            sumY+= listLocalEvent.at(listLocalEvent.size()- nbPt+1).centerOfMass.y;

        }

        sumX+= le.centerOfMass.x;
        sumY+= le.centerOfMass.y;

        avgX = sumX/nbPt;
        avgY = sumY/nbPt;

        avPos.push_back(Point(avgX, avgY));

        // Check linearity
        if((avPos.size()+1)>=2){

            Point A1 = avPos.at(avPos.size() - 2);
            Point B1 = avPos.at(avPos.size() - 1);
            Point C1 = le.centerOfMass;

            Point v1 = Point(B1.x - A1.x, B1.y - A1.y );

            Point A2 = B1;
            Point B2 = Point(B1.x + v1.x, B1.y + v1.y);

            Point v2 = Point(C1.x - A2.x, C1.y - A2.y );
            Point v3 = Point(B2.x - A2.x, B2.y - A2.y );

            float thetaRad = (v3.x*v2.x+v3.y*v2.y)/(sqrt(pow(v3.x,2)+pow(v3.y,2))*sqrt(pow(v2.x,2)+pow(v2.y,2)));

            float thetaDeg = (180 * acos(thetaRad))/3.14159265358979323846;

            if(thetaDeg > 45.0 || thetaDeg < -45.0 ){

                circle(dirMap, center, 5, Scalar(0,0,255), CV_FILLED, 8, 0);

                return false;

            }else{

                circle(dirMap, center, 5, Scalar(0,255,0), CV_FILLED, 8, 0);

            }
        }
    }

    circle(dirMap, center, 1, Scalar(255,0,0), 2, 8, 0);

    listLocalEvent.push_back(le);

    cout << "list lE : " << listLocalEvent.size()<<endl;

    //reset ageLastELem
    ageLastElem = 0;

    //Update event's map
    Mat res = mapEvent + le.getMap();
    res.copyTo(mapEvent);
    cptt++;
    // SaveImg::saveBMP(le.getMap(),"/home/fripon/data2/mapEvent_" + Conversion::intToString(cptt) );

    return true;

}

Mat GlobalEvent::getDirMap(){

    return dirMap;

}

vector<Mat> GlobalEvent::getEvPrevBuffer(){

    return eventPrevBuffer;

}

vector<Mat> GlobalEvent::getEvBuffer(){

    return eventBuffer;

}

vector<Mat> GlobalEvent::getEvAfterBuffer(){

    return eventAfterBuffer;

}

void GlobalEvent::setEvPrevBuffer(Mat f){

    eventPrevBuffer.push_back(f);

}

void GlobalEvent::setEvBuffer(Mat f){

     eventBuffer.push_back(f);

}

void GlobalEvent::setEvAfterBuffer(Mat f){

     eventAfterBuffer.push_back(f);

}

vector<Point> GlobalEvent::getAvgPos(){

    return avPos;

}
