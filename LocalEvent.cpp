/*
                            LocalEvent.cpp

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
* \file    LocalEvent.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Event occured on a single frame.
*/

#include "LocalEvent.h"

LocalEvent::LocalEvent( Scalar color, Point roiPos, int frameHeight, int frameWidth, const int *roiSize):
                        mLeColor(color), mFrameHeight(frameHeight), mFrameWidth(frameWidth), mLeNumFrame(0), index(0) {

    mPosMassCenter = Point(0,0);
    mNegMassCenter = Point(0,0);
    mPosRadius = 0.5;
    mNegRadius = 0.5;
    mPosCluster = false;
    mNegCluster = false;
    mergedFlag = false;
    uNegToPos = Point(0,0);

    // Save position of the first ROI.
    mLeRoiList.push_back(roiPos);

    // Create LE map.
    mLeMap = Mat::zeros(frameHeight, frameWidth, CV_8UC1);

    // Add first ROI in the LE map.
    Mat roi(roiSize[1],roiSize[0],CV_8UC1,Scalar(255));
    roi.copyTo(mLeMap(Rect(roiPos.x-roiSize[0]/2,roiPos.y-roiSize[1]/2,roiSize[0],roiSize[1])));

}

LocalEvent::~LocalEvent() {

}

void LocalEvent::computeMassCenter() {

    float x = 0, y = 0;

    vector<Point>::iterator it;

    for(it = mAbsPos.begin(); it != mAbsPos.end(); ++it){

        x += (*it).x;
        y += (*it).y;

    }

    x = x / mAbsPos.size();
    y = y / mAbsPos.size();

    mLeMassCenter = Point(x,y);

}

void LocalEvent::setMap(Point p, int h, int w){

    // Add new ROI to the LE map.
    Mat roi(h,w,CV_8UC1,Scalar(255));
    roi.copyTo(mLeMap(Rect(p.x, p.y, w, h)));

}

void LocalEvent::addAbs(vector<Point> p) {

    mAbsPos.insert(mAbsPos.end(), p.begin(), p.end());

}

void LocalEvent::addPos(vector<Point> p) {

    mPosPos.insert(mPosPos.end(), p.begin(), p.end());
    if(mPosPos.size()!=0) mPosCluster = true;

}

void LocalEvent::addNeg(vector<Point> p) {

    mNegPos.insert(mNegPos.end(), p.begin(), p.end());
    if(mNegPos.size()!=0) mNegCluster = true;

}

bool LocalEvent::localEventIsValid() {

    bool posCluster = false, negCluster = false;

    vector<Point>::iterator it;

    // Positive cluster.
    if(mPosPos.size() != 0) {

        float xPos = 0.0, yPos = 0.0;

        for(it = mPosPos.begin(); it != mPosPos.end(); ++it) {

            xPos += (*it).x;
            yPos += (*it).y;

        }

        xPos = xPos / mPosPos.size();
        yPos = yPos / mPosPos.size();

        mPosMassCenter = Point(xPos, yPos);

        // Search radius.
        for(it = mPosPos.begin(); it != mPosPos.end(); ++it) {

            float radius = sqrt(pow(((*it).x - mPosMassCenter.x),2) + pow(((*it).y - mPosMassCenter.y),2));

            if(radius > mPosRadius) {

                mPosRadius = radius;

            }
        }

        if(mPosRadius > 0.0) posCluster = true;

    }

    // Negative cluster.
    if(mNegPos.size() != 0) {

        float xNeg = 0.0, yNeg = 0.0;

        for(it = mNegPos.begin(); it != mNegPos.end(); ++it){

            xNeg += (*it).x;
            yNeg += (*it).y;

        }

        xNeg = xNeg / mNegPos.size();
        yNeg = yNeg / mNegPos.size();

        mNegMassCenter = Point(xNeg, yNeg);

        // Search radius.
        for(it = mNegPos.begin(); it != mNegPos.end(); ++it){

            float radius = sqrt(pow(((*it).x - mNegMassCenter.x),2) + pow(((*it).y - mNegMassCenter.y),2));

            if(radius > mNegRadius) {

                mNegRadius = radius;

            }
        }

        if(mNegRadius > 0.0) negCluster = true;

    }

    // Check intersection between clusters.
    if(negCluster && posCluster) {

        // Vector from neg cluster to pos cluster.
        uNegToPos = Point(mPosMassCenter.x - mNegMassCenter.x, mPosMassCenter.y - mNegMassCenter.y);


        Circle pos(mPosMassCenter, mPosRadius);
        Circle neg(mNegMassCenter, mNegRadius);

        float surfaceCircle1 = 0.0, surfaceCircle2 = 0.0, intersectedSurface = 0.0;

        // Intersection ?
        bool res = pos.computeDiskSurfaceIntersection(neg,surfaceCircle1, surfaceCircle2, intersectedSurface, false, "");

        if(!res) {

            // Le is valid.
            return true;

        }else {

            if( surfaceCircle1 != 0 &&
                intersectedSurface != 0 &&
                surfaceCircle2 != 0 ) {

                // One of the two circles is intersected more than 50% of its surface.
                if((intersectedSurface * 100)/surfaceCircle1 > 50 || (intersectedSurface * 100)/surfaceCircle2 > 50) {

                    return false; // Le is not valid.

                } else {

                    // Le is valid.
                    return true;

                }

            }else {

                return false; // Le is not valid.

            }

        }

    }

    return true;

}

