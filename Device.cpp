/*
                                Device.cpp

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
* \file    Device.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    02/09/2014
* \brief
*/

#include "Device.h"

boost::log::sources::severity_logger< LogSeverityLevel >  Device::logger;

Device::Init Device::initializer;

Device::Device(cameraParam cp, framesParam fp, videoParam vp, int cid) {

    mCam        = NULL;
    mCamID      = 0;
    mVerbose    = true;
    mNbDev      = -1;
    mVideoFramesInput = false;
    mGenCamID = cid;
    mFPS = cp.ACQ_FPS;
    mNightExposure = cp.ACQ_NIGHT_EXPOSURE;
    mNightGain = cp.ACQ_NIGHT_GAIN;
    mDayExposure = cp.ACQ_DAY_EXPOSURE;
    mDayGain = cp.ACQ_DAY_GAIN;
    mMinExposureTime = -1;
    mMaxExposureTime = -1;
    mMinGain = -1;
    mMaxGain = -1;
    mShiftBits = cp.SHIFT_BITS;
    mFormat = cp.ACQ_FORMAT;
    mCustomSize = cp.ACQ_RES_CUSTOM_SIZE;
    mSizeWidth = cp.ACQ_WIDTH;
    mSizeHeight = cp.ACQ_HEIGHT;
    mDeviceType = UNDEFINED_INPUT_TYPE;
    mvp = vp;
    mfp = fp;
    //mNbFrame = 0;
}

Device::Device() {

    mFormat         = MONO8;
    mNightExposure  = 0;
    mNightGain      = 0;
    mDayExposure    = 0;
    mDayGain        = 0;
    mMinExposureTime = -1;
    mMaxExposureTime = -1;
    mMinGain = -1;
    mMaxGain = -1;
    mFPS            = 30;
    mCamID          = 0;
    mGenCamID       = 0;
    mCustomSize     = false;
    mSizeWidth      = 640;
    mSizeHeight     = 480;
    mCam            = NULL;
    mVideoFramesInput = false;
    mShiftBits      = false;
    mNbDev          = -1;
    mVerbose        = true;
    mDeviceType     = UNDEFINED_INPUT_TYPE;
    vector<string> finput, vinput;
    mvp.INPUT_VIDEO_PATH = vinput;
    mfp.INPUT_FRAMES_DIRECTORY_PATH = finput;
    //mNbFrame = 0;

}

Device::~Device(){

    if(mCam != NULL)
        delete mCam;

}

bool Device::createCamera(int id, bool create) {

    if(id >=0 && id < mDevices.size()) {

        // Create Camera object with the correct sdk.
        if(!createDevicesWith(mDevices.at(id).second.second)){
            cout << "Fail to select correct sdk : "<< mDevices.at(id).second.second << endl;
            return false;
        }

        mCamID = mDevices.at(id).first;

        if(mCam != NULL) {
            if(create) {
                if(!mCam->createDevice(mCamID)){
                    BOOST_LOG_SEV(logger, fail) << "Fail to create device with ID  : " << id;
                    mCam->grabCleanse();
                    return false;
                }else{
                    //BOOST_LOG_SEV(logger, fail) << "Success to create device with ID  : " << id;
                }
            }
            return true;
        }
    }

    BOOST_LOG_SEV(logger, fail) << "No device with ID " << id;

    return false;

}

bool Device::createCamera() {

    if(mGenCamID >=0 && mGenCamID < mDevices.size()) {

        // Create Camera object with the correct sdk.
        if(!createDevicesWith(mDevices.at(mGenCamID).second.second))
            return false;

        mCamID = mDevices.at(mGenCamID).first;

        if(mCam != NULL) {
            if(!mCam->createDevice(mCamID)){
                BOOST_LOG_SEV(logger, fail) << "Fail to create device with ID  : " << mGenCamID;
                mCam->grabCleanse();
                return false;
            }
            return true;
        }

    }

    BOOST_LOG_SEV(logger, fail) << "No device with ID " << mGenCamID;

    return false;

}

