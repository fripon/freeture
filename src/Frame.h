/*
                                    Frame.h

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
* \file    Frame.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    19/06/2014
* \brief   Image container.
*/

#pragma once

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <boost/tokenizer.hpp>
#include "Conversion.h"
#include "SaveImg.h"
#include "ECamPixFmt.h"
#include "TimeDate.h"

using namespace std;
using namespace cv;

class Frame {

    public :

        TimeDate::Date      mDate;               // Acquisition date.
        double              mExposure;           // Camera's exposure value used to grab the frame.
        int                 mGain;               // Camera's gain value used to grab the frame.
        CamPixFmt           mFormat;             // Pixel format.
        Mat                 mImg;                // Frame's image data.
        string              mFileName;           // Frame's name.
        int                 mFrameNumber;        // Each frame is identified by a number corresponding to the acquisition order.
        int                 mFrameRemaining;     // Define the number of remaining frames if the input source is a video or a set of single frames.
        double              mSaturatedValue;     // Max pixel value in the image.
        int                 mFps;                // Camera's fps.
        int                 mWidth;
        int                 mHeight;

        Frame(Mat capImg, int g, double e, string acquisitionDate);

        Frame();

        ~Frame();

};
