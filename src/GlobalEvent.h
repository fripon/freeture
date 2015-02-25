/*
				GlobalEvent.h

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
*	Last modified:		10/11/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    GlobalEvent.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   A Detected event occured on different consecutives frames in time.
*/

#pragma once

#include <math.h>
#include <vector>

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>

#include "Frame.h"
#include "LocalEvent.h"
#include "SaveImg.h"

using namespace cv;
using namespace std;

class GlobalEvent{

    private:

        int             geAge;
        int             geAgeLastLE;
        vector<string>  geDate;
        Mat             geMap;
        int             numFirstFrame;
        int             numLastFrame;

        Mat             dirMap;
        Mat             dirMap2;
        float           velocity;
        bool            newLEAdded;
        bool            linear;
        int             badPoint;
        int             goodPoint;
        Mat             geMapPrev;
        bool            geStatic;
        bool            checkPos;
        int nbCheckConsecutivePosError;
        Point           lastPos;

    public:

        vector<LocalEvent> LEList;
        vector<Mat> eventBuffer;
        vector<bool> pos;
        vector<float> dist;
        vector<Point>   mainPoints;

        GlobalEvent(vector<string> frameDate, int frameNum, int frameHeight, int frameWidth);

        ~GlobalEvent();

        Mat             getMapEvent             ()          {return geMap;};
        Mat             getDirMap               ()          {return dirMap;};
        Mat             getDirMap2              ()          {return dirMap2;};
        int             getAge                  ()          {return geAge;};
        int             getAgeLastElem          ()          {return geAgeLastLE;};
        vector<string>  getDate                 ()          {return geDate;};
        bool            getLinearStatus         ()          {return linear;};
        bool            getGeStatic             ()          {return geStatic;};
        float           getVelocity             ()          {return velocity;};
        bool            getNewLEStatus          ()          {return newLEAdded;};
        int             getBadPos               ()          {return badPoint;};
        int             getGoodPos              ()          {return goodPoint;};
        int             getNumFirstFrame        ()          {return numFirstFrame;};
        int             getNumLastFrame         ()          {return numLastFrame;};

        void            setAge                  (int a)     {geAge = a;};
        void            setAgeLastElem          (int a)     {geAgeLastLE = a;};
        void            setMapEvent             (Mat m)     {m.copyTo(geMap);};
        void            setNewLEStatus          (bool s)    {newLEAdded = s;};
        void            setNumFirstFrame        (int n)     {numFirstFrame = n;};
        void            setNumLastFrame         (int n)     {numLastFrame = n;};

        bool addLE(LocalEvent le);
        bool continuousGoodPos(int n);

};
