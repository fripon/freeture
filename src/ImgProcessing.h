/*
                              ImgProcessing.h

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
* \file    ImgProcessing.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
*/

#pragma once

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <iostream>

using namespace std;
using namespace cv;

class ImgProcessing {

    public :

        /**
        * Gamma correction on Mono8 image..
        *
        * @param img Opencv mat image to correct.
        * @param gamma Gamma value.
        * @return Image with gamma corrected.
        */
        static Mat correctGammaOnMono8(Mat& img, double gamma);

        /**
        * Gamma correction on Mono12 image.
        *
        * @param img Opencv mat image to correct.
        * @param gamma Gamma value.
        * @return Image with gamma corrected.
        */
        static Mat correctGammaOnMono12(Mat& img, double gamma);


        static Mat subdivideFrame(Mat img, int n) {

            vector<Point> listSubPos;

            int subW = img.cols/n;
            int subH = img.rows/n;

            cout << "subW : " << subW << endl;
            cout << "subH : " << subH << endl;

            for(int j = 0; j < n; j++) {

                for(int i = 0; i < n; i++) {

                    listSubPos.push_back(Point(i*subW, j*subH));
                   // cout << Point(i*subW, j*subH)<< endl;

                }

            }

            Mat imgSubdivided;
            img.copyTo(imgSubdivided);

            for(int i = 0; i < n; i++)
                line(imgSubdivided, Point(i * subW, 0), Point(i*subW, subH * n), Scalar(255), 1, 8);

            for(int j = 0; j < n; j++)
                line(imgSubdivided, Point(0, j * subH), Point(subW * n, j * subH), Scalar(255), 1, 8);

            return imgSubdivided;

        };


};
