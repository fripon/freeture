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
    posFailed       = 0;
    posSuccess      = 0;

    frameDownSampled = downsample;

    dateEvent       = date;
    nbPt = 4;
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

int GlobalEvent::getPosFailed(){

    return posFailed;

}

int GlobalEvent::getPosSuccess(){

    return posSuccess;

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

    //Add LE
    listLocalEvent.push_back(le);

    if(listLocalEvent.size() == 0){

        mainPoints.push_back(center);
        circle(dirMap, center, 5, Scalar(0,255,0), CV_FILLED, 8, 0);

    }else if((listLocalEvent.size())%nbPt == 0){

        mainPoints.push_back(center);

        if(mainPoints.size() >= 3){

            Point   A   = Point(mainPoints.at(mainPoints.size()- 3)),
                    B   = Point(mainPoints.at(mainPoints.size()- 2)),
                    C   = Point(mainPoints.at(mainPoints.size()- 1));

            Point   v1  = Point(B.x - A.x, B.y - A.y);

            Point   B2  = Point(B.x + v1.x, B.y + v1.y);

            Point   v2  = Point(C.x - B.x, C.y - B.y);
            Point   v3  = Point(B2.x - B.x, B2.y - B.y);

            float thetaRad = (v3.x*v2.x+v3.y*v2.y)/(sqrt(pow(v3.x,2)+pow(v3.y,2))*sqrt(pow(v2.x,2)+pow(v2.y,2)));
            float thetaDeg = (180 * acos(thetaRad))/3.14159265358979323846;

            if(thetaDeg > 35.0 || thetaDeg < -35.0 ){

                circle(dirMap, center, 5, Scalar(0,0,255), CV_FILLED, 8, 0);
                posFailed++;

                return false;

            }else{
                posSuccess++;
                circle(dirMap, center, 5, Scalar(0,255,0), CV_FILLED, 8, 0);

            }
        }
    }

    circle(dirMap, center, 3, Scalar(255,0,0), CV_FILLED, 8, 0);

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
