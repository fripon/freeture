/*
								ImgReduction.cpp

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
*	Last modified:		12/12/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    ImgReduction.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    12/12/2014
* \brief   Dynamic reduction of an image.
*/

#include "ImgReduction.h"

ImgReduction::ImgReduction(){
    //ctor
}

Mat ImgReduction::dynamicReductionBasedOnHistogram(double percent, Mat& img ){

	Scalar imgMean;
    Scalar imgStddev;
    meanStdDev(img, imgMean, imgStddev);

	double minVal, maxVal;
    minMaxLoc(img, &minVal, &maxVal);

	Mat mask = Mat(img.rows,img.cols,CV_8UC1,Scalar(255));
	Mat copyImg;
	img.copyTo(copyImg);
	copyImg.convertTo(copyImg, CV_32FC1, 1.0/maxVal);

	cout << "> High   : " << img.rows 					<< endl;
    cout << "> Width  : " << img.cols 					<< endl;
    cout << "> Pixel  : " << img.rows * img.cols 		<< endl;
	cout << "> Mean   : " << (cvRound(imgMean.val[0])) 	<< endl;
    cout << "> Stddev : " << (cvRound(imgStddev.val[0]))<< endl;
	cout << "> Max    : " << maxVal						<< endl;
    cout << "> Min    : " << minVal						<< endl << endl;

	// Count saturated pixels and create a mask to locate them to avoid to use them in the
	// histogram contruction.

	int nb_sat = 0;

    float * ptr;
    unsigned char * ptr2;

    for(int i = 0; i < copyImg.rows; i++){

        ptr = copyImg.ptr<float>(i);
        ptr2 = mask.ptr<unsigned char>(i);

        for(int j = 0; j < copyImg.cols; j++){

             if( ptr[j] >= 0.99){

                nb_sat++;
                ptr2[j] = 0;

             }
        }
    }

    cout << "> Sat pix : "<< nb_sat<<endl<<endl;

	// Compute histogram

	/// Establish the number of bins
    int histSize = 700000;

    /// Set the ranges ( for B,G,R) )
    float range[] = { 0.0f, 1.0f} ;
    const float* histRange = { range };

    bool uniform = true; bool accumulate = false;

    Mat hist;

    cout << "Bins : " << histSize << endl;
	cout << "Bins size : " << 1.0f/histSize << endl;
	cout << "Bins size : " << maxVal/histSize << endl<<endl;
    calcHist( &copyImg, 1, 0, mask, hist, 1, &histSize, &histRange, uniform, accumulate );

	int f = cvRound( (double) ((100 - percent) * (copyImg.rows * copyImg.cols  - nb_sat ))/100 );
    cout << "> Rm pix : "<< f<<endl;

	int newMaxValue = 0 ;
    int a = 0;
    for( int i = histSize - 1; i >=0; i-- ){

        if(a<f)
            a += hist.at<float>(i);
        else {

            cout << "> Bin found :  " << i << endl;
			newMaxValue = cvRound( (double) maxVal/histSize) * i + cvRound( (double) (maxVal/histSize));
            cout << "> Value : " << newMaxValue<< endl;

            break;
        }
       // cout << "bin " << i<< ": " << hist.at<float>(i)<< endl;

    }

    cout << "> Pix sum rm : " << a << endl;

	//t = (((double)getTickCount() - t )/getTickFrequency())*1000;

	//cout << "Time to find new max value  : " << t << endl<<endl;

	Mat newMat(copyImg.rows,copyImg.cols, CV_16SC1, Scalar(0));

	float* ptr0;
    short *ptr1;

	int max16 = 32767/*65535/**/, min16 = -32768/**/;

	float Bzero, Bscale;

	Bscale = (newMaxValue - minVal)/(max16 - min16);

	cout << "BSCALE : " << Bscale<< endl;

	Bzero = (max16 * minVal - min16 * newMaxValue)/(max16 - min16);

	cout << "BZERO : " << Bzero<< endl;

	for(int i = 0; i < img.rows; i++){

        ptr0 = img.ptr<float>(i);
        ptr1 = newMat.ptr<short>(i);

        for(int j = 0; j < img.cols; j++){

			if(ptr0[j] > newMaxValue)

				ptr1[j] = (newMaxValue - Bzero ) / Bscale;

			else

            	//ptr1[j] = (ptr0[j] - ((minVal*65535 - (0) * newMaxValue)/(65535-(0))))/ ((newMaxValue-minVal)/(65535-0));

            	ptr1[j] = (ptr0[j] - Bzero ) / Bscale;

        }
    }

    return newMat;

}

Mat ImgReduction::dynamicReductionByFactorDivision(Mat& img, CamBitDepth bitpix, int imgToSum, float &bzero, float &bscale){

    Mat newMat;

    float factor;

    switch(bitpix){

        case MONO_8 :

            {

                newMat = Mat(img.rows,img.cols, CV_8SC1, Scalar(0));
                factor = imgToSum;
                bscale = factor;
                bzero  = 128 * factor;

                float * ptr;
                char * ptr2;

                for(int i = 0; i < img.rows; i++){

                    ptr = img.ptr<float>(i);
                    ptr2 = newMat.ptr<char>(i);

                    for(int j = 0; j < img.cols; j++){

                        if(cvRound(ptr[j] / factor) - 128 > 255){

                            ptr2[j] = 255;

                        }else{

                            ptr2[j] = cvRound(ptr[j] / factor) - 128;
                        }

                    }
                }
            }

            break;

        case MONO_12 :

            {

                newMat = Mat(img.rows,img.cols, CV_16SC1, Scalar(0));
                factor = (4095.0f * imgToSum)/65535;

                bscale = factor;
                bzero  = 32768 * factor;

                float * ptr;
                short * ptr2;

                for(int i = 0; i < img.rows; i++){

                    ptr = img.ptr<float>(i);
                    ptr2 = newMat.ptr<short>(i);

                    for(int j = 0; j < img.cols; j++){

                        if(cvRound(ptr[j] / factor) - 32768 > 32767){

                            ptr2[j] = 32767;

                        }else{

                            ptr2[j] = cvRound(ptr[j] / factor) - 32768;
                        }
                    }
                }
            }

            break;

    }

    return newMat;

}
