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

Device::Device(string cfgPath) {

    mCam        = NULL;
    mCamID      = 0;
    mCfgPath    = cfgPath;
    mVideoFramesInput = false;

    Configuration cfg;

    if(!cfg.Load(cfgPath))
        throw "Fail to load parameters for device object from configuration file.";

    if(!cfg.Get("CAMERA_ID", mGenCamID))
        throw "Fail to get CAMERA_ID for device object.";

    if(!cfg.Get("ACQ_FPS", mFPS))
        throw "Fail to get ACQ_FPS for device object.";

    if(!cfg.Get("ACQ_NIGHT_EXPOSURE", mNightExposure))
        throw "Fail to get ACQ_NIGHT_EXPOSURE for device object.";

    if(!cfg.Get("ACQ_NIGHT_GAIN", mNightGain))
        throw "Fail to get ACQ_NIGHT_GAIN for device object.";

    if(!cfg.Get("ACQ_DAY_EXPOSURE", mDayExposure))
        throw "Fail to get ACQ_DAY_EXPOSURE for device object.";

    if(!cfg.Get("ACQ_DAY_GAIN", mDayGain))
        throw "Fail to get ACQ_DAY_GAIN for device object.";

    string acqBitDepth;
    cfg.Get("ACQ_BIT_DEPTH", acqBitDepth);
    EParser<CamBitDepth> camBitDepth;
    mBitDepth = camBitDepth.parseEnum("ACQ_BIT_DEPTH", acqBitDepth);

    if(!cfg.Get("ACQ_RES_CUSTOM_SIZE", mCustomSize))
        throw "Fail to get ACQ_RES_CUSTOM_SIZE for device object.";

    if(mCustomSize) {

        string res;
        if(!cfg.Get("ACQ_RES_SIZE", res))
            throw "Fail to get ACQ_BIT_DEPTH for device object.";

        string width = res.substr(0,res.find("x"));
        string height = res.substr(res.find("x")+1,string::npos);
        mSizeWidth = atoi(width.c_str());
        mSizeHeight = atoi(height.c_str());

    }else {

        mSizeWidth      = 640;
        mSizeHeight     = 480;

    }
}

Device::Device() {

    mBitDepth       = MONO_8;
    mNightExposure  = 0;
    mNightGain      = 0;
    mDayExposure    = 0;
    mDayGain        = 0;
    mFPS            = 30;
    mCamID          = 0;
    mGenCamID       = 0;
    mCustomSize     = true;
    mSizeWidth      = 640;
    mSizeHeight     = 480;
    mCam            = NULL;
    mVideoFramesInput = false;

}

Device::~Device(){

    if(mCam != NULL)
        delete mCam;

}

bool Device::createCamera(int id, bool create) {

    if(id >=0 && id < mDevices.size()) {

        // Create Camera object with the correct sdk.
        if(!createDevicesWith(mDevices.at(id).second.second))
            return false;

        mCamID = mDevices.at(id).first;

        if(mCam != NULL) {
            if(create) {
                if(!mCam->createDevice(mCamID)){
                    BOOST_LOG_SEV(logger, fail) << "Fail to create device with ID  : " << id;
                    mCam->grabCleanse();
                    return false;
                }
            }
            return true;
        }
    }

    BOOST_LOG_SEV(logger, fail) << "No device with ID " << id;
    cout << "No device with ID " << id << endl;

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
    cout << "No device with ID " << mGenCamID << endl;

    return false;

}

bool Device::createDevicesWith(CamSdkType sdk) {

    switch(sdk) {

        case VIDEOFILE :

            {
                Configuration cfg;
                if(!cfg.Load(mCfgPath)) {
                    BOOST_LOG_SEV(logger, fail) << "Can't load parameters from configuration file.";
                    cout << "Can't load parameters from configuration file." << endl;
                    return false;
                }

                // Get frames locations.
                string INPUT_VIDEO_PATH; cfg.Get("INPUT_VIDEO_PATH", INPUT_VIDEO_PATH);

                mVideoFramesInput = true;

                vector<string> videoList;

                typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                boost::char_separator<char> sep(",");
                tokenizer tokens(INPUT_VIDEO_PATH, sep);

                int n = 1;

                for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
                    videoList.push_back(*tok_iter);
                    cout << "VIDEO " << Conversion::intToString(n) << " : " << *tok_iter << endl;
                    n++;
                }

                // Create camera using pre-recorded video in input.
                if(videoList.size()!=0) {
                    mCam = new CameraVideo(videoList);
                    mCam->grabInitialization();
                }else{
                    BOOST_LOG_SEV(logger, fail) << "No video files found.";
                    cout << "No video files found." << endl;
                    return false;
                }
            }

            break;

        case FRAMESDIR :

            {
                Configuration cfg;
                if(!cfg.Load(mCfgPath)) {
                    BOOST_LOG_SEV(logger, fail) << "Can't load parameters from configuration file.";
                    cout << "Can't load parameters from configuration file." << endl;
                    return false;
                }

                // Get frames location.
                string INPUT_FRAMES_DIRECTORY_PATH; cfg.Get("INPUT_FRAMES_DIRECTORY_PATH", INPUT_FRAMES_DIRECTORY_PATH);
                BOOST_LOG_SEV(logger, normal) << "Read INPUT_FRAMES_DIRECTORY_PATH from configuration file : " << INPUT_FRAMES_DIRECTORY_PATH;

                mVideoFramesInput = true;

                vector<string> framesLocationList;

                typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                boost::char_separator<char> sep(",");
                tokenizer tokens(INPUT_FRAMES_DIRECTORY_PATH, sep);

                int n = 1;

                for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
                    framesLocationList.push_back(*tok_iter);
                    cout << "LOCATION 1 : " << Conversion::intToString(n) << " : " << *tok_iter << endl;
                    n++;
                }

                // Create camera using pre-recorded fits2D in input.
                if(framesLocationList.size()!=0) {
                    mCam = new CameraFrames(framesLocationList, 1);
                    if(!mCam->grabInitialization())
                        throw "Fail to prepare acquisition on the first frames directory.";
                }else {
                    BOOST_LOG_SEV(logger, fail) << "No frames directories found.";
                    cout << "No frames directories found." << endl;
                    return false;
                }
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
                    mCam = new CameraGigeAravis();
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

    }

    return true;

}

