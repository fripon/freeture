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

boost::log::sources::severity_logger< LogSeverityLevel >  Stack::logger;
Stack::_Init Stack::_initializer;

Stack::Stack(int nbFrameToSum){

	fullStatus			= false;
	curFrames			= 0;
	maxFrames			= nbFrameToSum;


}

Stack::~Stack(){}

void Stack::addFrame(Frame &i){

    try{

        if(curFrames == 0){

            BOOST_LOG_SEV(logger, notification) << "First frame of stack received.";

            stack			= Mat::zeros(i.getImg().rows, i.getImg().cols, CV_32FC1);
            gainFirstFrame	= i.getGain();
            expFirstFrame	= i.getExposure();
            dateFirstFrame	= i.getAcqDateMicro();
            fps				= i.getFPS();
            bitdepth		= i.getFrameBitDepth();

            BOOST_LOG_SEV(logger, notification) << "Gain : " << gainFirstFrame;
            BOOST_LOG_SEV(logger, notification) << "Exposure : " << expFirstFrame;
            BOOST_LOG_SEV(logger, notification) << "Date : " << dateFirstFrame;
            BOOST_LOG_SEV(logger, notification) << "Fps : " << fps;
            BOOST_LOG_SEV(logger, notification) << "Bitdepth : " << bitdepth;

        }

        Mat curr = Mat::zeros(i.getImg().rows, i.getImg().cols, CV_32FC1);

        cout << "> STACK : " << curFrames << " / " << maxFrames  << endl;
        BOOST_LOG_SEV(logger, normal) << "> STACK : " << curFrames << " / " << maxFrames;

        i.getImg().convertTo(curr, CV_32FC1);
        cout << "accumulate" << endl;
        accumulate(curr, stack);
        curFrames++;
        dateLastFrame = i.getAcqDateMicro();
        cout << "dateLastFrame: " << dateLastFrame << endl;

        if(curFrames >= maxFrames){

            BOOST_LOG_SEV(logger, notification) << "Last frame of stack received.";
            BOOST_LOG_SEV(logger, notification) << "Date : " << dateLastFrame;

            fullStatus = true;

        }

    }catch(exception& e){

        cout << e.what() << endl;
        BOOST_LOG_SEV(logger, critical) << e.what() ;

    }

}

