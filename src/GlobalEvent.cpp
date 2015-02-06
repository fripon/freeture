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
 * \file    GlobalEvent.cpp
 * \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * \version 1.0
 * \date    03/06/2014
 * \brief   A Detected event occured on different consecutives frames in time.
 */

#include "GlobalEvent.h"

GlobalEvent::GlobalEvent(vector<string> frameDate, int frameNum, int frameHeight, int frameWidth){

    geAge           = 0;
    geAgeLastLE     = 0;
    geDate          = frameDate;
    numFirstFrame   = frameNum;
    numLastFrame    = frameNum;
    newLEAdded      = false;
    geMap           = Mat(frameHeight,frameWidth, CV_8UC1,Scalar(0));
    geMapPrev       = Mat(frameHeight,frameWidth, CV_8UC1,Scalar(0));
    dirMap          = Mat(frameHeight,frameWidth, CV_8UC3,Scalar(0,0,0));
    dirMap2         = Mat(960,1280, CV_8UC3,Scalar(0,0,0));
    linear          = true;
    velocity        = 0;
    badPoint        = 0;
    goodPoint       = 0;
    geStatic        = false;
    checkPos        = false;
    nbCheckConsecutivePosError = 0;

}

GlobalEvent::~GlobalEvent(){

}

bool GlobalEvent::addLE(LocalEvent le){

    // Get LE position.
    Point center = le.getMassCenter();

    // Add the LE to the current GE.
    LEList.push_back(le);

    // First position of the GE.
    if(LEList.size() == 1){

        Point mapCenter = Point(640,480);
        lastPos = Point(640,480);

        putText(dirMap2, "0", Point(mapCenter.x,mapCenter.y + 7 ),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(255,255,255), 1, CV_AA);
        circle(dirMap2, mapCenter, 5, Scalar(255,255,255), 1, 8, 0);
        mainPoints.push_back(center);
        dist.push_back(0.0);

    }else if(LEList.size() % 3 == 0){

        // Shifted value.
        Point diff = Point(center.x - mainPoints.back().x, center.y - mainPoints.back().y) * 5;
        // New Position in dirMap system.
        Point newPos = Point(lastPos.x + diff.x, lastPos.y + diff.y);
        // Draw line.
        line(dirMap2, lastPos, newPos, Scalar( 0, 0, 255 ), 2, 8);
        // Update.
        lastPos = newPos;

        putText(dirMap2, Conversion::intToString(mainPoints.size()), Point(newPos.x,newPos.y + 7 ),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(255,255,255), 1, CV_AA);

        float d = sqrt(pow(center.x - mainPoints.back().x,2) + pow(center.y - mainPoints.back().y,2));
        dist.push_back(d);

        if(d == 0) geStatic = true;

        mainPoints.push_back(center);

        if(mainPoints.size() >= 3){

            Point   A   = Point(mainPoints.front()),
                    B   = Point(mainPoints.at(mainPoints.size()- 2)),
                    C   = Point(mainPoints.back());

            Point   v1  = Point(B.x - A.x, B.y - A.y);

            Point   B2  = Point(B.x + v1.x, B.y + v1.y);

            Point   v2  = Point(C.x - B.x, C.y - B.y);
            Point   v3  = Point(B2.x - B.x, B2.y - B.y);

            float thetaRad = (v3.x*v2.x+v3.y*v2.y)/(sqrt(pow(v3.x,2)+pow(v3.y,2))*sqrt(pow(v2.x,2)+pow(v2.y,2)));
            float thetaDeg = (180 * acos(thetaRad))/3.14159265358979323846;

            if(thetaDeg > 40.0 || thetaDeg < -40.0 ){

                circle(dirMap, center, 5, Scalar(0,0,255), 1, 8, 0);
                circle(dirMap2, newPos, 5, Scalar(0,0,255), 1, 8, 0);

                badPoint++;

                // Two consecutives bad points.
                if(pos.back() == false) linear = false;

                pos.push_back(false);

                return false;

            }else{

                pos.push_back(true);
                circle(dirMap, center, 5, Scalar(0,255,0), 1, 8, 0);
                circle(dirMap2, newPos, 5, Scalar(0,255,0), 1, 8, 0);
                goodPoint++;

            }

        }else{

            circle(dirMap2, newPos, 5, Scalar(0,255,0), 1, 8, 0);
        }
    }

    circle(dirMap, center, 3, Scalar(255,0,0), 1, 8, 0);

    //reset ageLastELem
    geAgeLastLE = 0;

    //Update event's map
    Mat res = geMap + le.getMap();
    res.copyTo(geMap);

    if(LEList.size() == 1){

        res.copyTo(geMapPrev);

    }

    /*if(LEList.size() % 20 == 0){

        Mat res2 = geMapPrev & le.getMap();
        int nbPixNonZero = countNonZero(res2);

        if( nbPixNonZero > 0 ){

            geStatic = true;

        }

        Mat temp = le.getMap();
        temp.copyTo(geMapPrev);

    }*/

    return true;

}

bool GlobalEvent::continuousGoodPos(int n){

    int nb = 0;
    int nn = 0;

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

    if(nb >= n) return true;
    else return false;

}

