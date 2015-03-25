/*
								LocalEvent.h

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
* \file    LocalEvent.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Event occured on a single frame.
*/

#pragma once

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

class LocalEvent{

    private:

        Scalar  leColor;
        Mat     leMap;
        Point   leMassCenter;
        int     leNumFrame;

    public:

        // Contains position of region of interest which compose a local event.
        vector<Point> leRoiList;

        vector<Mat> subdivision;

        LocalEvent(Scalar color, Point roiPos, int frameHeight, int frameWidth, const int *roiSize);

        LocalEvent(Scalar color, Point sPos, Mat s);

        ~LocalEvent();

        void computeMassCenter();

        Scalar  getColor()          {return leColor;};
        Mat     getMap()            {return leMap;};
        Point   getMassCenter()     {return leMassCenter;};
        int     getNumFrame()       {return leNumFrame;};
        void    setNumFrame(int n)  {leNumFrame = n;};

        void    setMap(Point p, int h, int w);

};