bool Device::recreateCamera() {

    if(mGenCamID >=0 && mGenCamID < mDevices.size()) {

        mCamID = mDevices.at(mGenCamID).first;

        if(mCam != NULL) {
            if(!mCam->createDevice(mCamID)){
                BOOST_LOG_SEV(logger, fail) << "Fail to create device with ID  : " << mGenCamID;
                mCam->grabCleanse();
                return false;
            }
            return true;
        }

    }

    BOOST_LOG_SEV(logger, fail) << "No device with ID " << mGenCamID;

    return false;

}

void Device::setVerbose(bool status) {

    mVerbose = status;

}

CamSdkType Device::getDeviceSdk(int id){

    if(id >=0 && id < mDevices.size()) {
        return mDevices.at(id).second.second;
    }

    return UNKNOWN;

}

bool Device::createDevicesWith(CamSdkType sdk) {

    switch(sdk) {

        case VIDEOFILE :

            {
                mVideoFramesInput = true;
                mCam = new CameraVideo(mvp.INPUT_VIDEO_PATH, mVerbose);
                mCam->grabInitialization();
            }

            break;

        case FRAMESDIR :

            {

                mVideoFramesInput = true;
                // Create camera using pre-recorded fits2D in input.
                mCam = new CameraFrames(mfp.INPUT_FRAMES_DIRECTORY_PATH, 1, mVerbose);
                if(!mCam->grabInitialization())
                    throw "Fail to prepare acquisition on the first frames directory.";

            }

            break;

        case V4L2 :

            {
                #ifdef LINUX
                    mCam = new CameraV4l2();
                #endif
            }

            break;

        case VIDEOINPUT :

            {
                #ifdef WINDOWS
                    mCam = new CameraWindows();
                #endif
            }

            break;

        case ARAVIS :

            {
                #ifdef LINUX
                    mCam = new CameraGigeAravis(mShiftBits);
                #endif
            }

            break;

        case PYLONGIGE :

            {
                #ifdef WINDOWS
                    mCam = new CameraGigePylon();
                #endif
            }

            break;

        case TIS :

            {
                #ifdef WINDOWS
                    mCam = new CameraGigeTis();
                #endif
            }

            break;

        default :

            cout << "Unknown sdk." << endl;

    }

    return true;

}

InputDeviceType Device::getDeviceType(CamSdkType t) {

    switch(t){

        case VIDEOFILE :
            return VIDEO;
            break;

        case FRAMESDIR :
            return SINGLE_FITS_FRAME;
            break;

        case V4L2 :
        case VIDEOINPUT :
        case ARAVIS :
        case PYLONGIGE :
        case TIS :
            return CAMERA;
            break;

        case UNKNOWN :
                return UNDEFINED_INPUT_TYPE;
            break;
    }

    return UNDEFINED_INPUT_TYPE;
}

