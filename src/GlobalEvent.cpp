/*
                            GlobalEvent.cpp

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
*   Last modified:      20/07/2015
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

GlobalEvent::GlobalEvent(TimeDate::Date frameDate, int frameNum, int frameHeight, int frameWidth, Scalar c){

    geAge               = 0;
    geAgeLastLE         = 0;
    geDate              = frameDate;
    geFirstFrameNum     = frameNum;
    geLastFrameNum      = frameNum;
    newLeAdded          = false;
    geMap               = Mat(frameHeight,frameWidth, CV_8UC1, Scalar(0));
    geMapColor          = Mat(frameHeight,frameWidth, CV_8UC3, Scalar(0,0,0));
    geDirMap            = Mat(frameHeight,frameWidth, CV_8UC3, Scalar(0,0,0));
    geLinear            = true;
    geBadPoint          = 0;
    geGoodPoint         = 0;
    geShifting          = 0;
    geColor             = c;
    geDir               = Point(0,0);

}

GlobalEvent::~GlobalEvent(){

}

bool GlobalEvent::addLE(LocalEvent le){

    // Get LE position.
    Point center = Point(le.getMassCenter().x, le.getMassCenter().y);

    // Indicates if the le in input can be added to the global event.
    bool addLeDecision = true;

    // First LE's position become a main point.
    if(pts.size()==0){

        mainPts.push_back(center);
        geGoodPoint++;
        ptsValidity.push_back(true);

    }

    // If the current le is at least the second.
    else if(pts.size()>0){

        //float d = sqrt(pow(pt.back().x - pt.at(pt.size()-2).x,2.0) + pow(pt.back().y - pt.at(pt.size()-2).y,2.0));

        if(listv.size()>1 ) {

            float scalar = le.getLeDir().x * listv.back().x + le.getLeDir().y * listv.back().y;

            leDir = le.getLeDir();

            if(scalar <= 0.0) clusterNegPos.push_back(false);
            else clusterNegPos.push_back(true);

        }


        // Check global event direction each 3 new local event.
        if((pts.size()+1)%3 == 0){

            // If there is already at least two main points.
            if(mainPts.size()>=2){

                // Get first main point.
                Point A = mainPts.front();
                listA.push_back(A);
                // Get last main point.
                Point B = mainPts.back();
                listB.push_back(B);
                // Get current le position.
                Point C = center;
                listC.push_back(C);
                // Vector from first main point to last main point.
                Point u  = Point(B.x - A.x, B.y - A.y);
                listu.push_back(u);



                // Vector from last main point to current le position.
                Point v  = Point(C.x - B.x, C.y - B.y);
                listv.push_back(v);
                geDir = v;

                // Same mainPts position : No Displacement
                if((v.x == 0 && v.y == 0) || (u.x == 0 && u.y == 0)){

                    listRad.push_back(0);
                    listAngle.push_back(0);
                    mainPtsValidity.push_back(false);
                    addLeDecision = false;
                    ptsValidity.push_back(false);

                    /*geBadPoint = 0;
                    geGoodPoint++;
                    mainPts.push_back(center);
                    ptsValidity.push_back(true);
                    mainPtsValidity.push_back(true);
                    circle(geDirMap, center, 5, Scalar(255,255,255), 1, 8, 0);*/

                }else{

                    // Birds filter
                    /*float scalar = le.getLeDir().x * v.x + le.getLeDir().y * v.y;

                    if(scalar <= 0.0) clusterNegPos.push_back(false);
                    else clusterNegPos.push_back(true);
                    */

                    // Norm vector u
                    float normU = sqrt(pow(u.x,2.0)+pow(u.y,2.0));
                    // Norm vector v
                    float normV = sqrt(pow(v.x,2.0)+pow(v.y,2.0));
                    // Compute angle between u and v.
                    float thetaRad = (u.x*v.x+u.y*v.y)/(normU*normV);
                    listRad.push_back(thetaRad);
                    float thetaDeg = (180 * acos(thetaRad))/3.14159265358979323846;
                    listAngle.push_back(thetaDeg);

                    if(thetaDeg > 40.0 || thetaDeg < -40.0 ){

                        geBadPoint++;

                        if(geBadPoint == 2){

                            geLinear = false;

                        }

                        addLeDecision = false;
                        ptsValidity.push_back(false);
                        mainPtsValidity.push_back(false);
                        circle(geDirMap, center, 5, Scalar(0,0,255), 1, 8, 0);

                    }else{

                        geBadPoint = 0;
                        geGoodPoint++;
                        mainPts.push_back(center);
                        ptsValidity.push_back(true);
                        mainPtsValidity.push_back(true);
                        circle(geDirMap, center, 5, Scalar(255,255,255), 1, 8, 0);

                    }

                }

            }else{

                // Create new main point.
                mainPts.push_back(center);
                geGoodPoint++;
                ptsValidity.push_back(true);
                circle(geDirMap, center, 5, Scalar(255,255,255), 1, 8, 0);

            }
        }
    }

    // Add the le in input to the current ge.
    if(addLeDecision){

        // Save center of mass.
        pts.push_back(center);

        // Add the LE to the current GE.
        LEList.push_back(le);

        // Reset age without any new le.
        geAgeLastLE = 0;

        // Update ge map.
        Mat res = geMap + le.getMap();
        res.copyTo(geMap);

        // Update colored ge map.
        vector<Point>::iterator it;
        int roiH = 10, roiW = 10;
        for(it = le.mLeRoiList.begin(); it != le.mLeRoiList.end(); ++it){

            Mat roi(roiH,roiW,CV_8UC3,geColor);
            roi.copyTo(geMapColor(Rect((*it).x-roiW/2,(*it).y-roiH/2,roiW,roiH)));

        }

        // Update dirMap
        geDirMap.at<Vec3b>(center.y,center.x) = Vec3b(0,255,0);

    }else{

        // Update dirMap
        geDirMap.at<Vec3b>(center.y,center.x) = Vec3b(0,0,255);

    }

    return true;

}

