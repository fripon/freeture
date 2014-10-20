/*
								DetByLines.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
 * @file    DetByLines.cpp
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    17/06/2014
 */

#include "DetByLines.h"

/*
bool Detection::detectionTrailBy_Sum_And_HT(Mat sum){

    bool line = false;

    double tconvert = (double)getTickCount();

    double min, max, min1, max1;
    minMaxLoc(sum, &min, &max);
    sum.convertTo(sum, CV_8UC1, 255.0/((max/2) - min)); // , -min * 255.0/(max - min)
    medianBlur(sum, sum, 3);

    tconvert = (((double)getTickCount() - tconvert)/getTickFrequency())*1000;
    cout << "Times to convert: " <<std::setprecision(5)<< std::fixed<< tconvert << " ms"<< endl;

    double tHough = (double)getTickCount();

    Mat tmp;
    pyrDown( sum, tmp, Size( sum.cols/2, sum.rows/2 ));
    Mat maskDown = Mat::zeros(sum.rows,sum.cols, CV_8UC1);
    pyrDown(mask, maskDown, Size(mask.cols/2, mask.rows/2));

    minMaxLoc(sum, &min1, &max1);

    line = stdHT(tmp,        // HT will be applied on this image
                     maskDown,         // HT will be applied where the pixels have a 1 value
                     max1,     // pixels that exceed this threshold will be counted for search for aligned pixel
                     20,            // number of aligned pixel needed to consider to have a line
                     0,             // debug parameter to view HT in action with detected lines
                     0,             // debug parameter to view accumulator
                     1,             // compute canny edge detector
                     0,             // to reduce the height of the image from the bottom
                     1);            // to save image with detected lines

    tHough = (((double)getTickCount() - tHough)/getTickFrequency())*1000;
    //cout << "Times HT : " <<std::setprecision(5)<< std::fixed<< tHough << " ms"<< endl;

    return line;

}

bool Detection::detectionTrailBy_MaxPix_And_HT(Mat maxPix){

    bool line = false;

    double tHough = (double)getTickCount();
    double min, max;
    Mat tmp;
    pyrDown( maxPix, tmp, Size( maxPix.cols/2, maxPix.rows/2 ));
    minMaxLoc(maxPix, &min, &max);
    Mat mask2 = Mat::zeros(maxPix.rows,maxPix.cols, CV_8UC1);
    pyrDown( mask, mask2, Size( mask.cols/2, mask.rows/2 ));

    line = stdHT(tmp,        // HT will be applied on this image
                     mask2,         // HT will be applied where the pixels have a 1 value
                     max,     // pixels that exceed this threshold will be counted for search for aligned pixel
                     20,            // number of aligned pixel needed to consider to have a line
                     0,             // debug parameter to view HT in action with detected lines
                     0,             // debug parameter to view accumulator
                     1,             // compute canny edge detector
                     0,             // to reduce the height of the image from the bottom
                     1);            // to save image with detected lines

    tHough = (((double)getTickCount() - tHough)/getTickFrequency())*1000;
    //cout << "Times HT : " <<std::setprecision(5)<< std::fixed<< tHough << " ms"<< endl;

    return line;

}


bool Detection::stdHT( Mat imgInput,
                       Mat maskImage,
                       int pixThreshold,
                       int nbPixAligned,
                       bool viewHT,
                       bool viewAccu,
                       bool canny,
                       int cutFromBottom,
                       bool saveHT){

	Mat imgHough;
	imgInput.copyTo(imgHough);

	Mat imgCanny;
	imgInput.copyTo(imgCanny,maskImage);

	int img_h = imgInput.rows;
	int img_w = imgInput.cols;

	int nbLines = 0;
	bool lineFlag = false;

	if(viewHT){
		namedWindow("Input", CV_WINDOW_AUTOSIZE);
		cv::moveWindow("Input", 300, 400);
		namedWindow("Output", CV_WINDOW_AUTOSIZE);
		cv::moveWindow("Output", 1000, 400);
	}

	if(viewAccu)
		namedWindow("Accumulator", WINDOW_NORMAL );

    Scalar thresh = mean(imgInput,maskImage);

    pixThreshold = 15;

    //cout << "mean : "<< thresh.val[0]<<endl ;

	if(canny)
		//Canny(imgInput,imgCanny,100,150,3);
		//Canny(imgCanny,imgCanny,10,50,3);
		//Canny(imgCanny,imgCanny,thresh.val[0]*11,thresh.val[0]*16,3);

	if(imgHough.channels()==1)
		cvtColor(imgHough, imgHough, CV_GRAY2BGR);

	// compute the maximum rhô (r) value
	double r_max = sqrt(pow(img_h,2)+pow(img_w,2));

	// accumulator height : -r -> +r
	double accu_h = r_max * 2;

	// accumulator width
	double accu_w = 180;

	int accu_size = (int)accu_h * (int)accu_w;

	// accumulator memory allocation
	unsigned int * _accu = (unsigned int*)calloc(accu_size , sizeof(unsigned int));

	// bright pixel coordinates in imgInput
	double px = 0.;
	double py = 0.;

	// r value according to theta value
	double r = 0.;

	// theta value in radian
	double tRad = 0.;

	/// Buil accumulator

    uchar * ptr;
    uchar * ptr2;

	// iterate though imgInput
	for (int i = 0; i < img_h - cutFromBottom; i++){

        ptr = imgCanny.ptr<uchar>(i);
		ptr2 = maskImage.ptr<uchar>(i);

        for (int j = 0; j < img_w; j++){

			// decide which pixel will be transform in hough space
			if((int)ptr2[j]!=0){

                if((int)ptr[j]>pixThreshold){

                    // shift image's origin and save current pixel position
                    px = (double)j - img_w/2;
                    py = (double)i - img_h/2;

                    // compute r for different theta values
                    for(double t=0;t<180;t++){

                        tRad = (double)t* DEG2RAD;
                        r =  px * cos(tRad) + py * sin(tRad);

                        // increment the accumulator : we count the number of pixel which have the same (r,theta) value. It means that these pixels are aligned
                        _accu[ (int)(((Conversion::roundToNearest(r + r_max) * 180.0)) + t)]++;

                    }
                }
			}
        }
    }

	/// Search hough peaks and lines

	// iterate though accumulator
	for (int r = 0; r < 2* r_max; r++){

        for (int t = 0; t < 180; t++){

			// if there are enough pixels aligned for considerate that we have a line
			if((int)_accu[(r*180) + t] >= nbPixAligned){

				 // is this point a local maxima (9x9)
                int max = _accu[(r*180) + t];

                for(int ly=-4;ly<=4;ly++){

                    for(int lx=-4;lx<=4;lx++){

                        if( (ly+r>=0 && ly+r<2*r_max) && (lx+t>=0 && lx+t<180) )  {

                            if( (int)_accu[( (r+ly)*180) + (t+lx)] > max ){

                                max = _accu[( (r+ly)*180) + (t+lx)];
                                ly = lx = 5;

                            }
                        }
                    }
                }

                if(max > (int)_accu[(r*180) + t])
                    continue;

				int x1=0, x2=0, y1=0, y2=0;

				// compute line equation
				if(t >= 45 && t <= 135){

					x1 = 0;
					y1 =((double)(r-((r_max*2)/2)) - ((x1 - (img_w/2) ) * cos(t * DEG2RAD))) / sin(t * DEG2RAD) + ((img_h) / 2);

					x2 = img_w;
					y2 =((double)(r - ((r_max*2)/2)) -  ((x2 - (img_w/2) )* cos(t * DEG2RAD))) / sin(t * DEG2RAD) + (img_h / 2);

				}else{

					y1 = 0;
					x1 = ((double)(r-((r_max*2)/2)) - ((y1 - (img_h/2) ) * sin(t * DEG2RAD))) / cos(t * DEG2RAD) + (img_w / 2);
					y2 = img_h - 0;
					x2 = ((double)(r-((r_max*2)/2)) - ((y2 - (img_h/2) ) * sin(t * DEG2RAD))) / cos(t * DEG2RAD) + (img_w / 2);
				}

				// draw line
				line(imgHough,cvPoint(x1, y1),cvPoint(x2, y2),CV_RGB(255,0,0));
				nbLines ++ ;
				lineFlag = true;
			}
		}
	}

	putText(imgHough,"Lines : " +Conversion::intToString(nbLines), cvPoint(30,30), CV_FONT_HERSHEY_SIMPLEX,0.8,cvScalar(0,0,255),1);

	if(lineFlag && saveHT){


        string acquisitionDate = TimeDate::localDateTime(second_clock::universal_time(),"%Y%m%d_%H%M%S");
        saveImageToJPEG(imgHough, "hough_"+acquisitionDate);
        saveImageToJPEG(imgCanny, "canny_"+acquisitionDate);
        saveImageToJPEG(imgInput, "sum_"+acquisitionDate);

	}

	if(viewHT){

		imshow("Output", imgHough);
        imshow("Input", imgCanny);
        waitKey(27);

	}

	if(viewAccu){

	//	Mat imgAccu = accumulatorVisualization(r_max, 180, _accu);
	//	imshow("Accumulator", imgAccu);

	}

    free(_accu);

    return lineFlag;

}


   /// Meteor Detection by lines' detection

           /* double tLoopBuffer = (double)getTickCount();

            Mat sum = Mat::zeros(imgH,imgW, CV_32FC1);

            //CV_8U ou CV_16U en fonction du format de l'acquisition
            Mat diff;

            if(imgFormat == 8){

                diff = Mat::zeros(imgH,imgW, CV_8UC1);

            }else{

                diff = Mat::zeros(imgH,imgW, CV_16UC1);

            }

            if(!sumMethod){

                double tLoopBuffer = (double)getTickCount();

                for(int a = 0 ; a < framesQueue->getSizeQueue(); a++){

                    absdiff(framesQueue->getFifoElementAt(a).getImg(), framesQueue->getFifoElementAt(a+1).getImg(), diff);

                    accumulate(diff, sum);
                }

                tLoopBuffer = (((double)getTickCount() - tLoopBuffer)/getTickFrequency())*1000;
                //cout << "Time to sum : " <<std::setprecision(5)<< std::fixed<< tLoopBuffer << " ms"<< endl;

                lock.unlock(); //  on redonne l'acces au framebuffer partagé.

                lineFlag = detectionTrailBy_Sum_And_HT(sum);

            }else{

                if(!firstLoopDone){

                    Mat maxPixInit;

                    if(imgFormat == 8){

                        maxPixInit = Mat::zeros(imgH,imgW, CV_8UC1);

                    }else{

                        maxPixInit = Mat::zeros(imgH,imgW, CV_16UC1);

                    }

                    maxPixInit.copyTo(maxPix);

                    firstLoopDone = true;
                }

                double tLoop = (double)getTickCount();

                uchar * ptr;
                uchar * ptrMaxPix;

                absdiff(framesQueue->getFifoElementAt(0).getImg(), framesQueue->getFifoElementAt(1).getImg(), diff);

                lock.unlock(); //  on redonne l'acces au framebuffer partagé.

                for (int i = 0; i < imgH; i++){

                    ptr = diff.ptr<uchar>(i);
                    ptrMaxPix = maxPix.ptr<uchar>(i);

                    for (int j = 0; j < imgW; j++){

                        if((int)ptr[j]>(int)ptrMaxPix[j]){

                            ptrMaxPix[j] = (int)ptr[j];

                        }
                    }
                }

                tLoop = (((double)getTickCount() - tLoop)/getTickFrequency())*1000;
                //cout << "Time to sum : " <<std::setprecision(5)<< std::fixed<< tLoop << " ms"<< endl;

                Mat tmp2;

                maxPix.copyTo(tmp2);

                if(imgFormat!=8){

                    double min, max;
                    minMaxLoc(tmp2, &min, &max);
                    tmp2.convertTo(tmp2, CV_8UC1, 255.0/(max - min));

                }

                lineFlag = detectionTrailBy_MaxPix_And_HT(tmp2);

                if(cptImage==60){

                    if(imgFormat == 8){

                        maxPix = Mat::zeros(imgH,imgW, CV_8UC1);

                    }else{

                        maxPix = Mat::zeros(imgH,imgW, CV_16UC1);

                    }

                    cptImage = 1;

                }else{

                     cptImage ++;

                }
            }*/

