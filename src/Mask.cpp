/*
                                Mask.cpp

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
* \file    Mask.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    26/11/2014
*/

#include "Mask.h"

Mask::Mask(int timeInterval, bool customMask, string customMaskPath, bool downsampleMask, CamPixFmt format, bool updateMask):
mUpdateInterval(timeInterval), mUpdateMask(updateMask) {

    mMaskToCreate = false;
    updateStatus = false;
    refDate = to_simple_string(boost::posix_time::second_clock::universal_time());
    satMap = boost::circular_buffer<Mat>(2);

    // Load a mask from file.
    if(customMask) {

        mOriginalMask = imread(customMaskPath, CV_LOAD_IMAGE_GRAYSCALE);
        SaveImg::saveJPEG(mOriginalMask, "/home/fripon/mOriginalMask");
        if(!mOriginalMask.data)
            throw "Fail to load the mask from its path.";

        if(downsampleMask)
            pyrDown(mOriginalMask, mOriginalMask, Size(mOriginalMask.cols/2, mOriginalMask.rows/2));

        mOriginalMask.copyTo(mCurrentMask);

    }else{

        mMaskToCreate = true;

    }

    // Estimate saturated value.
    switch(format) {

        case MONO12 :

                saturatedValue = 4092;

            break;

        default :

                saturatedValue = 254;

    }

}

bool Mask::applyMask(Mat &currFrame) {

    if(mMaskToCreate) {

        mOriginalMask = Mat(currFrame.rows, currFrame.cols, CV_8UC1,Scalar(255));
        mOriginalMask.copyTo(mCurrentMask);
        mMaskToCreate = false;

    }

    if(mUpdateMask) {

        if(updateStatus) {

            if(mCurrentMask.rows != currFrame.rows || mCurrentMask.cols != currFrame.cols) {

                throw "Mask's size is not correct according to frame's size.";

            }

            // Reference date.
            refDate = to_simple_string(boost::posix_time::second_clock::universal_time());

            Mat saturateMap = ImgProcessing::buildSaturatedMap(currFrame, saturatedValue);

            // Dilatation of the saturated map.
            int dilation_size = 10;
            Mat element = getStructuringElement(MORPH_RECT, Size(2*dilation_size + 1, 2*dilation_size+1), Point(dilation_size, dilation_size));
            cv::dilate(saturateMap, saturateMap, element);

            satMap.push_back(saturateMap);

            if(satMap.size() == 2) {

                Mat temp = satMap.front() & satMap.back();
                bitwise_not(temp,temp);
                temp.copyTo(mCurrentMask, mOriginalMask);

            }

            Mat temp; currFrame.copyTo(temp, mCurrentMask);
            temp.copyTo(currFrame);

            updateStatus = false;
            return true; // Mask not applied, only computed.

        }

        string nowDate = to_simple_string(boost::posix_time::second_clock::universal_time());
        boost::posix_time::ptime t1(boost::posix_time::time_from_string(refDate));
        boost::posix_time::ptime t2(boost::posix_time::time_from_string(nowDate));
        boost::posix_time::time_duration td = t2 - t1;
        long diffTime = td.total_seconds();

        if(diffTime >= mUpdateInterval) {

            updateStatus = true;

        }

        cout << "NEXT MASK : " << (mUpdateInterval - (int)diffTime) << "s" << endl;

    }

    if(!mCurrentMask.data || (mCurrentMask.rows != currFrame.rows && mCurrentMask.cols != currFrame.cols)) {
        mMaskToCreate = true;
        return true;
    }

    Mat temp; currFrame.copyTo(temp, mCurrentMask);
    temp.copyTo(currFrame);

    return false; // Mask applied.

}

void Mask::resetMask() {

    mOriginalMask.copyTo(mCurrentMask);
    updateStatus = true;
    satMap.clear();

}


