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

#ifdef WINDOWS

CameraWindows::CameraWindows() {

    mVideoInput.setVerbose(false);
    mExposureAvailable = false;
    mGainAvailable = false;
    mFrameCounter = 0;
    mInputDeviceType = CAMERA;
    mDevNumber = -1;
    mBuffer = NULL;
}

CameraWindows::~CameraWindows()
{
    if(mBuffer != NULL)
        delete[] mBuffer;
}

vector<pair<int,string>> CameraWindows::getCamerasList() {

    vector<pair<int,string>> camerasList;

    int nbCamFound = mVideoInput.listDevices(true);

    if(nbCamFound > 0) {

         for(int i = 0; i < nbCamFound; i++) {

             pair<int,string> c;
             c.first = i;
             c.second = "NAME[" + string(mVideoInput.getDeviceName(i)) + "] SDK[VI]";
             camerasList.push_back(c);

         }

     }

    return camerasList;

}

bool CameraWindows::setSize(int width, int height, bool customSize) {

    if(customSize)
        return mVideoInput.setupDevice(mDevNumber,width,height);
    else
        return mVideoInput.setupDevice(mDevNumber,640,480);

}

 bool  CameraWindows::grabSingleImage(Frame &frame, int camID) {

    int numDevices = mVideoInput.listDevices(true);

    if(frame.mWidth > 0 && frame.mHeight > 0) {
        if(!mVideoInput.setupDevice(camID, frame.mWidth, frame.mHeight))
            return false;
    }else{
        if(!mVideoInput.setupDevice(camID, 640, 480))
            return false;
    }

    // As requested width and height can not always be accomodated make sure to check the size once the device is setup
    mWidth = mVideoInput.getWidth(camID);
    mHeight = mVideoInput.getHeight(camID);
    mSize = mVideoInput.getSize(camID);
    cout << ">> Size setted to : " << mWidth << "x" << mHeight << endl;

    // Create the buffer where the video will be captured
    mBuffer = new unsigned char[mSize];

    // Disable autofocus and set focus to 0
    // mVideoInput.setVideoSettingCamera(camID, CameraControl_Focus, mDefaultFocus, CameraControl_Flags_Manual);

    setPixelFormat(frame.mFormat);
    setExposureTime(frame.mExposure);
    setGain(frame.mGain);

    bool success = mVideoInput.getPixels(camID, mBuffer, false, true);

    if(success) {

        cv::Mat image( mHeight, mWidth, CV_8UC3, mBuffer );
        Mat img;
        cv::cvtColor(image, img, CV_BGR2GRAY);
        boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
        string acquisitionDate = to_iso_extended_string(time);
        frame = Frame(img, 0, 0, acquisitionDate);
        frame.mFps = 0;
        frame.mFormat = MONO8;
        frame.mSaturatedValue = 255;
        frame.mFrameNumber = 0;
        mVideoInput.stopDevice(camID);
        return true;

    }

    std::cout << "Error loading frame from camera (Windows)." << std::endl;
    mVideoInput.stopDevice(camID);
    return false;

};

bool  CameraWindows::createDevice(int id){

    mDevNumber = id;
    return true;

}

bool  CameraWindows::setPixelFormat(CamPixFmt format){
    cout << ">> (WARNING) Can't set format with VI." << endl;
    return true;
}

void  CameraWindows::getExposureBounds(double &eMin, double &eMax){
    eMin = -1;
    eMax = -1;
}

void  CameraWindows::getGainBounds(int &gMin, int &gMax){
    gMin = -1;
    gMax = -1;
}

bool  CameraWindows::getFPS(double &value){
    value = 0;
    return false;
}

bool  CameraWindows::setExposureTime(double value){
    cout << ">> (WARNING) Can't set exposure time with VI." << endl;
    return true;
}

bool  CameraWindows::setGain(int value){
    cout << ">> (WARNING) Can't set gain with VI." << endl;
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

bool  CameraWindows::acqStart(){return true;};

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
        newFrame.mFormat = MONO8;
        newFrame.mSaturatedValue = 255;
        newFrame.mFrameNumber = mFrameCounter;
        mFrameCounter++;
        return true;

    }

    std::cout << "Error loading frame from camera (Windows)." << std::endl;
    return false;

}

void  CameraWindows::acqStop(){
    mVideoInput.stopDevice(mDevNumber);
}

void  CameraWindows::grabCleanse(){

}

bool  CameraWindows::getPixelFormat(CamPixFmt &format){
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

#endif