Mat LocalEvent::createPosNegAbsMap() {

    Mat map = Mat::zeros(mFrameHeight, mFrameWidth, CV_8UC3);

    vector<Point>::iterator it;

    for(it = mAbsPos.begin(); it != mAbsPos.end(); ++it){

        map.at<Vec3b>((*it).y, (*it).x)[0] = 255;
        map.at<Vec3b>((*it).y, (*it).x)[1] = 255;
        map.at<Vec3b>((*it).y, (*it).x)[2] = 255;

    }

    if(mPosPos.size() != 0) {
        float xPos = 0.0, yPos = 0.0;

        for(it = mPosPos.begin(); it != mPosPos.end(); ++it){

            map.at<Vec3b>((*it).y, (*it).x)[0] = 0;
            map.at<Vec3b>((*it).y, (*it).x)[1] = 255;
            map.at<Vec3b>((*it).y, (*it).x)[2] = 0;

            xPos += (*it).x;
            yPos += (*it).y;

        }

        xPos = xPos / mPosPos.size();
        yPos = yPos / mPosPos.size();

        Point mPosMassCenter = Point(xPos, yPos);

        // Search radius.
        float posRadius = 0.0;

        for(it = mPosPos.begin(); it != mPosPos.end(); ++it){

            float radius = sqrt(pow(((*it).x - mPosMassCenter.x),2) + pow(((*it).y - mPosMassCenter.y),2));

            if(radius > posRadius) {

                posRadius = radius;

            }

        }

        if(mPosMassCenter.x > 0 && mPosMassCenter.y > 0)
            circle(map, mPosMassCenter, (int) posRadius, Scalar(0,255,0));

    }

    if(mNegPos.size() != 0) {

        float xNeg = 0.0, yNeg = 0.0;

        for(it = mNegPos.begin(); it != mNegPos.end(); ++it){

            if(map.at<Vec3b>((*it).y, (*it).x) == Vec3b(0,255,0)) {

                map.at<Vec3b>((*it).y, (*it).x)[0] = 255;
                map.at<Vec3b>((*it).y, (*it).x)[1] = 0;
                map.at<Vec3b>((*it).y, (*it).x)[2] = 0;

            }else {

                map.at<Vec3b>((*it).y, (*it).x)[0] = 0;
                map.at<Vec3b>((*it).y, (*it).x)[1] = 0;
                map.at<Vec3b>((*it).y, (*it).x)[2] = 255;



            }

            xNeg += (*it).x;
                yNeg += (*it).y;
        }

        xNeg = xNeg / mNegPos.size();
        yNeg = yNeg / mNegPos.size();

        Point mNegMassCenter = Point(xNeg, yNeg);

        // Search radius.
        float negRadius = 0.0;

        for(it = mNegPos.begin(); it != mNegPos.end(); ++it){

            float radius = sqrt(pow(((*it).x - mNegMassCenter.x),2) + pow(((*it).y - mNegMassCenter.y),2));

            if(radius > negRadius) {

                negRadius = radius;

            }

        }

        if(mNegMassCenter.x > 0 && mNegMassCenter.y > 0)
            circle(map, mNegMassCenter, (int) negRadius, Scalar(0,0,255));

    }

    return map;

}

void LocalEvent::mergeWithAnOtherLE(LocalEvent &LE) {

    mLeRoiList.insert(mLeRoiList.end(), LE.mLeRoiList.begin(), LE.mLeRoiList.end());

    float dist = sqrt(pow((mLeMassCenter.x - LE.getMassCenter().x),2) + pow((mLeMassCenter.y - LE.getMassCenter().y),2));
    completeGapWithRoi(mLeMassCenter,LE.getMassCenter());
    mAbsPos.insert(mAbsPos.end(), LE.mAbsPos.begin(), LE.mAbsPos.end());
    mPosPos.insert(mPosPos.end(), LE.mPosPos.begin(), LE.mPosPos.end());
    mNegPos.insert(mNegPos.end(), LE.mNegPos.begin(), LE.mNegPos.end());
    computeMassCenter();
    Mat temp = mLeMap + LE.getMap();
    temp.copyTo(mLeMap);
    if(mPosPos.size()!=0) mPosCluster = true;
    if(mNegPos.size()!=0) mNegCluster = true;

}

//http://mathforum.org/library/drmath/view/66794.html
void LocalEvent::completeGapWithRoi(Point p1, Point p2) {

    Mat roi(10,10,CV_8UC1,Scalar(255));

    float dist = sqrt(pow((p1.x - p2.x),2) + pow((p1.y - p2.y),2));

    float part = dist / 10.0;

    if((int)part!=0) {

        Point p3 = Point(p1.x,p2.y);

        float dist1 = sqrt(pow((p1.x - p3.x),2) + pow((p1.y - p3.y),2)); //A--> C
        float dist2 = sqrt(pow((p2.x - p3.x),2) + pow((p2.y - p3.y),2)); //B-> C

        float part1 = dist1 / part;
        float part2 = dist2 / part;

        for(int i = 0; i < part ; i++){

            Point p = Point(p3.x + i * part2, p1.y + i* part1);

            if(p.x-5 > 0 && p.x+5 < mLeMap.cols && p.y-5 > 0 && p.y+5 < mLeMap.rows)
                roi.copyTo(mLeMap(Rect(p.x-5,p.y-5,10,10)));

        }
    }

}
