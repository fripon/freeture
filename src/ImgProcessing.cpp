/*
                            ImgProcessing.cpp

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
* \file    ImgProcessing.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
*/

#include "ImgProcessing.h"

Mat ImgProcessing::correctGammaOnMono8(Mat& img, double gamma) {

    double gammaInverse = 1.0 / gamma;

    Mat lutMatrix(1, 256, CV_8UC1 );
    uchar * ptr = NULL;
    ptr = lutMatrix.ptr();
    for( int i = 0; i < 256; i++ )
    ptr[i] = (int)( pow( (double) i / 255.0, gammaInverse ) * 255.0 );

    Mat result;
    LUT( img, lutMatrix, result );

    return result;

}

Mat ImgProcessing::correctGammaOnMono12(Mat& img, double gamma) {

    double gammaInverse = 1.0 / gamma;

    Mat result = Mat(img.rows, img.cols, CV_16UC1, Scalar(0));

    unsigned short * ptr = NULL;
    unsigned short * ptr2 = NULL;

    for(int i = 0; i < img.rows; i++){

        ptr = img.ptr<unsigned short>(i);
        ptr2 = result.ptr<unsigned short>(i);

        for(int j = 0; j < img.cols; j++){

            ptr2[j] = (int)( pow( (double) ptr[j] / 4095.0, gammaInverse ) * 4095.0 );

        }

    }

    return result;

}

Mat ImgProcessing::buildSaturatedMap(Mat &img, int maxval) {

    Mat saturatedMap = Mat::zeros(img.rows, img.cols, CV_8UC1);

    unsigned short * ptr1;
    unsigned char * ptr2;

    for(int i = 0; i < img.rows; i++) {

        ptr1 = img.ptr<unsigned short>(i);
        ptr2 = saturatedMap.ptr<unsigned char>(i);

        for(int j = 0; j < img.cols; j++){

            if(ptr1[j] >= maxval) {
                ptr2[j] = 255;
            }
        }
    }

    return saturatedMap;

}

Mat ImgProcessing::thresholding(Mat &img, Mat &mask, int factor, Thresh threshType) {

    Mat thresholdedMap = Mat(img.rows,img.cols, CV_8UC1,Scalar(0));
    Scalar mean, stddev;
    cv::meanStdDev(img, mean, stddev, mask);
    int threshold = 0;

    switch(threshType) {

        case Thresh::MEAN :

            threshold = mean[0] * factor;

            break;

        case Thresh::STDEV :

            threshold = stddev[0] * factor;

            break;

    }

    switch(img.type()) {

        case CV_16UC1 :
            {
                if(threshold == 0)
                    threshold = 65535;

                unsigned short * ptr1;
                unsigned char * ptrMap;

                for(int i = 0; i < img.rows; i++) {

                    ptr1 = img.ptr<unsigned short>(i);
                    ptrMap = thresholdedMap.ptr<unsigned char>(i);

                    for(int j = 0; j < img.cols; j++){

                        if(ptr1[j] > threshold) {
                            ptrMap[j] = 255;
                        }
                    }
                }
            }

            break;

        case CV_8UC1 :

            {
                if(threshold == 0)
                    threshold = 255;

                unsigned char * ptr1;
                unsigned char * ptrMap;

                for(int i = 0; i < img.rows; i++) {

                    ptr1 = img.ptr<unsigned char>(i);
                    ptrMap = thresholdedMap.ptr<unsigned char>(i);

                    for(int j = 0; j < img.cols; j++){

                        if(ptr1[j] > threshold) {
                            ptrMap[j] = 255;
                        }
                    }
                }
            }
            break;

    }

    return thresholdedMap;

}
