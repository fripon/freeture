/*
/*
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

Device::Device(CamType type, string cfgPath):
    mType(type), mCfgPath(cfgPath) {

    initialization();
}

Device::Device(CamType type):
    mType(type) {

    initialization();

}

Device::~Device(){

    if(cam != NULL) delete cam;

}

void Device::initialization() {

    mStationName            = "STATION";
    mDataPath               = "./";
    mScheduleEnabled        = false;
    mNightExposure          = 0;
    mNightGain              = 0;
    mDayExposure            = 0;
    mDayGain                = 0;
    mFPS                    = 1;
    mCamID                  = 0;
    mMaskEnabled            = false;
    mMaskPath               = "./";
    mEphemerisEnabled       = true;
    mExpCtrlSaveImg         = false;
    mExpCtrlSaveInfos       = false;
    mExpCtrlFrequency       = 300;
    mSunsetDuration         = 3600;
    mSunriseDuration        = 3600;
    mMinExposureTime        = 0;
    mMaxExposureTime        = 0;
    mMinGain                = 0;
    mMaxGain                = 0;
    mVideoFramesInInput     = false;
    mEphemerisUpdated       = false;

    mStartSunriseTime       = 0;
    mStopSunriseTime        = 0;
    mStartSunsetTime        = 0;
    mStopSunsetTime         = 0;

    switch(mType){

        case BASLER_GIGE :

            {
                #ifdef USE_PYLON

                    BOOST_LOG_SEV(logger, normal) << "INPUT : BASLER_GIGE -> Use Pylon";
                    cam = new CameraGigePylon();

                #else
                    #ifdef LINUX

                        BOOST_LOG_SEV(logger, normal) << "INPUT : BASLER_GIGE -> Use Aravis";
                        cam = new CameraGigeAravis();

                    #endif
                #endif
            }

            break;

        case DMK_GIGE :

            {

                #ifdef WINDOWS

                    BOOST_LOG_SEV(logger, normal) << "INPUT : DMK_GIGE -> Use Imaging Source";
                    cam = new CameraGigeTis();

                #else
                    #ifdef LINUX

                        BOOST_LOG_SEV(logger, normal) << "INPUT : DMK_GIGE -> Use Aravis";
                        cam = new CameraGigeAravis(true);

                    #endif
                #endif

            }

            break;

        case TYTEA_USB :

            {

                #ifdef WINDOWS

                    cout << "TYTEA is not supported on Windows." << endl;

                #else
                    #ifdef LINUX

                        //BOOST_LOG_SEV(logger, normal) << "INPUT : TYTEA_USB -> Use V4L2";
                        //cam = new CameraV4l2();
                        cout << "TYTEA is not supported on Linux." << endl;
                        cam = NULL;

                    #endif
                #endif

            }

            break;

        default :

            cout << "Type of camera not supported." << endl;
            cam = NULL;

    }

}

bool Device::prepareDevice() {

    try {

        Configuration cfg;
        cfg.Load(mCfgPath);

        // Get detection option status.
        cfg.Get("DETECTION_ENABLED", mDetectionEnabled);

        switch(mType){

            case FRAMES :

                {

                    // Get frames location.
                    string INPUT_FRAMES_DIRECTORY_PATH; cfg.Get("INPUT_FRAMES_DIRECTORY_PATH", INPUT_FRAMES_DIRECTORY_PATH);
                    BOOST_LOG_SEV(logger, normal) << "Read INPUT_FRAMES_DIRECTORY_PATH from configuration file : " << INPUT_FRAMES_DIRECTORY_PATH;

                    mVideoFramesInInput = true;

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
                    cam = new CameraFrames(framesLocationList, 1);
                    if(!cam->grabInitialization())
                        throw "Fail to prepare acquisition on the first frames directory.";

                }

                break;

            case VIDEO :

                {
                    // Get frames locations.
                    string INPUT_VIDEO_PATH; cfg.Get("INPUT_VIDEO_PATH", INPUT_VIDEO_PATH);

                    mVideoFramesInInput = true;

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
                    cam = new CameraVideo(videoList);
                    cam->grabInitialization();
                }

                break;

            default :

                // CAMERA_ID.
                cfg.Get("CAMERA_ID", mCamID);
                BOOST_LOG_SEV(logger, notification) << "CAMERA_ID : " << mCamID;

                // Get data location.
                cfg.Get("DATA_PATH", mDataPath);
                BOOST_LOG_SEV(logger, notification) << "DATA_PATH : " << mDataPath;

                // Get station name.
                cfg.Get("STATION_NAME", mStationName);
                BOOST_LOG_SEV(logger, notification) << "STATION_NAME : " << mStationName;

                // Get acquisition format.
                string acq_bit_depth; cfg.Get("ACQ_BIT_DEPTH", acq_bit_depth);
                EParser<CamBitDepth> cam_bit_depth;
                mBitDepth = cam_bit_depth.parseEnum("ACQ_BIT_DEPTH", acq_bit_depth);
                BOOST_LOG_SEV(logger, notification) << "ACQ_BIT_DEPTH : " << acq_bit_depth;

                // Get night exposure time.
                cfg.Get("ACQ_NIGHT_EXPOSURE", mNightExposure);
                BOOST_LOG_SEV(logger, notification) << "ACQ_NIGHT_EXPOSURE : " << mNightExposure;

                // Get night gain.
                cfg.Get("ACQ_NIGHT_GAIN", mNightGain);
                BOOST_LOG_SEV(logger, notification) << "ACQ_NIGHT_GAIN : " << mNightGain;

                // Get day exposure time.
                cfg.Get("ACQ_DAY_EXPOSURE", mDayExposure);
                BOOST_LOG_SEV(logger, notification) << "ACQ_DAY_EXPOSURE : " << mDayExposure;

                // Get day gain.
                cfg.Get("ACQ_DAY_GAIN", mDayGain);
                BOOST_LOG_SEV(logger, notification) << "ACQ_DAY_GAIN : " << mDayGain;

                // Get detection mode.
                string detection_mode;
                cfg.Get("DET_MODE", detection_mode);
                BOOST_LOG_SEV(logger, notification) << "DET_MODE : " << detection_mode;
                EParser<TimeMode> detMode;
                mDetectionMode = detMode.parseEnum("DET_MODE", detection_mode);

                // Get stack mode.
                string stack_mode;
                cfg.Get("STACK_MODE", stack_mode);
                BOOST_LOG_SEV(logger, notification) << "STACK_MODE : " << stack_mode;
                EParser<TimeMode> stackMode;
                mStackMode = stackMode.parseEnum("STACK_MODE", stack_mode);

                // Get schedule option status.
                cfg.Get("ACQ_SCHEDULE_ENABLED", mScheduleEnabled);
                BOOST_LOG_SEV(logger, notification) << "ACQ_SCHEDULE_ENABLED : " << mScheduleEnabled;

                // Get schedule acquisition ouput type.
                string sOutput;
                cfg.Get("ACQ_SCHEDULE_OUTPUT", sOutput);
                BOOST_LOG_SEV(logger, notification) << "ACQ_SCHEDULE_OUTPUT : " << sOutput;
                EParser<ImgFormat> sOutput1;
                mScheduleOutput = sOutput1.parseEnum("ACQ_SCHEDULE_OUTPUT", sOutput);

                 // Get regular acquisition option status.
                cfg.Get("ACQ_REGULAR_ENABLED", mRegularAcqEnabled);
                BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_ENABLED : " << mRegularAcqEnabled;

                if(mScheduleEnabled && mRegularAcqEnabled){

                    throw "Check configuration file : \n \"You can enable ACQ_SCHEDULE_ENABLED or ACQ_REGULAR_ENABLED (not both)\"\n";

                }

                // Get regular acquisition mode.
                string regular_mode;
                cfg.Get("ACQ_REGULAR_MODE", regular_mode);
                BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_MODE : " << regular_mode;
                EParser<TimeMode> regMode;
                mRegularMode = regMode.parseEnum("ACQ_REGULAR_MODE", regular_mode);

                string regAcqParam;
                cfg.Get("ACQ_REGULAR_CFG", regAcqParam);
                std::transform(regAcqParam.begin(), regAcqParam.end(),regAcqParam.begin(), ::toupper);

                typedef boost::tokenizer<boost::char_separator<char> > tokenizer1;
                boost::char_separator<char> sep1("HMSEGFN");
                tokenizer1 tokens1(regAcqParam, sep1);

                vector<string> res1;

                for(tokenizer1::iterator tokIter = tokens1.begin();tokIter != tokens1.end(); ++tokIter){

                    res1.push_back(*tokIter);

                }

                if(res1.size() == 7) {

                    // Get regular acquisition time interval.
                    mRegularInterval = atoi(res1.at(0).c_str()) * 3600 + atoi(res1.at(1).c_str()) * 60 + atoi(res1.at(2).c_str());

                    // Get regular acquisition exposure time.
                    mRegularExposure = atoi(res1.at(3).c_str());

                    // Get regular acquisition gain.
                    mRegularGain = atoi(res1.at(4).c_str());

                    // Get regular acquisition repetition.
                    mRegularRepetition = atoi(res1.at(6).c_str());

                    // Get regular acquisition format.
                    EParser<CamBitDepth> format;
                    Conversion::intBitDepthToCamBitDepthEnum(atoi(res1.at(5).c_str()), mRegularFormat);

                    /*cout << "ACQ REGULAR : [each " << mRegularInterval << " sec.] ["
                         << mRegularExposure << " exp.] "
                         << endl << "              ["
                         << mRegularGain << " gain] ["
                         << mRegularRepetition << " rep.] ["
                         << format.getStringEnum(mRegularFormat) << " format]" << endl;
                         */
                    BOOST_LOG_SEV(logger, notification) << "ACQ REGULAR : ";
                    BOOST_LOG_SEV(logger, notification) << "  - Each : " << mRegularInterval << " seconds.";
                    BOOST_LOG_SEV(logger, notification) << "  - " << mRegularExposure << " exposure time.";
                    BOOST_LOG_SEV(logger, notification) << "  - " << mRegularGain << " gain.";
                    BOOST_LOG_SEV(logger, notification) << "  - " << mRegularRepetition << " repetition.";
                    BOOST_LOG_SEV(logger, notification) << "  - " << format.getStringEnum(mRegularFormat) << " format.";

                }

                // Get regular acquisition ouput type.
                string rOutput;
                cfg.Get("ACQ_REGULAR_OUTPUT", rOutput);
                BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_OUTPUT : " << rOutput;
                EParser<ImgFormat> rOutput1;
                mRegularOutput = rOutput1.parseEnum("ACQ_REGULAR_OUTPUT", rOutput);

                // Get ephemeris option.
                cfg.Get("EPHEMERIS_ENABLED", mEphemerisEnabled);

                if(mEphemerisEnabled) {

                    //cout << "EPHEMERIS   :  ON" << endl;
                    BOOST_LOG_SEV(logger, notification) << "EPHEMERIS_ENABLED : ON";

                }else {

                    //cout << "EPHEMERIS   :  OFF" << endl;
                    BOOST_LOG_SEV(logger, notification) << "EPHEMERIS_ENABLED : OFF";

                }

                if(!mEphemerisEnabled) {

                    // Get sunrise time.
                    string sunrise_time;
                    cfg.Get("SUNRISE_TIME", sunrise_time);
                    BOOST_LOG_SEV(logger, notification) << "SUNRISE_TIME : " << sunrise_time;

                    {

                        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                        boost::char_separator<char> sep(":");
                        tokenizer tokens(sunrise_time, sep);

                        for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){

                            mSunriseTime.push_back(atoi((*tok_iter).c_str()));

                        }
                    }

                    // Get sunset time.
                    string sunset_time;
                    cfg.Get("SUNSET_TIME", sunset_time);
                    BOOST_LOG_SEV(logger, notification) << "SUNSET_TIME : " << sunset_time;

                    {

                        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                        boost::char_separator<char> sep(":");
                        tokenizer tokens(sunset_time, sep);

                        for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){

                            mSunsetTime.push_back(atoi((*tok_iter).c_str()));

                        }
                    }

                }else {

                    // Get sun height on the horizon.
                    cfg.Get("SUN_HORIZON_1", mSunHorizon1);
                    BOOST_LOG_SEV(logger, notification) << "SUN_HORIZON_1 : " << mSunHorizon1;

                    cfg.Get("SUN_HORIZON_2", mSunHorizon2);
                    BOOST_LOG_SEV(logger, notification) << "SUN_HORIZON_2 : " << mSunHorizon2;

                    // Get latitude.
                    cfg.Get("SITELAT", mStationLatitude);

                    // Get longitude.
                    cfg.Get("SITELONG", mStationLongitude);

                }

                // Get sunrise duration.
                cfg.Get("SUNRISE_DURATION", mSunriseDuration);
                BOOST_LOG_SEV(logger, notification) << "SUNRISE_DURATION : " << mSunriseDuration;

                 // Get sunset duration.
                cfg.Get("SUNSET_DURATION", mSunsetDuration);
                BOOST_LOG_SEV(logger, notification) << "SUNSET_DURATION : " << mSunsetDuration;

                // Get acquisition FPS.
                cfg.Get("ACQ_FPS", mFPS);
                BOOST_LOG_SEV(logger, notification) << "ACQ_FPS : " << mFPS;
                if(mFPS <= 0) mFPS = 1;

                // Get exposure control save infos option.
                cfg.Get("EXPOSURE_CONTROL_SAVE_INFOS", mExpCtrlSaveInfos);
                BOOST_LOG_SEV(logger, notification) << "EXPOSURE_CONTROL_SAVE_INFOS : " << mExpCtrlSaveInfos;

                // Get exposure control save image option.
                cfg.Get("EXPOSURE_CONTROL_SAVE_IMAGE", mExpCtrlSaveImg);
                BOOST_LOG_SEV(logger, notification) << "EXPOSURE_CONTROL_SAVE_IMAGE : " << mExpCtrlSaveImg;

                cfg.Get("EXPOSURE_CONTROL_FREQUENCY", mExpCtrlFrequency);
                BOOST_LOG_SEV(logger, notification) << "EXPOSURE_CONTROL_FREQUENCY : " << mExpCtrlFrequency;
                mExpCtrlFrequency = mExpCtrlFrequency * mFPS;

                if(mScheduleEnabled){

                    string sACQ_SCHEDULE;
                    cfg.Get("ACQ_SCHEDULE", sACQ_SCHEDULE);

                    vector<string> sch1;

                    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                    boost::char_separator<char> sep(",");
                    tokenizer tokens(sACQ_SCHEDULE, sep);

                    int n = 1;
                    BOOST_LOG_SEV(logger, notification) << "SCHEDULE : ";
                    for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
                        string s = *tok_iter;
                        std::transform(s.begin(), s.end(),s.begin(), ::toupper);
                        sch1.push_back(s);
                        cout << "-> " << Conversion::intToString(n) << " - " << s << endl;
                        BOOST_LOG_SEV(logger, notification) << "-> " << Conversion::intToString(n) << " - " << s;
                        n++;
                    }

                    //23h25m00s10000000e400g12f1n
                    for(int i = 0; i < sch1.size(); i++){

                        typedef boost::tokenizer<boost::char_separator<char> > tokenizer_;
                        boost::char_separator<char> sep_("HMSEGFN");
                        tokenizer tokens_(sch1.at(i), sep_);

                        vector<string> sp;

                        for(tokenizer::iterator tok_iter_ = tokens_.begin();tok_iter_ != tokens_.end(); ++tok_iter_){

                        sp.push_back(*tok_iter_);

                        }

                        if(sp.size() == 7){

                            AcqSchedule r = AcqSchedule(atoi(sp.at(0).c_str()), atoi(sp.at(1).c_str()), atoi(sp.at(2).c_str()), atoi(sp.at(3).c_str()), atoi(sp.at(4).c_str()), atoi(sp.at(5).c_str()), atoi(sp.at(6).c_str()));

                            int scheduledTimeInSec = atoi(sp.at(0).c_str()) * 3600 + atoi(sp.at(1).c_str()) * 60 + atoi(sp.at(2).c_str());

                            mSchedule.push_back(r);

                        }
                    }
                }

                // Get use mask option.
                cfg.Get("ACQ_MASK_ENABLED", mMaskEnabled);
                BOOST_LOG_SEV(logger, notification) << "ACQ_MASK_ENABLED : " << mMaskEnabled;

                if(mMaskEnabled){

                    cfg.Get("ACQ_MASK_PATH", mMaskPath);
                    BOOST_LOG_SEV(logger, notification) << "ACQ_MASK_PATH : " << mMaskPath;

                    mMask = imread(mMaskPath, CV_LOAD_IMAGE_GRAYSCALE);

                    if(!mMask.data){

                        BOOST_LOG_SEV(logger, fail) << " Can't load the mask from this location : " << mMaskPath;
                        throw "Failed to load the mask";

                    }

                }

                BOOST_LOG_SEV(logger, notification) << "Loading fits keywords...";
                mFitsHeader.loadKeywordsFromConfigFile(mCfgPath);

                if(cam != NULL)
                    runContinuousAcquisition();
                else
                    return false;

        }

    }catch(exception& e){

        cout << e.what() << endl;
        return false;

    }catch(const char * msg){

        cout << msg << endl;
        return false;

    }

    return true;
}

