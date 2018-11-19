/*
                                Stack.cpp

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
* \file    Stack.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    19/06/2014
* \brief
*/

#include "Stack.h"

boost::log::sources::severity_logger< LogSeverityLevel >  Stack::logger;

Stack::Init Stack::initializer;

Stack::Stack(string fitsCompression, fitskeysParam fkp, stationParam stp):
mFitsCompressionMethod(fitsCompression),
curFrames(0), varExpTime(false),
sumExpTime(0.0), gainFirstFrame(0), expFirstFrame(0), fps(0), format(MONO8){
    mfkp = fkp;
    mstp = stp;
}

Stack::~Stack(){}

void Stack::addFrame(Frame &i){

    try{

        if(TimeDate::getYYYYMMDD(i.mDate) != "00000000") {

            if(curFrames == 0){

                stack = Mat::zeros(i.mImg.rows, i.mImg.cols, CV_32FC1);
                gainFirstFrame = i.mGain;
                expFirstFrame = i.mExposure;
                mDateFirstFrame = i.mDate;
                fps = i.mFps;
                format = i.mFormat;

            }

            if(expFirstFrame != i.mExposure)
                varExpTime = true;

            sumExpTime+=i.mExposure;

            Mat curr = Mat::zeros(i.mImg.rows, i.mImg.cols, CV_32FC1);

            i.mImg.convertTo(curr, CV_32FC1);

            accumulate(curr, stack);
            curFrames++;
            mDateLastFrame = i.mDate;

        }

    }catch(exception& e){

        cout << e.what() << endl;
        BOOST_LOG_SEV(logger, critical) << e.what() ;

    }

}