bool GlobalEvent::continuousGoodPos(int n, string &msg){

    msg += "continuousGoodPos\n";
    int nb = 0;
    int nn = 0;

    msg += "size pts validity : " + Conversion::intToString(ptsValidity.size()) + " \n";

    for(int i = 0; i < ptsValidity.size(); i++){

        if(ptsValidity.at(i)){

            nb++;
            nn=0;

            if(nb >= n) {
                msg += "continuousGoodPos " + Conversion::intToString(n) + " = OK\n";
                return true;
            }

        }else{

            nn++;
            nb=0;

            if(nn == 2) {
                msg += "continuousGoodPos " + Conversion::intToString(n) + " = NOT OK\n";
                return false;
            }

        }

    }

    return false;

}

bool GlobalEvent::continuousBadPos(int n){

    int nb = 0;
    int nn = 0;

    for(int i = 0; i < ptsValidity.size(); i++){

        if(!ptsValidity.at(i)){

            nb++;
            nn=0;

            if(nb >= n)
                return true;

        }else{

            nn++;
            nb=0;

            if(nn == 2)
                return false;

        }

    }

    return false;

}

bool GlobalEvent::negPosClusterFilter(string &msg) {

    msg += "negPosClusterFilter\n";
    float counter = 0;
    msg += "clusterNegPos size = " +Conversion::intToString(clusterNegPos.size())+ " \n";
    for(int i = 0; i < clusterNegPos.size(); i++) {

        if(clusterNegPos.at(i)) {
            msg += "clusterNegPos true\n";
            counter+=1.0;
        }else{
            msg += "clusterNegPos false\n";

        }

    }

    if(counter >= (float)clusterNegPos.size()/2.0 && counter !=0) {
        msg += "negPosClusterFilter = OK\n";
        return true;
    }else {
        msg += "negPosClusterFilter = NOT OK\n";
        return false;
    }
}

bool GlobalEvent::ratioFramesDist(string &msg){

    msg += "ratioFramesDist\n";
    float d = sqrt(pow(mainPts.back().x - mainPts.front().x,2.0) + pow(mainPts.back().y - mainPts.front().y,2.0));
    msg += "d = "  +Conversion::floatToString(d)+   " \n";
    int n = geLastFrameNum - geFirstFrameNum;
    msg += "n = "+Conversion::intToString(n)+ " \n";

    if(d > (n *0.333)){
        msg += "ratio = ok\n";
        return true;
    }else{
        msg += "ratio = not ok\n";
        return false;
    }


}