bool Device::getSunTimes() {

    int sunriseStartH = 0, sunriseStartM = 0, sunriseStopH = 0, sunriseStopM = 0,
        sunsetStartH = 0, sunsetStartM = 0, sunsetStopH = 0, sunsetStopM = 0;

    boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
    string date = to_iso_extended_string(time);
    vector<int> intDate = TimeDate::getIntVectorFromDateString(date);

    string month = Conversion::intToString(intDate.at(1));
    if(month.size() == 1) month = "0" + month;
    string day = Conversion::intToString(intDate.at(2));
    if(day.size() == 1) day = "0" + day;
    mCurrentDate = Conversion::intToString(intDate.at(0)) + month + day;
    mCurrentTime = intDate.at(3) * 3600 + intDate.at(4) * 60 + intDate.at(5);

    cout << "LOCAL DATE      :  " << mCurrentDate << endl;

    if(mEphemerisEnabled) {

        Ephemeris ephem1 = Ephemeris(mCurrentDate, mSunHorizon1, mStationLongitude, mStationLatitude );

        if(!ephem1.computeEphemeris(sunriseStartH, sunriseStartM,sunsetStopH, sunsetStopM)) {

            return false;

        }

        Ephemeris ephem2 = Ephemeris(mCurrentDate, mSunHorizon2, mStationLongitude, mStationLatitude );

        if(!ephem2.computeEphemeris(sunriseStopH, sunriseStopM,sunsetStartH, sunsetStartM)) {

            return false;

        }

    }else {

        sunriseStartH = mSunriseTime.at(0);
        sunriseStartM = mSunriseTime.at(1);

        double intpart1 = 0;
        double fractpart1 = modf((double)mSunriseDuration/3600.0 , &intpart1);

        if(intpart1!=0) {

            if(sunriseStartH + intpart1 < 24) {

                sunriseStopH = sunriseStartH + intpart1;

            }else {

                sunriseStopH = sunriseStartH + intpart1 - 24;

            }

        }

        double intpart2 = 0;
        double fractpart2 = modf(fractpart1 * 60 , &intpart2);

        if(sunriseStopM + intpart2 < 60) {

            sunriseStopM = sunriseStartM + intpart2;

        }else {


            if(sunriseStopH + 1 < 24) {

                sunriseStopH += 1;

            }else {

                sunriseStopH = sunriseStopH + 1 - 24;

            }


            sunriseStopM = intpart2;

        }

        sunsetStartH = mSunsetTime.at(0);
        sunsetStartM = mSunsetTime.at(1);

        double intpart3 = 0;
        double fractpart3 = modf((double)mSunsetDuration/3600.0 , &intpart3);

        if(intpart3!=0) {

            if(sunsetStartH + intpart3 < 24) {

                sunsetStopH = sunsetStartH + intpart3;

            }else {

                sunsetStopH = sunsetStartH + intpart3 - 24;

            }

        }

        double intpart4 = 0;
        double fractpart4 = modf(fractpart3 * 60 , &intpart4);

        if(sunsetStopM + intpart4 < 60) {

            sunsetStopM = sunsetStartM + intpart4;

        }else {


            if(sunsetStopH + 1 < 24) {

                sunsetStopH += 1;

            }else {

                sunsetStopH = sunsetStopH + 1 - 24;

            }

            sunsetStopM = intpart4;

        }

    }

    cout << "SUNRISE         :  " << sunriseStartH << "H" << sunriseStartM << " - " << sunriseStopH << "H" << sunriseStopM << endl;
    cout << "SUNSET          :  " << sunsetStartH << "H" << sunsetStartM << " - " << sunsetStopH << "H" << sunsetStopM << endl;

    mStartSunriseTime = sunriseStartH * 3600 + sunriseStartM * 60;
    mStopSunriseTime = sunriseStopH * 3600 + sunriseStopM * 60;
    mStartSunsetTime = sunsetStartH * 3600 + sunsetStartM * 60;
    mStopSunsetTime = sunsetStopH * 3600 + sunsetStopM * 60;

    return true;

}