bool Stack::saveStack(string path, StackMeth stackMthd, bool stackReduction){


    double  debObsInSeconds = mDateFirstFrame.hours*3600 + mDateFirstFrame.minutes*60 + mDateFirstFrame.seconds;
    double  endObsInSeconds = mDateLastFrame.hours*3600 + mDateLastFrame.minutes*60 + mDateLastFrame.seconds;
    double  elapTime        = endObsInSeconds - debObsInSeconds;
    double  julianDate      = TimeDate::gregorianToJulian(mDateFirstFrame);
    double  julianCentury   = TimeDate::julianCentury(julianDate);
    double  sideralT        = TimeDate::localSideralTime_2(julianCentury, mDateFirstFrame.hours,  mDateFirstFrame.minutes, (int)mDateFirstFrame.seconds, mstp.SITELONG);

    BOOST_LOG_SEV(logger, notification) << "Start create fits2D to save the stack.";

    // Fits creation.
    Fits2D newFits(path);
    newFits.loadKeys(mfkp, mstp);
    BOOST_LOG_SEV(logger, notification) << "Fits path : " << path;
    // Creation date of the fits file : YYYY-MM-DDTHH:MM:SS
    boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
    BOOST_LOG_SEV(logger, notification) << "Setting Fits DATE (creation date) key : " << to_iso_extended_string(time);
    newFits.kDATE = to_iso_extended_string(time);
    // Frame exposure time (sec.)
    BOOST_LOG_SEV(logger, notification) << "Setting fits ONTIME (Frame exposure time (sec.)) key : " << sumExpTime/1000000.0;
    newFits.kONTIME = sumExpTime/1000000.0;
    // Detector gain
    BOOST_LOG_SEV(logger, notification) << "Setting fits GAIN key : " << gainFirstFrame;
    newFits.kGAINDB = gainFirstFrame;
    // Acquisition date of the first frame 'YYYY-MM-JJTHH:MM:SS.SS'
    newFits.kDATEOBS = TimeDate::getIsoExtendedFormatDate(mDateFirstFrame);

    if(varExpTime)
        newFits.kEXPOSURE = 999999;
    else
        newFits.kEXPOSURE = expFirstFrame/1000000.0;

    // end obs. date - start obs. date (sec.)
    BOOST_LOG_SEV(logger, notification) << "Setting fits ELAPTIME (end obs. date - start obs. date (sec.)) key : " << elapTime;
    newFits.kELAPTIME = elapTime;
    // Sideral time
    BOOST_LOG_SEV(logger, notification) << "Setting fits CRVAL1 (sideraltime) key : " << sideralT;
    newFits.kCRVAL1 = sideralT;
    // Fps
    BOOST_LOG_SEV(logger, notification) << "Setting fits CD3_3 (fps) key : " << fps;
    newFits.kCD3_3 = (double)fps;
    // Projection and reference system
    BOOST_LOG_SEV(logger, notification) << "Setting fits DATEOBS key : RA---ARC";
    newFits.kCTYPE1 = "RA---ARC";
    BOOST_LOG_SEV(logger, notification) << "Setting fits DATEOBS key : DEC--ARC";
    newFits.kCTYPE2 = "DEC--ARC";
    // Equinox
    BOOST_LOG_SEV(logger, notification) << "Setting fits DATEOBS key : 2000.0";
    newFits.kEQUINOX = 2000.0;

    switch(stackMthd){

        case MEAN :

            {

                BOOST_LOG_SEV(logger, notification) << "MEAN STACK MODE";

                // 'SINGLE' 'SUM' 'AVERAGE' ('MEDIAN')
                newFits.kOBSMODE = "AVERAGE";
                stack = stack/curFrames;

                switch(format){

                    case MONO12 :

                        {
                            BOOST_LOG_SEV(logger, notification) << "Mono12 format";

                            Mat newMat = Mat(stack.rows,stack.cols, CV_16SC1, Scalar(0));

                            BOOST_LOG_SEV(logger, notification) << "Setting fits BZERO key : 32768";
                            newFits.kBZERO = 32768;
                            BOOST_LOG_SEV(logger, notification) << "Setting fits BSCALE key : 1";
                            newFits.kBSCALE = 1;
                            BOOST_LOG_SEV(logger, notification) << "Setting fits SATURATE key : 4095";
                            newFits.kSATURATE = 4095;


                            float *ptr = NULL;
                            short *ptr2 = NULL;

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
                            return newFits.writeFits(newMat, S16, "", mFitsCompressionMethod);

                        }

                        break;

                    default :

                        {
                            BOOST_LOG_SEV(logger, notification) << "Mono8 format";

                            BOOST_LOG_SEV(logger, notification) << "Setting fits SATURATE key : 255";
                            newFits.kSATURATE = 255;

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
                            return newFits.writeFits(newMat, UC8, "" , mFitsCompressionMethod);

                        }

                }

            }

            break;

        case SUM :

            {
                BOOST_LOG_SEV(logger, notification) << "SUM STACK MODE";

                // 'SINGLE' 'SUM' 'AVERAGE' ('MEDIAN')
                newFits.kOBSMODE = "SUM";


                if(format == MONO12){
                    BOOST_LOG_SEV(logger, notification) << "Setting fits SATURATE key : 4095 * curFrames";
                    newFits.kSATURATE = 4095 * curFrames;
                }
                else {
                    BOOST_LOG_SEV(logger, notification) << "Setting fits SATURATE key : 255 * curFrames";
                    newFits.kSATURATE = 255 * curFrames;
                }

                if(stackReduction){

                    BOOST_LOG_SEV(logger, notification) << "stackReduction option enabled";

                    Mat newMat ;

                    float bzero  = 0.0;
                    float bscale = 1.0;

                    BOOST_LOG_SEV(logger, notification) << "Call reduction function.";
                    reductionByFactorDivision(bzero, bscale).copyTo(newMat);

                    BOOST_LOG_SEV(logger, notification) << "Setting fits BZERO key : " << bzero;
                    newFits.kBZERO = bzero;
                    BOOST_LOG_SEV(logger, notification) << "Setting fits BSCALE key : " << bscale;
                    newFits.kBSCALE = bscale;

                    switch(format){

                        case MONO12 :

                            {
                                BOOST_LOG_SEV(logger, notification) << "Writting Fits signed short.";
                                return newFits.writeFits(newMat, S16, "", mFitsCompressionMethod);

                            }

                            break;

                        default :

                            {
                                BOOST_LOG_SEV(logger, notification) << "Writting Fits unsigned char.";
                                return newFits.writeFits(newMat, UC8, "", mFitsCompressionMethod);

                            }

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

    switch(format){

        case MONO12 :

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

        default :

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

    }

    return newMat;

}
