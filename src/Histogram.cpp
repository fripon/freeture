/*
								Histogram.cpp

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
 * @file    Histogram.cpp
 * @author  Yoan Audureau
 * @version 1.0
 * @date    03/06/2014
 * @section DESCRIPTION
 *
 * The detection class contains all meteor detection methods
 */

#include "Histogram.h"

Histogram::Histogram(Mat image, Mat mask){

        double minVal, maxVal;
        minMaxLoc(image, &minVal, &maxVal);

        histSize = 256;

        cout <<"histSize: "<<histSize<<endl;

        float range[] = { 0, 256 } ;
        const float* histRange = { range };

        bool uniform = true, accumulate = false;

        calcHist( &image, 1, 0, mask, hist, 1, &histSize, &histRange, uniform, accumulate );

}


Histogram::Histogram(Mat image, double maxVal){

        // Establish the number of bins
        histSize = (int)maxVal;

        float range[] = { 0, (float)maxVal } ;
        const float* histRange = { range };

        bool uniform = true, accumulate = false;

        calcHist( &image, 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate );

}

Histogram::~Histogram(){

}
/*
void Histogram::printHist8UCImage(){

    /// Load image
    Mat src = imread( "/home/fripon/Histogram_Calculation_Original_Image.jpg", 1 );

    cvtColor(src, src, CV_RGB2GRAY);

    if( !src.data )
    { return -1; }

    /// Separate the image in 3 places ( B, G and R )
    //vector<Mat> bgr_planes;
    //split( src, bgr_planes );

    /// Establish the number of bins
    int histSize = 256;

    /// Set the ranges ( for B,G,R) )
    float range[] = { 0, 256 } ;
    const float* histRange = { range };

    bool uniform = true; bool accumulate = false;

    Mat hist;

    calcHist( &src, 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate );
    // Draw the histograms for B, G and R
    int hist_w = 1280;
    int hist_h = 960;
    int bin_w = cvRound( (double) hist_w/histSize );

    Mat histImage( hist_h, hist_w, CV_8UC1, Scalar( 0) );

    /// Normalize the result to [ 0, histImage.rows ]
    normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

    /// Draw
    for( int i = 1; i < histSize; i++ )
    {
      line( histImage, Point( bin_w*(i-1), hist_h - cvRound(hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(hist.at<float>(i)) ),
                       Scalar( 255, 0, 0), 2, 8, 0  );

    }

    /// Display
    namedWindow("calcHist Demo", CV_WINDOW_AUTOSIZE );
    imshow("calcHist Demo", histImage );

    waitKey(0);


}*/

Mat Histogram::drawHist(){

    double hist_w = 1280;
    double hist_h = 960;
    double bin_w  = hist_w/histSize ;

    Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

    // Normalize the result to [ 0, histImage.rows ]
    Mat histNormalized;
    hist.copyTo(histNormalized);

    normalize(hist, histNormalized, 0, histImage.rows, NORM_MINMAX, -1, Mat() );



    // find the maximum intensity element from histogram
    int maxi =0;
    for(int i = 0; i < histSize; i++){
        if(maxi < (int)hist.at<float>(i)){
            maxi = (int)hist.at<float>(i);
        }
    }

    cout <<"maxi value in hist : "<<maxi<<endl;

    // Draw
    for( int i = 1; i < histSize; i++ ){

      line( histImage, Point( bin_w*(double)(i-1), hist_h - (double)histNormalized.at<float>(i-1) ),
                       Point( bin_w*((double)i), hist_h - (double)histNormalized.at<float>(i) ),
                       Scalar( 0,0,255), 2, 8, 0  );



    }

    return histImage;

}

void Histogram::printHist(){

    // Show the calculated histogram in console
    for( int h = 0; h < histSize; h++ ){

        cout<<"|"<<hist.at<float>(h);

     }

     cout<<"|";

}

int Histogram::highLimitValue(float percentLimit, int nbTotalPixel){

    float threshold = (percentLimit * (float)nbTotalPixel) / 100.0;
    float nbPix = 0;
    int newMaxValue = histSize;

    for(int i = 0; i < histSize; i++){

        nbPix = nbPix + hist.at<float>(i);

        if(nbPix > threshold){

            newMaxValue = i;
            break;

        }
    }

    return newMaxValue;
}

vector<float> Histogram::checkSkyInRange(int firstLimit, int secondLimit, float percentLimit, int nbTotalPixel){

    float threshold = (percentLimit * (float)nbTotalPixel) / 100.0;

    float nbPix1 = 0;
    float nbPix2 = 0;
    float nbPix3 = 0;

    float percent1 = 0;
    float percent2 = 0;
    float percent3 = 0;

    int returnValue = 0;

    for(int i = 0; i < firstLimit; i++){

        nbPix1 = nbPix1 + hist.at<float>(i);

    }

    percent1 = (nbPix1 * 100.0) / (float)nbTotalPixel;

    for(int i = firstLimit+1; i < secondLimit; i++){

        nbPix2 = nbPix2 + hist.at<float>(i);

    }

    percent2 = (nbPix2 * 100.0) / (float)nbTotalPixel;


    for(int i = secondLimit+1; i < histSize; i++){

        nbPix3 = nbPix3 + hist.at<float>(i);

    }

    percent3 = (nbPix3 * 100.0) / (float)nbTotalPixel;


    vector<float> res;

    res.push_back(percent1);
    res.push_back(percent2);
    res.push_back(percent3);

    return res;

}

double Histogram::computeMSV(){

    double rangeSize = histSize/5;

    double range0_start = 0.0;
    double range0_end   = rangeSize;

    double range1_start = rangeSize;
    double range1_end   = 2*rangeSize;

    double range2_start = 2*rangeSize;
    double range2_end   = 3*rangeSize;

    double range3_start = 3*rangeSize;
    double range3_end   = 4*rangeSize;

    double range4_start = 4*rangeSize;
    double range4_end   = histSize;

    double range5       = histSize;

    double x0 = 0, x1 = 0, x2 = 0, x3 = 0, x4 = 0, x5 = 0;

    int pxTotal = 0;

    for(int i = 0; i < histSize; i++){

        if(i<=range0_end){

            x0 = x0 + hist.at<float>(i);

        }else if((i>range1_start)&&(i<=range1_end)){

            x1 = x1 + hist.at<float>(i);

        }else if((i>range2_start)&&(i<=range2_end)){

            x2 = x2 + hist.at<float>(i);

        }else if((i>range3_start)&&(i<=range3_end)){

            x3 = x3 + hist.at<float>(i);

        }else if(i>range4_start){

            x4 = x4 + hist.at<float>(i);

        }

        pxTotal = pxTotal + (int)hist.at<float>(i);
    }

    double msv = ((x0 + 2*x1 + 3*x2 + 4*x3 + 5*x4 )/(x0+x1+x2+x3+x4));

  /*  cout <<"stats : 0->"<< (x0*100)/pxTotal
        <<"% 1->"<< (x1*100)/pxTotal
        <<"% 2->"<< (x2*100)/pxTotal
        <<"% 3->"<< (x3*100)/pxTotal
        <<"% 4->"<< (x4*100)/pxTotal<<"%"<<endl;

    cout <<"pxTotal"<<pxTotal<<endl;
    cout <<"pxDernier : "<<x4<<endl;*/

    return msv;

}






