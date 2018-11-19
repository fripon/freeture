/*
                            ExposureControl.cpp

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
* \file    ExposureControl.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Auto Exposure time adjustment.
*/

#include "ExposureControl.h"

boost::log::sources::severity_logger< LogSeverityLevel >  ExposureControl::logger;

ExposureControl::Init ExposureControl::initializer;

ExposureControl::ExposureControl(int timeInterval,
                                 bool saveImage,
                                 bool saveInfos,
                                 string dataPath,
                                 string station){

    bin_0 = 0;
    bin_1 = 0;
    bin_2 = 0;
    bin_3 = 0;
    bin_4 = 0;

    // Frame interval between two exposure time adjustment.
    autoExposureTimeInterval = timeInterval;
    // Exposure adjustment finished status.
    autoExposureFinished = false;
    // Exposure adjustment initialization status.
    autoExposureInitialized = false;

    stationName = station;

    autoExposureSaveImage = saveImage;

    autoExposureSaveInfos = saveInfos;
    autoExposureDataLocation = dataPath;

    minCameraExposureValue = 0;
    maxCameraExposureValue = 0;

    exposureValue = 0;

    frameToSkip = 2;
    frameSkippedCounter = 0;

    incrementExposureTimeValue = true;

    msvMin_1 = 0.f;
    msvMax_1 = 0.f;
    expMin_1 = 0;
    expMax_1 = 0;

    msvMin_2 = 0.f;
    msvMax_2 = 0.f;
    expMin_2 = 0;
    expMax_2 = 0;

    step1 = true;
    step2 = false;

    finalDataLocation = "";

    finalExposureTime = 0;

    // First reference date.
    boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
    string cDate = to_simple_string(time);
    string dateDelimiter = ".";
    mRefDate = cDate.substr(0, cDate.find(dateDelimiter));
    mSecTime = 0;
    mNbFramesControlled = 0;
}

bool ExposureControl::calculate(Mat& image, Mat &mask){

    if(!mask.data){

        mask = Mat(image.rows, image.cols, CV_8UC1, Scalar(255));

    }

    this->clear();

    unsigned char * ptr_mask;

    switch(image.type()){

        case CV_16U :

            {

                unsigned short * ptr = NULL;

                // Loop pixel's region.
                for(int i = 0; i < image.rows; i++){

                    ptr = image.ptr<unsigned short>(i);
                    ptr_mask = mask.ptr<unsigned char>(i);

                    for(int j = 0; j < image.cols; j++){

                        if(ptr_mask[j] != 0){

                            if(ptr[j] >= 0 && ptr[j] <= 819){

                                bin_0 += 1.f;

                            }else if(ptr[j] > 819 && ptr[j] <= 1638){

                                bin_1 += 1.f;

                            }else if(ptr[j] > 1638 && ptr[j] <= 2458){

                                bin_2 += 1.f;

                            }else if(ptr[j] > 2458 && ptr[j] <= 3278){

                                bin_3 += 1.f;

                            }else if(ptr[j] > 3278 && ptr[j] <= 4095){

                                bin_4 += 1.f;

                            }
                        }
                    }
                }

            }

            break;

        case CV_8U :

            {

                unsigned char * ptr = NULL;

                // Loop pixel's region.
                for(int i = 0; i < image.rows; i++){

                    ptr = image.ptr<unsigned char>(i);
                    ptr_mask = mask.ptr<unsigned char>(i);

                    for(int j = 0; j < image.cols; j++){

                        if((int)ptr_mask[j] != 0){

                            if(ptr[j] >= 0 && ptr[j] <= 50){

                                bin_0 += 1;

                            }else if(ptr[j] > 50 && ptr[j] <= 100){

                                bin_1 += 1;

                            }else if(ptr[j] > 100 && ptr[j] <= 150){

                                bin_2 += 1;

                            }else if(ptr[j] > 150 && ptr[j] <= 200){

                                bin_3 += 1;

                            }else if(ptr[j] > 200 && ptr[j] <= 255){

                                bin_4 += 1;

                            }
                        }
                    }
                }
            }

            break;
    }

    return true;

}

float ExposureControl::computeMSV(){

    return ((bin_0 + 2 * bin_1 + 3 * bin_2 + 4 * bin_3 + 5 * bin_4) / (bin_0 + bin_1 + bin_2 + bin_3 + bin_4));

}

