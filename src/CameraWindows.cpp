/*
                                CameraWindows.cpp

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
*   Last modified:      02/10/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    CameraWindows.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    02/10/2015
*/

#include "CameraWindows.h"

CameraWindows::CameraWindows() {

    mVideoInput.setVerbose(false);

}

CameraWindows::~CameraWindows()
{
    mVideoInput.stopDevice(mDevNumber);
    delete[] mBuffer;
}

 bool CameraWindows::listCameras() {

     int numDevices = mVideoInput.listDevices(true);

     cout << endl << "--------------- CAMERAS WITH DSHOW -------------" << endl << endl;

     if(numDevices > 0) {

         for(int i = 0; i < numDevices; i++) {

             cout << "-> [" << i << "] " << mVideoInput.getDeviceName(i) << endl;

         }

         cout << endl << "------------------------------------------------" << endl << endl;

         return true;

     }

     cout << "-> No cameras detected..." << endl;
     cout << endl << "------------------------------------------------" << endl << endl;

     return false;

 }

 bool  CameraWindows::grabSingleImage(Frame &frame, int camID) {
     
    listCameras();

    mVideoInput.setupDevice(camID);

    // As requested width and height can not always be accomodated make sure to check the size once the device is setup
    mWidth = mVideoInput.getWidth(camID);
    mHeight = mVideoInput.getHeight(camID);
    mSize = mVideoInput.getSize(camID);

    // Create the buffer where the video will be captured
    mBuffer = new unsigned char[mSize];

    // Disable autofocus and set focus to 0
    mVideoInput.setVideoSettingCamera(camID, CameraControl_Focus, mDefaultFocus, CameraControl_Flags_Manual);

    bool success = mVideoInput.getPixels(camID, mBuffer, false, true);

    if(success) {

        cv::Mat image( mHeight, mWidth, CV_8UC3, mBuffer );
        Mat img;
        cv::cvtColor(image, img, CV_BGR2GRAY);
        boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
        string acquisitionDate = to_iso_extended_string(time);
        frame = Frame(img, 0, 0, acquisitionDate);
        frame.mFps = 0;
        frame.mBitDepth = MONO_8;
        frame.mSaturatedValue = 255;
        frame.mFrameNumber = 0;

        return true;

    }

    std::cout << "Error loading frame from camera (Windows)." << std::endl;
    return false;

};

bool  CameraWindows::createDevice(int id){
    
    mVideoInput.setupDevice(id,640,480);
    mDevNumber = id;
    return true;

}

bool  CameraWindows::setPixelFormat(CamBitDepth format){
    return true;
}

void  CameraWindows::getExposureBounds(int &eMin, int &eMax){

}

void  CameraWindows::getGainBounds(int &gMin, int &gMax){

}

bool  CameraWindows::getFPS(double &value){
    value = 30;
    return true;
}

bool  CameraWindows::setExposureTime(double value){
    return true;
}

bool  CameraWindows::setGain(int value){
    return true;
}

bool  CameraWindows::setFPS(double value){
    
    // If you want to capture at a different frame rate (default is 30) specify it here, you are not guaranteed to get this fps though.
    // Call before setupDevice
    // directshow will try and get the closest possible framerate to what is requested
    mVideoInput.setIdealFramerate(mDevNumber, (int)value);

    return true;

}

bool  CameraWindows::setFpsToLowerValue(){
    return false;
}

bool  CameraWindows::grabInitialization() {

    // As requested width and height can not always be accomodated make sure to check the size once the device is setup
    mWidth = mVideoInput.getWidth(mDevNumber);
    mHeight = mVideoInput.getHeight(mDevNumber);
    mSize = mVideoInput.getSize(mDevNumber);
    cout << "Default size : " << mWidth << "x" << mHeight << endl;

    // Create the buffer where the video will be captured
    mBuffer = new unsigned char[mSize];

    // Disable autofocus and set focus to 0
    mVideoInput.setVideoSettingCamera(mDevNumber, CameraControl_Focus, mDefaultFocus, CameraControl_Flags_Manual);


    //long current_value,min_value,max_value,stepping_delta,flags,defaultValue; 

    //mVideoInput.getVideoSettingCamera(mDevNumber,mVideoInput.propBrightness ,min_value,max_value,stepping_delta,current_value,flags,defaultValue); 
    /*cout << "min: "<< min_value << endl;
    cout << "max: "<< max_value << endl;
    cout << "flags: "<< flags << endl;
    cout << "SteppingDelta: "<< stepping_delta << endl;
    cout << "currentValue: "<< current_value << endl;
    cout << "defaultValue: "<< defaultValue << endl;*/
    //mVideoInput.showSettingsWindow(mDevNumber);

    return true;
}

void  CameraWindows::acqStart(){};

bool  CameraWindows::grabImage(Frame &newFrame){
    
    bool success = mVideoInput.getPixels(mDevNumber, mBuffer, false, true);

    if(success) {

        cv::Mat image( mHeight, mWidth, CV_8UC3, mBuffer );
        Mat img;
        cv::cvtColor(image, img, CV_BGR2GRAY);
        boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
        string acquisitionDate = to_iso_extended_string(time);
        newFrame = Frame(img, 0, 0, acquisitionDate);
        newFrame.mFps = 0;
        newFrame.mBitDepth = MONO_8;
        newFrame.mSaturatedValue = 255;
        newFrame.mFrameNumber = 0;

        return true;

    }

    std::cout << "Error loading frame from camera (Windows)." << std::endl;
    return false;

}

void  CameraWindows::acqStop(){

}

void  CameraWindows::grabCleanse(){

}

bool  CameraWindows::getPixelFormat(CamBitDepth &format){
    return false;
}

// Return exposure time in seconds.
double  CameraWindows::getExposureTime() {

    long min = 0, max = 0, SteppingDelta = 0 , currentValue = 0, flags = 0, defaultValue = 0;

    // https://msdn.microsoft.com/en-us/library/dd318253(v=vs.85).aspx
    if(mVideoInput.getVideoSettingCamera(mDevNumber, CameraControl_Exposure, min, max, SteppingDelta, currentValue, flags, defaultValue)) {

        double e = 0.0;

        if(currentValue >= 0) {

            e = pow(2,currentValue);

        } else {

            e = 1.0 / pow(2,abs(currentValue));

        }

        return e;

    }

    return 0.0;

}

TimeMeasureUnit CameraWindows::getExposureUnit() {

    return SEC;

}