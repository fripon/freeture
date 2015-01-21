/*
								GlobalEvent.cpp

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
 * @file    GlobalEvent.cpp
 * @author  Yoan Audureau
 * @version 1.0
 * @date    03/06/2014
*/

#include "GlobalEvent.h"

GlobalEvent::GlobalEvent(vector<string> frameDate, int frameNum, int frameHeight, int frameWidth){

    geAge           = 0;
    geAgeLastLE     = 0;
    geDate          = frameDate;
    numFirstFrame   = frameNum;
    numLastFrame   = frameNum;
    newLEAdded      = false;
    geMap           = Mat(frameHeight,frameWidth, CV_8UC1,Scalar(0));
    dirMap          = Mat(frameHeight,frameWidth, CV_8UC3,Scalar(0,0,0));
    linear          = true;
    velocity        = 0;
    badPoint        = 0;
    goodPoint       = 0;

}

GlobalEvent::~GlobalEvent(){

}

bool GlobalEvent::addLE(LocalEvent le){

    Point center =  le.getMassCenter();

    LEList.push_back(le);

    if(LEList.size() % 2 == 0){

        mainPoints.push_back(center);

        if(mainPoints.size() >= 3){

            Point   A   = Point(mainPoints.at(0)),
                    B   = Point(mainPoints.at(floor(mainPoints.size()/2.0))),
                    C   = Point(mainPoints.at(mainPoints.size()- 1));

            Point   v1  = Point(B.x - A.x, B.y - A.y);

            Point   B2  = Point(B.x + v1.x, B.y + v1.y);

            Point   v2  = Point(C.x - B.x, C.y - B.y);
            Point   v3  = Point(B2.x - B.x, B2.y - B.y);

            float thetaRad = (v3.x*v2.x+v3.y*v2.y)/(sqrt(pow(v3.x,2)+pow(v3.y,2))*sqrt(pow(v2.x,2)+pow(v2.y,2)));
            float thetaDeg = (180 * acos(thetaRad))/3.14159265358979323846;

            if(thetaDeg > 35.0 || thetaDeg < -35.0 ){

                circle(dirMap, center, 5, Scalar(0,0,255), CV_FILLED, 8, 0);
                badPoint++;
                if(pos.back() == false)
                    linear = false;

                pos.push_back(false);

                return false;

            }else{

                pos.push_back(true);
                circle(dirMap, center, 5, Scalar(0,255,0), CV_FILLED, 8, 0);
                goodPoint++;
                velocity = sqrt(pow(LEList.front().getMassCenter().x - LEList.back().getMassCenter().x,2)+pow(LEList.front().getMassCenter().y - LEList.back().getMassCenter().y,2));

            }
        }
    }



    circle(dirMap, center, 3, Scalar(255,0,0), CV_FILLED, 8, 0);

    //reset ageLastELem
    geAgeLastLE = 0;

    //Update event's map
    Mat res = geMap + le.getMap();
    res.copyTo(geMap);

    return true;

}

bool GlobalEvent::continuousGoodPos(int n){

    int nb = 0;
    int nn = 0;

    cout << "start"<<endl;
    for(int i = 0; i < pos.size(); i++){

        if(pos.at(i)){

            nb++;

            if(nb > n)
                break;

        }else{

            nn++;

            if(nn == 3)
                break;

        }

    }
    cout << "end"<<endl;

    if(nb >= n) return true;
    else return false;

}