bool ExposureControl::controlExposureTime(Device *camera, Mat image, TimeDate::Date imageDate, Mat mask, double minExposureTime, double fps){

    try {

        // Get current DATE and check number of seconds passed since last exposure control.
        boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
        string cDate = to_simple_string(time);
        string dateDelimiter = ".";
        string nowDate = cDate.substr(0, cDate.find(dateDelimiter));
        boost::posix_time::ptime t1(boost::posix_time::time_from_string(mRefDate));
        boost::posix_time::ptime t2(boost::posix_time::time_from_string(nowDate));
        boost::posix_time::time_duration td = t2 - t1;
        mSecTime = td.total_seconds();

        // Check if it's time to run "exposure time control".
        if(mSecTime > autoExposureTimeInterval) {

            // If exposure control finished
            if(autoExposureFinished) {

                autoExposureFinished = false;

                // Get new reference DATE to count the next exposure control.
                boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
                string cDate = to_simple_string(time);
                string dateDelimiter = ".";
                mRefDate = cDate.substr(0, cDate.find(dateDelimiter));
                mSecTime = 0;

            // If exposure control not finished
            }else {

                // Skip n frames to give enough time to set new exposure value.
                if(frameSkippedCounter <= frameToSkip) {

                    frameSkippedCounter++;

                }else {

                    // Compute pixels value repartition.
                    calculate(image, mask);

                    // Compute Mean Sample Value
                    float msv = computeMSV();

                    BOOST_LOG_SEV(logger,notification) << mNbFramesControlled << " -> EXP : " << exposureValue <<  " MSV : " << msv;

                    // If method has not been initialized
                    if(!autoExposureInitialized) {

                        if(autoExposureSaveImage) {

                            // Save initial status before exposure control

                            Mat saveFrame; image.copyTo(saveFrame);

                            if(saveFrame.type() == CV_16U) saveFrame = Conversion::convertTo8UC1(saveFrame);

                            putText(saveFrame, "BEFORE EC -> MSV : " + Conversion::floatToString(msv), cvPoint(20,20),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(255), 1, CV_AA);

                            putText(saveFrame, "EXP : ? " /*Conversion::intToString(camera->mCam->getExposureTime())*/, cvPoint(20,40),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(255), 1, CV_AA);

                            if(checkDataLocation(imageDate))
                                SaveImg::saveBMP(saveFrame, finalDataLocation + "expControl_" + TimeDate::getYYYYMMDDThhmmss(imageDate) + "_before");

                        }

                        // Get minimum exposure time.
                        minCameraExposureValue = minExposureTime;
                        BOOST_LOG_SEV(logger,notification) << "Min EXP : " << minCameraExposureValue;

                        // Set maximum exposure time (us) according fps value.
                        if(fps > 0) maxCameraExposureValue = (int)((1.0/fps) * 1000000.0);
                        else throw "Fail to get FPS value from camera.";
                        BOOST_LOG_SEV(logger,notification) << "Max EXP : " << maxCameraExposureValue;

                        // Compute msv with the following exposure time
                        exposureValue = minCameraExposureValue;

                        // Set exposure time with the minimum value
                        if(!camera->setCameraExposureTime(minCameraExposureValue))
                            throw "Fail to set exposure time in initialization.";

                        BOOST_LOG_SEV(logger,notification) << "Set EXP to : " << minCameraExposureValue;
                        expArray_1.push_back(minCameraExposureValue);
                        autoExposureInitialized = true;

                        // Reset counter of skipped frames.
                        frameSkippedCounter = 0;

                    // If method has been initialized
                    }else{

                        if(step1) {

                            BOOST_LOG_SEV(logger,notification) << "STEP 1";

                            msvArray_1.push_back(msv);

                            // Increment exposure time.
                            if(incrementExposureTimeValue) {

                                double delta = 1000;

                                BOOST_LOG_SEV(logger,notification) << "STEP 1 : Incrementation by " << delta;

                                if(exposureValue + delta > maxCameraExposureValue) {

                                    exposureValue = maxCameraExposureValue;
                                    incrementExposureTimeValue = false;

                                }else if(exposureValue == minCameraExposureValue) {

                                    exposureValue = delta;

                                }else {

                                    exposureValue += delta;

                                }

                                expArray_1.push_back(exposureValue);

                                if(!camera->setCameraExposureTime(exposureValue))
                                    throw "Fail to set exposure time in step 1 : incrementation";

                                BOOST_LOG_SEV(logger,notification) << "Set EXP to : " << exposureValue;

                            }else{

                                BOOST_LOG_SEV(logger,notification) << "STEP 1 : Analyse msv";
                                BOOST_LOG_SEV(logger,notification) << "MSV ARRAY1 SIZE : " << msvArray_1.size() << "EXP ARRAY1 SIZE " << expArray_1.size();

                                // MSV too high (over exposition) -> set camera with minimum exposure time.
                                if(msvArray_1.front() > 2.5) {

                                    finalExposureTime = minCameraExposureValue;
                                    exposureValue = minCameraExposureValue;
                                    if(!camera->setCameraExposureTime(minCameraExposureValue))
                                        throw "Fail to set exposure time in step 1 : Analyse MSV > 2.5";

                                    BOOST_LOG_SEV(logger,notification) << "Set EXP to : " << exposureValue;

                                // MSV too low (under exposition) -> set camera with maximum exposure time.
                                }else if(msvArray_1.back() < 2.5) {

                                    finalExposureTime = maxCameraExposureValue;
                                    exposureValue = maxCameraExposureValue;
                                    if(!camera->setCameraExposureTime(maxCameraExposureValue))
                                        throw "Fail to set exposure time in step 1 : Analyse MSV < 2.5";

                                    BOOST_LOG_SEV(logger,notification) << "Set EXP to : " << exposureValue;

                                // Search best MSV and attached exposure time.
                                }else {

                                    for(int i = 0; i < msvArray_1.size(); i++) {

                                        if((i+1) < msvArray_1.size()) {

                                            // If two consecutives MSV values around 2.5
                                            if(msvArray_1.at(i) < 2.5 && msvArray_1.at(i+1) > 2.5 ) {

                                                expMin_1 = expArray_1.at(i);
                                                expMax_1 = expArray_1.at(i+1);
                                                msvMin_1 = msvArray_1.at(i);
                                                msvMax_1 = msvArray_1.at(i+1);

                                                minCameraExposureValue = expMin_1;
                                                maxCameraExposureValue = expMax_1;

                                                step2 = true;

                                                incrementExposureTimeValue = true;
                                                exposureValue = expMin_1;
                                                expMax_2 = expMax_1;

                                                BOOST_LOG_SEV(logger,notification) << "New interval found -> MSV[" << msvMin_1 << "-" << msvMax_1 << "] EXP[" << expMin_1 << "-" << expMax_1 << "]";

                                                if(!camera->setCameraExposureTime(exposureValue))
                                                    throw "Fail to set exposure time in step 1 : Analyse MSV : search";

                                                BOOST_LOG_SEV(logger,notification) << "Set EXP to : " << exposureValue;

                                                break;

                                            }
                                        }
                                    }
                                }

                                step1 = false;

                                /*for(int i = 0; i< msvArray_1.size(); i++){

                                    cout << "msvArray_1 at " << i << " : " << msvArray_1.at(i)<< endl;

                                }

                                for(int i = 0; i< expArray_1.size(); i++){

                                    cout << "expArray_1 at " << i << " : " << expArray_1.at(i)<< endl;

                                }*/

                            }

                        }else if(step2) {

                            BOOST_LOG_SEV(logger,notification) << "STEP 2";

                            msvArray_2.push_back(msv);

                            if(incrementExposureTimeValue) {

                                double delta = 30;

                                BOOST_LOG_SEV(logger,notification) << "STEP 2 : Incrementation by " << delta;

                                if(exposureValue + delta > expMax_1) {

                                    exposureValue = expMax_1;
                                    incrementExposureTimeValue = false;

                                }else{

                                    exposureValue += delta;

                                }

                                expArray_2.push_back(exposureValue);

                                if(!camera->setCameraExposureTime(exposureValue))
                                    throw "Fail to set exposure time in step 2 : incrementation";

                                BOOST_LOG_SEV(logger,notification) << "Set EXP to : " << exposureValue;

                            }else {

                                BOOST_LOG_SEV(logger,notification) << "STEP 2 : Analyse msv";
                                BOOST_LOG_SEV(logger,notification) << "MSV ARRAY2 SIZE : " << msvArray_2.size() << "EXP ARRAY2 SIZE " << expArray_2.size();

                                // MSV too high (over exposition) -> set camera with minimum exposure time.
                                if(msvArray_2.front() > 2.5) {

                                    finalExposureTime = expMin_1;
                                    if(!camera->setCameraExposureTime(expMin_1))
                                        throw "Fail to set exposure time in step 2 : Analyse MSV > 2.5";
                                    BOOST_LOG_SEV(logger,notification) << "Set EXP to : " << expMin_1;

                                // MSV too low (under exposition) -> set camera with maximum exposure time.
                                }else if(msvArray_2.back() < 2.5) {

                                    finalExposureTime = expMax_2;
                                    if(!camera->setCameraExposureTime(expMax_2))
                                        throw "Fail to set exposure time in step 2 : Analyse MSV < 2.5";
                                    BOOST_LOG_SEV(logger,notification) << "Set EXP to : " << expMax_2;

                                // Search best MSV and attached exposure time.
                                }else {

                                    for(int i = 0; i < msvArray_2.size(); i++) {

                                        if((i+1) < msvArray_2.size()) {

                                            if(msvArray_2.at(i) < 2.5 && msvArray_2.at(i+1) > 2.5 ) {

                                                expMin_2 = expArray_2.at(i);
                                                expMax_2 = expArray_2.at(i+1);
                                                msvMin_2 = msvArray_2.at(i);
                                                msvMax_2 = msvArray_2.at(i+1);

                                                if((2.5 - msvArray_1.at(i)) < (msvArray_1.at(i+1) - 2.5)) {

                                                    finalExposureTime = expArray_2.at(i);
                                                    exposureValue = finalExposureTime;

                                                }else{

                                                    finalExposureTime = expArray_2.at(i+1);
                                                    exposureValue = finalExposureTime;

                                                }

                                                if(!camera->setCameraExposureTime(exposureValue))
                                                    throw "Fail to set exposure time in step 2";
                                                BOOST_LOG_SEV(logger,notification) << "Set EXP to : " << exposureValue;

                                                BOOST_LOG_SEV(logger,notification) << "Interval choose -> MSV[" << msvMin_2 << "-" << msvMax_2 << "] EXP[" << expMin_2 << "-" << expMax_2 << "]";

                                                break;

                                            }
                                        }
                                    }
                                }

                                step2 = false;

                                /*
                                for(int i = 0; i< msvArray_2.size(); i++){

                                    cout << "msvArray_2 at " << i << " : " << msvArray_2.at(i)<< endl;

                                }

                                for(int i = 0; i< expArray_2.size(); i++){

                                    cout << "expArray_2 at " << i << " : " << expArray_2.at(i)<< endl;

                                }*/

                                BOOST_LOG_SEV(logger,notification) << "FINAL EXPOSURE : " << finalExposureTime;

                            }

                        }else {

                            // Save data to control exposure time process
                            if(autoExposureSaveImage) {

                                Mat saveFrame; image.copyTo(saveFrame);

                                if(saveFrame.type() == CV_16U) saveFrame = Conversion::convertTo8UC1(saveFrame);

                                putText(saveFrame, "AFTER EC -> MSV : " + Conversion::floatToString(msv), cvPoint(20,20),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(255), 1, CV_AA);

                                putText(saveFrame, "EXP : " + Conversion::intToString(finalExposureTime), cvPoint(20,40),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(255), 1, CV_AA);

                                if(checkDataLocation(imageDate))
                                    SaveImg::saveBMP(saveFrame, finalDataLocation + "expControl_" + TimeDate::getYYYYMMDDThhmmss(imageDate) + "_after" );

                            }

                            if(autoExposureSaveInfos) {

                                if(checkDataLocation(imageDate)) {

                                    ofstream infFile;
                                    string infFilePath = finalDataLocation + "ECInfos.txt";
                                    infFile.open(infFilePath.c_str(),std::ios_base::app);

                                    string d = TimeDate::getYYYYMMDDThhmmss(imageDate);

                                    infFile << "# DATE : " << d << "  EXP : " << Conversion::doubleToString(exposureValue) << "  MSV : "<< Conversion::floatToString(msv) << "\n";

                                    infFile.close();

                                }

                            }

                            // Reset variables
                            autoExposureFinished = true;
                            autoExposureInitialized = false;
                            step1 = true;
                            step2 = false;
                            incrementExposureTimeValue = true;
                            mNbFramesControlled = 0;

                            msvMin_1 = 0.f;
                            expMin_1 = 0.f;
                            msvMax_1 = 0;
                            expMax_1 = 0;

                            msvMin_2 = 0.f;
                            expMin_2 = 0.f;
                            msvMax_2 = 0;
                            expMax_2 = 0;

                            expArray_1.clear();
                            msvArray_1.clear();

                            expArray_2.clear();
                            msvArray_2.clear();

                            minCameraExposureValue = 0;
                            maxCameraExposureValue = 0;

                            exposureValue = 0;

                            finalExposureTime = 0;

                        }

                        frameSkippedCounter = 0;
                    }
                }
            }

            return true;

        }else{

            cout << "EXPOSURE CONTROL : " << mSecTime << "/" << autoExposureTimeInterval <<  endl;
            return false;
        }

    }catch(exception& e) {

        cout << "An exception occured : " << e.what() << endl;

    }catch(const char * msg) {

        BOOST_LOG_SEV(logger,fail) << msg << endl;
    }

    // Reset variables
    autoExposureFinished = true;
    autoExposureInitialized = false;
    mNbFramesControlled = 0;
    step1 = true;
    step2 = false;
    incrementExposureTimeValue = true;
    msvMin_1 = 0.f;
    expMin_1 = 0.f;
    msvMax_1 = 0;
    expMax_1 = 0;
    msvMin_2 = 0.f;
    expMin_2 = 0.f;
    msvMax_2 = 0;
    expMax_2 = 0;
    expArray_1.clear();
    msvArray_1.clear();
    expArray_2.clear();
    msvArray_2.clear();
    minCameraExposureValue = 0;
    maxCameraExposureValue = 0;
    exposureValue = 0;
    finalExposureTime = 0;
    frameSkippedCounter = 0;

    return false;
}

