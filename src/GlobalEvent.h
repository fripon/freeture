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
 * @file    GlobalEvent.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    10/11/2014
 */

#pragma once

#include "includes.h"
#include "Frame.h"
#include "LocalEvent.h"
#include "RecEvent.h"
#include "SaveImg.h"

using namespace cv;
using namespace std;

class GlobalEvent{

    private:

        //! Age in frames since its creation
        int age;

        //! Age in frames since 0 LocalEvent have been added
        int ageLastElem;

        //! Creation date
        vector<string> dateEvent;

        //! Local events
        vector<LocalEvent> listLocalEvent;

        //! Event frames with a LE
        vector<int> numEvFrameWithLE;

        //! Event map
        Mat mapEvent;

        //! Number of points to average to get a position
        int nbPt; // 3

        //! Averages position of the event
        vector<Point> avPos;

        //! Frames before the event
        vector<Mat> eventPrevBuffer;

        //! Frames of the event
        vector<Mat> eventBuffer;

        //! Frames after the event
        vector<Mat> eventAfterBuffer;

        vector<Point> mainPoints;

        int posFailed;
        int posSuccess;

        //! Event direction mask
        Mat dirMask;

        Mat dirMap;

        bool LELinked;

        bool frameDownSampled;



    public:
int cptt;
        GlobalEvent(/*LocalEvent f, Mat mapp,*/ vector<string> date,int imgH, int imgW, bool downsample);

        ~GlobalEvent();

        RecEvent extractEventRecInfos();

        int getAge();
        int getAgeLastElem();
        vector<string> getDate();
        vector<LocalEvent> *getListLocalEvent();
        Mat getMapEvent();
        bool getLELinked();
        vector<Mat> getEvPrevBuffer();
        vector<Mat> getEvBuffer();
        vector<Mat> getEvAfterBuffer();
        vector<Point> getAvgPos();
        int getPosFailed();
        int getPosSuccess();
        Mat getDirMap();

        void setAge(int a);
        void setAgeLastElem(int a);
        void setMapEvent(Mat m);
        void setLELinked(bool state);
        void setEvPrevBuffer(Mat f);
        void setEvBuffer(Mat f);
        void setEvAfterBuffer(Mat f);

        bool addLE(LocalEvent le);

};