void Device::listDevices(bool printInfos) {

    int nbCam = 0;
    mNbDev = 0;
    pair<int, CamSdkType> elem;             // general index to specify camera to use
    pair<int,pair<int,CamSdkType>> subElem; // index in a specific sdk
    vector<pair<int,string>> listCams;

    #ifdef WINDOWS

        // PYLONGIGE

        mCam = new CameraGigePylon();
        listCams = mCam->getCamerasList();
        for(int i = 0; i < listCams.size(); i++) {
            elem.first = mNbDev; elem.second = PYLONGIGE;
            subElem.first = listCams.at(i).first; subElem.second = elem;
            mDevices.push_back(subElem);
            if(printInfos) cout << "[" << mNbDev << "]    " << listCams.at(i).second << endl;
            mNbDev++;
        }
        delete mCam;

        // TIS

        mCam = new CameraGigeTis();
        listCams = mCam->getCamerasList();
        for(int i = 0; i < listCams.size(); i++) {
            elem.first = mNbDev; elem.second = TIS;
            subElem.first = listCams.at(i).first; subElem.second = elem;
            mDevices.push_back(subElem);
            if(printInfos) cout << "[" << mNbDev << "]    " << listCams.at(i).second << endl;
            mNbDev++;
        }
        delete mCam;

        // WINDOWS

        mCam = new CameraWindows();
        listCams = mCam->getCamerasList();
        for(int i = 0; i < listCams.size(); i++) {

            // Avoid to list basler
            std::string::size_type pos1 = listCams.at(i).second.find("Basler");
            std::string::size_type pos2 = listCams.at(i).second.find("BASLER");
            if((pos1 != std::string::npos) || (pos2 != std::string::npos)) {
                //std::cout << "found \"words\" at position " << pos1 << std::endl;
            } else {
                elem.first = mNbDev; elem.second = VIDEOINPUT;
                subElem.first = listCams.at(i).first; subElem.second = elem;
                mDevices.push_back(subElem);
                if(printInfos) cout << "[" << mNbDev << "]    " << listCams.at(i).second << endl;
                mNbDev++;
            }
        }

        delete mCam;

    #else

        // ARAVIS

        createDevicesWith(ARAVIS);
        listCams = mCam->getCamerasList();
        for(int i = 0; i < listCams.size(); i++) {
            elem.first = mNbDev; elem.second = ARAVIS;
            subElem.first = listCams.at(i).first; subElem.second = elem;
            mDevices.push_back(subElem);
            if(printInfos) cout << "[" << mNbDev << "]    " << listCams.at(i).second << endl;
            mNbDev++;
        }
        delete mCam;

        // V4L2

        createDevicesWith(V4L2);
        listCams = mCam->getCamerasList();
        for(int i = 0; i < listCams.size(); i++) {
            elem.first = mNbDev; elem.second = V4L2;
            subElem.first = listCams.at(i).first; subElem.second = elem;
            mDevices.push_back(subElem);
            if(printInfos) cout << "[" << mNbDev << "]    " << listCams.at(i).second << endl;
            mNbDev++;
        }
        delete mCam;

    #endif

    // VIDEO

    elem.first = mNbDev; elem.second = VIDEOFILE;
    subElem.first = 0; subElem.second = elem;
    mDevices.push_back(subElem);
    if(printInfos) cout << "[" << mNbDev << "]    VIDEO FILES" << endl;
    mNbDev++;

    // FRAMES

    elem.first = mNbDev; elem.second = FRAMESDIR;
    subElem.first = 0; subElem.second = elem;
    mDevices.push_back(subElem);
    if(printInfos) cout << "[" << mNbDev << "]    FRAMES DIRECTORY" << endl;
    mNbDev++;

    mCam = NULL;

}

bool Device::getDeviceName() {

    return mCam->getCameraName();

}

bool Device::setCameraPixelFormat() {

    if(!mCam->setPixelFormat(mFormat)){
        mCam->grabCleanse();
        BOOST_LOG_SEV(logger,fail) << "Fail to set camera format.";
        return false;
    }

    return true;
}

bool Device::getSupportedPixelFormats() {


    mCam->getAvailablePixelFormats();
    return true;

}

InputDeviceType Device::getDeviceType() {

    return mCam->getDeviceType();

}

bool Device::getCameraExposureBounds(double &min, double &max) {

    mCam->getExposureBounds(min, max);
    return true;
}

void Device::getCameraExposureBounds() {

    mCam->getExposureBounds(mMinExposureTime, mMaxExposureTime);

}

bool Device::getCameraGainBounds(int &min, int &max) {

    mCam->getGainBounds(min, max);
    return true;
}

void Device::getCameraGainBounds() {

    mCam->getGainBounds(mMinGain, mMaxGain);

}