bool ExposureControl::checkDataLocation(TimeDate::Date date){


    namespace fs = boost::filesystem;

    // data/
    path p(autoExposureDataLocation);

    // data/STATION_YYYYMMDD/
    string sp1 = autoExposureDataLocation + stationName + "_" + TimeDate::getYYYYMMDD(date) +"/";
    path p1(sp1);

    // data/STATION_YYYMMDD/exposure/
    string sp2 = sp1 + "exposure/";
    path p2(sp2);

    finalDataLocation = sp2;

    // If data/ exists
    if(fs::exists(p)){

        // If data/STATION_YYYMMDD/ exists
        if(fs::exists(p1)){

            // If data/STATION_YYYMMDD/exposure/ doesn't exists
            if(!fs::exists(p2)){

                // If fail to create data/STATION_YYYMMDD/exposure/
                if(!fs::create_directory(p2)){

                    //BOOST_LOG_SEV(logger,critical) << "Unable to create stack directory : " << p2.string();
                    return false;

                // If success to create data/STATION_YYYMMDD/exposure/
                }else{

                   //BOOST_LOG_SEV(logger,notification) << "Success to create stack directory : " << p2.string();
                   return true;

                }
            }

        // If data/STATION_YYYMMDD/ doesn't exists
        }else{

            // If fail to create data/STATION_YYYMMDD/
            if(!fs::create_directory(p1)){

                //BOOST_LOG_SEV(logger,fail) << "Unable to create STATION_YYYYMMDD directory : " << p1.string();
                return false;

            // If success to create data/STATION_YYYMMDD/
            }else{

                //BOOST_LOG_SEV(logger,notification) << "Success to create STATION_YYYYMMDD directory : " << p1.string();

                // If fail to create data/STATION_YYYMMDD/exposure/
                if(!fs::create_directory(p2)){

                    //BOOST_LOG_SEV(logger,critical) << "Unable to create stack directory : " << p2.string();
                    return false;

                // If success to create data/STATION_YYYMMDD/exposure/
                }else{

                    //BOOST_LOG_SEV(logger,notification) << "Success to create stack directory : " << p2.string();
                    return true;

                }
            }
        }

    // If data/  doesn't exists
    }else{

        // If fail to create data/
        if(!fs::create_directory(p)){

            //BOOST_LOG_SEV(logger,fail) << "Unable to create data/  directory : " << p.string();
            return false;

        // If success to create data/
        }else{

            //BOOST_LOG_SEV(logger,notification) << "Success to create DATA_PATH directory : " << p.string();

            // If fail to create data/STATION_YYYMMDD/
            if(!fs::create_directory(p1)){

                //BOOST_LOG_SEV(logger,fail) << "Unable to create STATION_YYYYMMDD directory : " << p1.string();
                return false;

            // If success to create data/STATION_YYYMMDD/
            }else{

                //BOOST_LOG_SEV(logger,notification) << "Success to create STATION_YYYYMMDD directory : " << p1.string();

                // If fail to create data/STATION_YYYMMDD/exposure/
                if(!fs::create_directory(p2)){

                    //BOOST_LOG_SEV(logger,critical) << "Unable to create stack directory : " << p2.string();
                    return false;

                // If success to create data/STATION_YYYMMDD/exposure/
                }else{

                    //BOOST_LOG_SEV(logger,notification) << "Success to create stack directory : " << p2.string();
                    return true;

                }
            }
        }
    }

    return true;
}