void Device::listDevices(bool printInfos) {

    int nbDev = 0, nbCam = 0;
    pair<int, CamSdkType> elem;             // general index to specify camera to use
    pair<int,pair<int,CamSdkType>> subElem; // index in a specific sdk
    vector<pair<int,string>> listCams;

    #ifdef WINDOWS

        // PYLONGIGE

        createDevicesWith(PYLONGIGE);
        listCams = mCam->getCamerasList();
        for(int i = 0; i < listCams.size(); i++) {
            elem.first = nbDev; elem.second = PYLONGIGE;
            subElem.first = listCams.at(i).first; subElem.second = elem;
            mDevices.push_back(subElem);
            if(printInfos) cout << "[" << nbDev << "]    " << listCams.at(i).second << endl;
            nbDev++;
        }
        delete mCam;

        // TIS

        createDevicesWith(TIS);
        listCams = mCam->getCamerasList();
        for(int i = 0; i < listCams.size(); i++) {
            elem.first = nbDev; elem.second = TIS;
            subElem.first = listCams.at(i).first; subElem.second = elem;
            mDevices.push_back(subElem);
            if(printInfos) cout << "[" << nbDev << "]    " << listCams.at(i).second << endl;
            nbDev++;
        }
        delete mCam;

        // WINDOWS

        createDevicesWith(VIDEOINPUT);
        listCams = mCam->getCamerasList();
        for(int i = 0; i < listCams.size(); i++) {

            // Avoid to list basler
            std::string::size_type pos1 = listCams.at(i).second.find("Basler");
            std::string::size_type pos2 = listCams.at(i).second.find("BASLER");
            if((pos1 != std::string::npos) || (pos2 != std::string::npos)) {
                //std::cout << "found \"words\" at position " << pos1 << std::endl;
            } else {
                elem.first = nbDev; elem.second = VIDEOINPUT;
                subElem.first = listCams.at(i).first; subElem.second = elem;
                mDevices.push_back(subElem);
                if(printInfos) cout << "[" << nbDev << "]    " << listCams.at(i).second << endl;
                nbDev++;
            }

        }
        delete mCam;

    #else

        // ARAVIS

        // V4L2

    #endif

    // VIDEO

    elem.first = nbDev; elem.second = VIDEOFILE;
    subElem.first = 0; subElem.second = elem;
    mDevices.push_back(subElem);
    if(printInfos) cout << "[" << nbDev << "]    VIDEO FILES" << endl;
    nbDev++;

    // FRAMES

    elem.first = nbDev; elem.second = FRAMESDIR;
    subElem.first = 0; subElem.second = elem;
    mDevices.push_back(subElem);
    if(printInfos) cout << "[" << nbDev << "]    FRAMES DIRECTORY" << endl;
    nbDev++;

    mCam = NULL;

}

bool Device::setCameraPixelFormat() {

    if(!mCam->setPixelFormat(mBitDepth)){
        mCam->grabCleanse();
        BOOST_LOG_SEV(logger,fail) << "Fail to set camera format.";
        return false;
    }

    return true;
}

bool Device::getCameraExposureBounds(double &min, double &max) {

    mCam->getExposureBounds(min, max);
    return true;
}

bool Device::getCameraGainBounds(int &min, int &max) {

    mCam->getGainBounds(min, max);
    return true;
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
    mCam->acqStart();
    return true;

}

bool Device::stopCamera() {

    BOOST_LOG_SEV(logger, notification) << "Stopping camera...";
    mCam->acqStop();
    mCam->grabCleanse();
    return true;

}

bool Device::runContinuousCapture(Frame &img) {
    double tacq = (double)getTickCount();
    if(mCam->grabImage(img)) {
        tacq = (((double)getTickCount() - tacq)/getTickFrequency())*1000;
        //std::cout << " >> [ TIME ACQ ] : " << tacq << " ms" << endl;
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