bool Device::setCameraNightExposureTime() {

    if(!mCam->setExposureTime(mNightExposure)) {
        BOOST_LOG_SEV(logger, fail) << "Fail to set night exposure time to " << mNightExposure;
        mCam->grabCleanse();
        return false;
    }

    return true;

}

bool Device::setCameraDayExposureTime() {

    if(!mCam->setExposureTime(mDayExposure)) {
        BOOST_LOG_SEV(logger, fail) << "Fail to set day exposure time to " << mDayExposure;
        mCam->grabCleanse();
        return false;
    }

    return true;

}

bool Device::setCameraNightGain() {

    if(!mCam->setGain(mNightGain)) {
        BOOST_LOG_SEV(logger, fail) << "Fail to set night gain to " << mNightGain;
        mCam->grabCleanse();
        return false;
    }

    return true;

}

bool Device::setCameraDayGain() {

    if(!mCam->setGain(mDayGain)) {
        BOOST_LOG_SEV(logger, fail) << "Fail to set day gain to " << mDayGain;
        mCam->grabCleanse();
        return false;
    }

    return true;

}

bool Device::setCameraExposureTime(double value) {

    if(!mCam->setExposureTime(value)) {
        BOOST_LOG_SEV(logger, fail) << "Fail to set exposure time to " << value;
        mCam->grabCleanse();
        return false;
    }

    return true;

}

bool Device::setCameraGain(int value) {

    if(!mCam->setGain(value)) {
        BOOST_LOG_SEV(logger, fail) << "Fail to set gain to " << value;
        mCam->grabCleanse();
        return false;
    }

    return true;

}

bool Device::setCameraFPS() {

    if(!mCam->setFPS(mFPS)) {
        BOOST_LOG_SEV(logger, fail) << "Fail to set FPS to " << mFPS;
        mCam->grabCleanse();
        return false;
    }

    return true;

}

bool Device::initializeCamera() {

    if(!mCam->grabInitialization()){
        BOOST_LOG_SEV(logger, fail) << "Fail to initialize camera.";
        mCam->grabCleanse();
        return false;
    }

    return true;

}

bool Device::startCamera() {

    BOOST_LOG_SEV(logger, notification) << "Starting camera...";
    if(!mCam->acqStart())
        return false;

    return true;

}

bool Device::stopCamera() {

    BOOST_LOG_SEV(logger, notification) << "Stopping camera...";
    mCam->acqStop();
    mCam->grabCleanse();
    return true;

}

bool Device::runContinuousCapture(Frame &img) {

    if(mCam->grabImage(img)) {
        //img.mFrameNumber = mNbFrame;
        //mNbFrame++;
        return true;
    }

    return false;

}

bool Device::runSingleCapture(Frame &img) {

    if(mCam->grabSingleImage(img, mCamID))
        return true;

    return false;

}

bool Device::setCameraSize() {

    if(!mCam->setSize(mSizeWidth, mSizeHeight, mCustomSize)) {
        BOOST_LOG_SEV(logger, fail) << "Fail to set camera size.";
        return false;
    }

    return true;

}

bool Device::setCameraSize(int w, int h) {

    if(!mCam->setSize(w, h, true)) {
        BOOST_LOG_SEV(logger, fail) << "Fail to set camera size.";
        return false;
    }

    return true;

}

bool Device::getCameraFPS(double &fps) {

    if(!mCam->getFPS(fps)) {
        //BOOST_LOG_SEV(logger, fail) << "Fail to get fps value from camera.";
        return false;
    }

    return true;

}

bool Device::getCameraStatus() {

    return mCam->getStopStatus();

}

bool Device::getCameraDataSetStatus() {

    return mCam->getDataSetStatus();

}

bool Device::loadNextCameraDataSet(string &location) {

    return mCam->loadNextDataSet(location);

}

bool Device::getExposureStatus() {

    return mCam->mExposureAvailable;

}

bool Device::getGainStatus() {

    return mCam->mGainAvailable;

}
