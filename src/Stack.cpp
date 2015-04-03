/*
								Stack.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
* \file    Stack.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    19/06/2014
* \brief
*/

#include "Stack.h"

Stack::Stack(int nbFrameToSum){

	fullStatus			= false;
	curFrames			= 0;
	maxFrames			= nbFrameToSum;

}

Stack::~Stack(){}

void Stack::addFrame(Frame &i){

	if(curFrames == 0){

		stack			= Mat::zeros(i.getImg().rows, i.getImg().cols, CV_32FC1);
		gainFirstFrame	= i.getGain();
		expFirstFrame	= i.getExposure();
		dateFirstFrame	= i.getAcqDateMicro();
		fps				= i.getFPS();
		bitdepth		= i.getFrameBitDepth();
		cout << "bitdepth : "<<bitdepth<< endl;

	}

	Mat curr = Mat::zeros(i.getImg().rows, i.getImg().cols, CV_32FC1);
	cout << "> STACK : " << curFrames << " / " << maxFrames  << endl;

    i.getImg().convertTo(curr, CV_32FC1);

    accumulate(curr, stack);
	curFrames++;
    dateLastFrame = i.getAcqDateMicro();

	if(curFrames >= maxFrames){

		fullStatus = true;

	}

}

bool Stack::saveStack(Fits fitsHeader, string path, StackMeth STACK_MTHD, string STATION_NAME, bool STACK_REDUCTION){

    cout << "dateLastFrame: " << dateLastFrame<< endl;
	// Vector<int> : YYYY, MM, DD, hh,mm, ss
	vector<int> firstDateInt = TimeDate::getIntVectorFromDateString(dateFirstFrame);
	vector<int> lastDateInt  = TimeDate::getIntVectorFromDateString(dateLastFrame);

	double  debObsInSeconds = firstDateInt.at(3)*3600 + firstDateInt.at(4)*60 + firstDateInt.at(5);
	double  endObsInSeconds = lastDateInt.at(3)*3600 + lastDateInt.at(4)*60 + lastDateInt.at(5);
	double  elapTime        = endObsInSeconds - debObsInSeconds;
	double  julianDate      = TimeDate::gregorianToJulian_2(firstDateInt);
	double  julianCentury   = TimeDate::julianCentury(julianDate);
	double  sideralT        = TimeDate::localSideralTime_2(julianCentury, firstDateInt.at(3), firstDateInt.at(4), firstDateInt.at(5), fitsHeader.getSitelong());

	// Fits creation.
    Fits2D newFits(path, fitsHeader);
	// Creation date of the fits file : YYYY-MM-DDTHH:MM:SS
    boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
    newFits.setDate(to_iso_extended_string(time));
    // Frame exposure time (sec.)
    newFits.setOntime(expFirstFrame);
    // Detector gain
    newFits.setGaindb(gainFirstFrame);
    // Acquisition date of the first frame 'YYYY-MM-JJTHH:MM:SS.SS'
    newFits.setDateobs(dateFirstFrame);
    // Integration time : 1/fps * nb_frames (sec.)
    if(fps <= 0) fps = 1;
    newFits.setExposure((1.0f/fps)*curFrames);
    // end obs. date - start obs. date (sec.)
    newFits.setElaptime(elapTime);
    // Sideral time
    newFits.setCrval1(sideralT);
    // Fps
    newFits.setCd3_3((double)fps);
    // Projection and reference system
    newFits.setCtype1("RA---ARC");
    newFits.setCtype2("DEC--ARC");
    // Equinox
    newFits.setEquinox(2000.0);

    switch(STACK_MTHD){

        case MEAN :

            {
                // 'SINGLE' 'SUM' 'AVERAGE' ('MEDIAN')
                newFits.setObsmode("AVERAGE");
				stack = stack/curFrames;

                switch(bitdepth){

                    case MONO_8 :

                        {

                            newFits.setSaturate(255);

                            Mat newMat = Mat(stack.rows,stack.cols, CV_8UC1, Scalar(0));

                            float * ptr;
                            unsigned char * ptr2;

                            for(int i = 0; i < stack.rows; i++){

                                ptr = stack.ptr<float>(i);
                                ptr2 = newMat.ptr<unsigned char>(i);

                                for(int j = 0; j < stack.cols; j++){

                                    ptr2[j] = (unsigned char)ptr[j];

                                }
                            }

                            // Create FITS image with BITPIX = BYTE_IMG (8-bits unsigned integers), pixel with TBYTE (8-bit unsigned byte)
                            newFits.writeFits(newMat, UC8, "" );

                        }

                        break;

                    case MONO_12 :

                        {

                            Mat newMat = Mat(stack.rows,stack.cols, CV_16SC1, Scalar(0));

                            newFits.setBzero(32768);
                            newFits.setBscale(1);
                            newFits.setSaturate(4095);

                            float * ptr;
                            short * ptr2;

                            for(int i = 0; i < stack.rows; i++){

                                ptr = stack.ptr<float>(i);
                                ptr2 = newMat.ptr<short>(i);

                                for(int j = 0; j < stack.cols; j++){

                                    if(ptr[j] - 32768 > 32767){

                                        ptr2[j] = 32767;

                                    }else{

                                        ptr2[j] = ptr[j] - 32768;
                                    }
                                }
                            }

                            newFits.writeFits(newMat, S16, "" );

                        }

                        break;

                }

            }

            break;

        case SUM :

            {

                // 'SINGLE' 'SUM' 'AVERAGE' ('MEDIAN')
                newFits.setObsmode("SUM");

                if(bitdepth == MONO_8)
                    newFits.setSaturate(255 * curFrames);

                else if(bitdepth = MONO_12)
                    newFits.setSaturate(4095 * curFrames);


                if(STACK_REDUCTION){

                    Mat newMat ;

                    float bzero	 = 0.0;
                    float bscale = 1.0;

                    reductionByFactorDivision(bzero, bscale).copyTo(newMat);

                    newFits.setBzero(bzero);
                    newFits.setBscale(bscale);

                    switch(bitdepth){

                        case MONO_8 :

                            {

                                newFits.writeFits(newMat, UC8, "" );

                            }

                            break;

                        case MONO_12 :

                            {

                                newFits.writeFits(newMat, S16, "" );

                            }

                            break;

                    }

                }else{

                    // Save fits in 32 bits.
                    newFits.writeFits(stack, F32, ""  );

                }

            }

            break;

    }

	return true;
}

Mat Stack::reductionByFactorDivision(float &bzero, float &bscale){

    Mat newMat;

    switch(bitdepth){

        case MONO_8 :

            {

                newMat = Mat(stack.rows,stack.cols, CV_8UC1, Scalar(0));
				float factor = curFrames;
                bscale = factor;
                bzero  = 0;

                float * ptr;
                unsigned char * ptr2;

                for(int i = 0; i < stack.rows; i++){

                    ptr = stack.ptr<float>(i);
                    ptr2 = newMat.ptr<unsigned char>(i);

                    for(int j = 0; j < stack.cols; j++){

                        ptr2[j] = cvRound(ptr[j] / factor) ;

                    }
                }
            }

            break;

        case MONO_12 :

            {

                newMat = Mat(stack.rows,stack.cols, CV_16SC1, Scalar(0));
                float factor = (4095.0f * curFrames)/65535;

                bscale = factor;
                bzero  = 32768 * factor;

                float * ptr;
                short * ptr2;

                for(int i = 0; i < stack.rows; i++){

                    ptr = stack.ptr<float>(i);
                    ptr2 = newMat.ptr<short>(i);

                    for(int j = 0; j < stack.cols; j++){

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
