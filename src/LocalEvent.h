/*
                                LocalEvent.h

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
* \file    LocalEvent.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Event occured on a single frame.
*/

#pragma once

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include "Circle.h"
#include "TimeDate.h"

using namespace cv;
using namespace std;

class LocalEvent {

    private :

        Scalar          mLeColor;           // Color attribute of the local event.
        Mat             mLeMap;             // ROI map.
        Point           mLeMassCenter;      // Mass center.
        int             mLeNumFrame;        // Associated frame.
        
        Point           mPosMassCenter;
        Point           mNegMassCenter;
        float           mPosRadius;
        float           mNegRadius;
        bool            mPosCluster;
        bool            mNegCluster;
        bool            mergedFlag;
        Point           uNegToPos;
        int             index;

    public :

        vector<Point>   mLeRoiList;   // Contains position of region of interest which compose a local event.
        vector<Point>   mAbsPos;
        vector<Point>   mPosPos;
        vector<Point>   mNegPos;
        int             mFrameHeight;
        int             mFrameWidth;
        TimeDate::Date  mFrameAcqDate;

        /**
        * Constructor.
        *
        * @param color
        * @param roiPos Position of the ROI.
        * @param frameHeight
        * @param frameWidth
        * @param roiSize
        */
        LocalEvent(Scalar color, Point roiPos, int frameHeight, int frameWidth, const int *roiSize);

        /**
        * Destructor.
        *
        */
        ~LocalEvent();

        /**
        * Compute mass center of the local event.
        *
        */
        void computeMassCenter();

        /**
        * Get local event's color.
        *
        * @return color.
        */
        Scalar getColor() {return mLeColor;};

        /**
        * Get local event's color.
        *
        * @return color.
        */
        Mat getMap() {return mLeMap;};

        /**
        * Get local event's mass center.
        *
        * @return Center of mass.
        */
        Point getMassCenter() {return mLeMassCenter;};

        /**
        * Get local event's frame number.
        *
        * @return Frame number.
        */
        int getNumFrame() {return mLeNumFrame;};

        /**
        * Set local event's frame number.
        *
        * @param n Frame number.
        */
        void setNumFrame(int n) {mLeNumFrame = n;};

        /**
        * Update local event's roi map.
        *
        * @param p Position of the new ROI.
        * @param h Height of the new ROI.
        * @param w Width of the new ROI.
        */
        void setMap(Point p, int h, int w);

        Point getLeDir() {return uNegToPos;};

        void addAbs(vector<Point> p);
        void addPos(vector<Point> p);
        void addNeg(vector<Point> p);

        bool getMergedStatus() {return mergedFlag;};
        void setMergedStatus(bool flag) {mergedFlag = flag;};

        Mat createPosNegAbsMap();
        bool localEventIsValid();

        bool getPosClusterStatus() {return mPosCluster;};
        bool getNegClusterStatus() {return mNegCluster;};

        void mergeWithAnOtherLE(LocalEvent &LE);
        void completeGapWithRoi(Point p1, Point p2);
        Point getPosMassCenter() {return mPosMassCenter;};
        Point getNegMassCenter() {return mNegMassCenter;};
        float getPosRadius() {return mPosRadius;};
        float getNegRadius() {return mNegRadius;};
        bool getPosCluster() {return mPosCluster;};
        bool getNegCluster() {return mNegCluster;};
        bool getMergedFlag() {return mergedFlag;};
        int getLeIndex() {return index;};
        void setLeIndex(int i) {index = i;};


};