void Device::runContinuousAcquisition(){

    /// List Gige Camera to check the ID.
    BOOST_LOG_SEV(logger, notification) << "Printing Connected Gige Camera...";
    if(!cam->listGigeCameras()){
        BOOST_LOG_SEV(logger, fail) << "No cameras.";
        throw "";
    }

    /// Create camera according to its ID.
    BOOST_LOG_SEV(logger, notification) << "Create device with ID  : " << mCamID;
    if(!cam->createDevice(mCamID)){
        BOOST_LOG_SEV(logger, fail) << "Fail to create device with ID  : " << mCamID;
        cam->grabCleanse();
        throw ">> Fail to create device.";
    }

    /// Set camera format.
    BOOST_LOG_SEV(logger, notification) << "Setting acquisition format...";
    if(!cam->setPixelFormat(mBitDepth)){
        cam->grabCleanse();
        throw ">> Fail to set Format.";
    }

    cam->getExposureBounds(mMinExposureTime, mMaxExposureTime);
    BOOST_LOG_SEV(logger, notification) << "Get camera exposure bounds : " << mMinExposureTime << " - " << mMaxExposureTime;

    cam->getGainBounds(mMinGain, mMaxGain);
    BOOST_LOG_SEV(logger, notification) << "Get camera gain bounds : " << mMinGain << " - " << mMaxGain;

    // Sunrise start/stop, Sunset start/stop.
    getSunTimes();

    // Check sunrise and sunset time.
    if((mCurrentTime > mStopSunsetTime) || (mCurrentTime < mStartSunriseTime)){

        BOOST_LOG_SEV(logger, notification) << "DAYTIME         :  NO";
        BOOST_LOG_SEV(logger, notification) << "AUTO EXPOSURE   :  NO";
        BOOST_LOG_SEV(logger, notification) << "EXPOSURE TIME   :  " << mNightExposure;
        BOOST_LOG_SEV(logger, notification) << "GAIN            :  " << mNightGain;

        /// Set camera exposure time.
        if(!cam->setExposureTime(mNightExposure)){
            cam->grabCleanse();
            throw ">> Fail to set Night Exposure.";
        }
        /// Set camera gain.
        if(!cam->setGain(mNightGain)){
            cam->grabCleanse();
            throw ">> Fail to set Night Gain.";
        }

    }else if((mCurrentTime > mStopSunriseTime && mCurrentTime < mStartSunsetTime)){

        BOOST_LOG_SEV(logger, notification) << "DAYTIME         :  YES";
        BOOST_LOG_SEV(logger, notification) << "AUTO EXPOSURE   :  NO";
        BOOST_LOG_SEV(logger, notification) << "EXPOSURE TIME   :  " << mDayExposure;
        BOOST_LOG_SEV(logger, notification) << "GAIN            :  " << mDayGain;

        /// Set camera exposure time.
        if(!cam->setExposureTime(mDayExposure)){
            cam->grabCleanse();
            throw ">> Fail to set Day Exposure.";
        }

        /// Set camera gain.
        if(!cam->setGain(mDayGain)){
            cam->grabCleanse();
            throw ">> Fail to set Day Gain.";
        }

    }else{

        BOOST_LOG_SEV(logger, notification) << "DAYTIME         :  NO";
        BOOST_LOG_SEV(logger, notification) << "AUTO EXPOSURE   :  YES";
        BOOST_LOG_SEV(logger, notification) << "EXPOSURE TIME   :  Minimum (" << mMinExposureTime << ")"<< mNightExposure;
        BOOST_LOG_SEV(logger, notification) << "GAIN            :  Minimum (" << mMinGain << ")";

        if(!cam->setExposureTime(mMinExposureTime)){
            cam->grabCleanse();
            throw ">> Fail to set Day Exposure.";
        }

        /// Set camera gain.
        if(!cam->setGain(mMinGain)){
            cam->grabCleanse();
            throw ">> Fail to set Day Gain.";
        }

    }

    /// Set camera fps.
    BOOST_LOG_SEV(logger, notification) << "Setting fps to " << mFPS;
    if(!cam->setFPS(mFPS)){
        cam->grabCleanse();
        throw ">> Fail to set Fps.";
    }

    /// Prepare grabbing.
    BOOST_LOG_SEV(logger, notification) << "Preparing camera to continuous acquisition...";
    if(!cam->grabInitialization()){
        cam->grabCleanse();
        throw ">> Fail to prepare acquisition.";
    }

    /// Start acquisition.
    BOOST_LOG_SEV(logger, notification) << "Starting acquisition...";
    cam->acqStart();

}