bool Stack::saveStack(Fits fitsHeader, string path, StackMeth STACK_MTHD, string STATION_NAME, bool STACK_REDUCTION){

	// Vector<int> : YYYY, MM, DD, hh,mm, ss
	vector<int> firstDateInt = TimeDate::getIntVectorFromDateString(dateFirstFrame);
	vector<int> lastDateInt  = TimeDate::getIntVectorFromDateString(dateLastFrame);

	double  debObsInSeconds = firstDateInt.at(3)*3600 + firstDateInt.at(4)*60 + firstDateInt.at(5);
	double  endObsInSeconds = lastDateInt.at(3)*3600 + lastDateInt.at(4)*60 + lastDateInt.at(5);
	double  elapTime        = endObsInSeconds - debObsInSeconds;
	double  julianDate      = TimeDate::gregorianToJulian_2(firstDateInt);
	double  julianCentury   = TimeDate::julianCentury(julianDate);
	double  sideralT        = TimeDate::localSideralTime_2(julianCentury, firstDateInt.at(3), firstDateInt.at(4), firstDateInt.at(5), fitsHeader.getSitelong());

    BOOST_LOG_SEV(logger, notification) << "Start create fits2D to save the stack.";

	// Fits creation.
    Fits2D newFits(path, fitsHeader);
    BOOST_LOG_SEV(logger, notification) << "Fits path : " << path;
	// Creation date of the fits file : YYYY-MM-DDTHH:MM:SS
    boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
    BOOST_LOG_SEV(logger, notification) << "Setting Fits DATE (creation date) key : " << to_iso_extended_string(time);
    newFits.setDate(to_iso_extended_string(time));
    // Frame exposure time (sec.)
    BOOST_LOG_SEV(logger, notification) << "Setting fits ONTIME (Frame exposure time (sec.)) key : " << expFirstFrame/1000000.0;
    newFits.setOntime(expFirstFrame/1000000.0);
    // Detector gain
    BOOST_LOG_SEV(logger, notification) << "Setting fits GAIN key : " << gainFirstFrame;
    newFits.setGaindb(gainFirstFrame);
    // Acquisition date of the first frame 'YYYY-MM-JJTHH:MM:SS.SS'
    BOOST_LOG_SEV(logger, notification) << "Setting fits DATEOBS (Acquisition date of the first frame) key : " << dateFirstFrame;
    newFits.setDateobs(dateFirstFrame);
    // Integration time : 1/fps * nb_frames (sec.)
    BOOST_LOG_SEV(logger, notification) << "Setting fits EXPOSURE (Integration time : 1/fps * nb_frames (sec.)) key : " << (1.0f/fps)*curFrames;
    if(fps <= 0) fps = 1;
    newFits.setExposure((1.0f/fps)*curFrames);
    // end obs. date - start obs. date (sec.)
    BOOST_LOG_SEV(logger, notification) << "Setting fits ELAPTIME (end obs. date - start obs. date (sec.)) key : " << elapTime;
    newFits.setElaptime(elapTime);
    // Sideral time
    BOOST_LOG_SEV(logger, notification) << "Setting fits CRVAL1 (sideraltime) key : " << sideralT;
    newFits.setCrval1(sideralT);
    // Fps
    BOOST_LOG_SEV(logger, notification) << "Setting fits CD3_3 (fps) key : " << fps;
    newFits.setCd3_3((double)fps);
    // Projection and reference system
    BOOST_LOG_SEV(logger, notification) << "Setting fits DATEOBS key : RA---ARC";
    newFits.setCtype1("RA---ARC");
    BOOST_LOG_SEV(logger, notification) << "Setting fits DATEOBS key : DEC--ARC";
    newFits.setCtype2("DEC--ARC");
    // Equinox
    BOOST_LOG_SEV(logger, notification) << "Setting fits DATEOBS key : 2000.0";
    newFits.setEquinox(2000.0);

    switch(STACK_MTHD){

        case MEAN :

            {

                BOOST_LOG_SEV(logger, notification) << "MEAN STACK MODE";

                // 'SINGLE' 'SUM' 'AVERAGE' ('MEDIAN')
                newFits.setObsmode("AVERAGE");
				stack = stack/curFrames;

                switch(bitdepth){

                    case MONO_8 :

                        {
                            BOOST_LOG_SEV(logger, notification) << "Mono8 format";

                            BOOST_LOG_SEV(logger, notification) << "Setting fits SATURATE key : 255";
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
                            BOOST_LOG_SEV(logger, notification) << "Writing FITS image with BITPIX = BYTE_IMG (8-bits unsigned integers), pixel with TBYTE (8-bit unsigned byte)";
                            return newFits.writeFits(newMat, UC8, "" );

                        }

                        break;

                    case MONO_12 :

                        {
                            BOOST_LOG_SEV(logger, notification) << "Mono12 format";

                            Mat newMat = Mat(stack.rows,stack.cols, CV_16SC1, Scalar(0));

                            BOOST_LOG_SEV(logger, notification) << "Setting fits BZERO key : 32768";
                            newFits.setBzero(32768);
                            BOOST_LOG_SEV(logger, notification) << "Setting fits BSCALE key : 1";
                            newFits.setBscale(1);
                            BOOST_LOG_SEV(logger, notification) << "Setting fits SATURATE key : 4095";
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

                            BOOST_LOG_SEV(logger, notification) << "Writing FITS signed short image.";
                            return newFits.writeFits(newMat, S16, "" );

                        }

                        break;

                }

            }

            break;

        case SUM :

            {
                BOOST_LOG_SEV(logger, notification) << "SUM STACK MODE";

                // 'SINGLE' 'SUM' 'AVERAGE' ('MEDIAN')
                newFits.setObsmode("SUM");

                if(bitdepth == MONO_8){
                    BOOST_LOG_SEV(logger, notification) << "Setting fits SATURATE key : 255 * curFrames";
                    newFits.setSaturate(255 * curFrames);
                }
                else if(bitdepth = MONO_12){
                    BOOST_LOG_SEV(logger, notification) << "Setting fits SATURATE key : 4095 * curFrames";
                    newFits.setSaturate(4095 * curFrames);
                }

                if(STACK_REDUCTION){

                    BOOST_LOG_SEV(logger, notification) << "STACK_REDUCTION option enabled";

                    Mat newMat ;

                    float bzero	 = 0.0;
                    float bscale = 1.0;

                    BOOST_LOG_SEV(logger, notification) << "Call reduction function.";
                    reductionByFactorDivision(bzero, bscale).copyTo(newMat);

                    BOOST_LOG_SEV(logger, notification) << "Setting fits BZERO key : " << bzero;
                    newFits.setBzero(bzero);
                    BOOST_LOG_SEV(logger, notification) << "Setting fits BSCALE key : " << bscale;
                    newFits.setBscale(bscale);

                    switch(bitdepth){

                        case MONO_8 :

                            {
                                BOOST_LOG_SEV(logger, notification) << "Writting Fits unsigned char.";
                                return newFits.writeFits(newMat, UC8, "" );

                            }

                            break;

                        case MONO_12 :

                            {
                                BOOST_LOG_SEV(logger, notification) << "Writting Fits signed short.";
                                return newFits.writeFits(newMat, S16, "" );

                            }

                            break;

                    }

                }else{

                    // Save fits in 32 bits.
                    BOOST_LOG_SEV(logger, notification) << "Writting Fits 32 bits.";
                    return newFits.writeFits(stack, F32, ""  );

                }

            }

            break;

    }

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
                float factor = (4095.0f * curFrames)/4095.0f;

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
