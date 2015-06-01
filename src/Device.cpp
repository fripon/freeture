/*
                                Device.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau
*                               FRIPON-GEOPS-UPSUD-CNRS
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
    mDayAcqEnabled          = false;
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
    mDisplayVideoInInput    = false;
    mVideoFramesInInput     = false;

    switch(mType){

        case BASLER_GIGE :

            {
                #ifdef USE_PYLON
                    BOOST_LOG_SEV(logger, normal) << "INPUT : BASLER_GIGE -> Use Pylon";
                    cam = new CameraGigeSdkPylon();
                #else
                    #ifdef LINUX
                        BOOST_LOG_SEV(logger, normal) << "INPUT : BASLER_GIGE -> Use Aravis";
                        cam = new CameraGigeSdkAravis();
                    #endif
                #endif
            }

            break;

        case DMK_GIGE:

            {

                #ifdef WINDOWS
                    BOOST_LOG_SEV(logger, normal) << "INPUT : DMK_GIGE -> Use Imaging Source";
                    cam = new CameraGigeSdkIc();
                #else
                    #ifdef LINUX
                        BOOST_LOG_SEV(logger, normal) << "INPUT : DMK_GIGE -> Use Aravis";
                        cam = new CameraGigeSdkAravis(true);
                    #endif
                #endif

            }

            break;

        default :

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

                    // Get separator position in frame's name.
                    int FRAMES_SEPARATOR_POSITION; cfg.Get("FRAMES_SEPARATOR_POSITION", FRAMES_SEPARATOR_POSITION);
                    BOOST_LOG_SEV(logger, normal) << "Read FRAMES_SEPARATOR_POSITION from configuration file : " << FRAMES_SEPARATOR_POSITION;

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
                    cam = new CameraFrames(framesLocationList, FRAMES_SEPARATOR_POSITION);
                    if(!cam->grabInitialization())
                        throw "Fail to prepare acquisition on the first frames directory.";

                }

                break;

            case VIDEO:

                {
                    // Get frames locations.
                    string INPUT_VIDEO_PATH; cfg.Get("INPUT_VIDEO_PATH", INPUT_VIDEO_PATH);

                    // Get display input option.
                    cfg.Get("INPUT_VIDEO_DISPLAY", mDisplayVideoInInput);

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

                // Get camera ID to use.
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

                // Get day option.
                cfg.Get("ACQ_DAY_ENABLED", mDayAcqEnabled);
                BOOST_LOG_SEV(logger, notification) << "ACQ_DAY_ENABLED : " << mDayAcqEnabled;

                // Get day exposure time.
                cfg.Get("ACQ_DAY_EXPOSURE", mDayExposure);
                BOOST_LOG_SEV(logger, notification) << "ACQ_DAY_EXPOSURE : " << mDayExposure;

                // Get day gain.
                cfg.Get("ACQ_DAY_GAIN", mDayGain);
                BOOST_LOG_SEV(logger, notification) << "ACQ_DAY_GAIN : " << mDayGain;

                // Get ephemeris option.
                cfg.Get("EPHEMERIS_ENABLED", mEphemerisEnabled);
                BOOST_LOG_SEV(logger, notification) << "EPHEMERIS_ENABLED : " << mEphemerisEnabled;

                // Get schedule option status.
                cfg.Get("ACQ_SCHEDULE_ENABLED", mScheduleEnabled);
                BOOST_LOG_SEV(logger, notification) << "ACQ_SCHEDULE_ENABLED : " << mScheduleEnabled;

                // Get option to save a jpeg of a regular capture.
                cfg.Get("ACQ_SCHEDULE_SAVE_JPEG", mScheduleSaveJPEG);
                BOOST_LOG_SEV(logger, notification) << "ACQ_SCHEDULE_SAVE_JPEG : " << mScheduleSaveJPEG;

                 // Get regular acquisition option status.
                cfg.Get("ACQ_REGULAR_ENABLED", mRegularAcqEnabled);
                BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_ENABLED : " << mRegularAcqEnabled;

                if(mScheduleEnabled && mRegularAcqEnabled){

                    throw "\nCheck configuration file : \n \"You can enable ACQ_SCHEDULE_ENABLED or ACQ_REGULAR_ENABLED (not both)\"\n";

                }

                // Get regular acquisition time interval.
                cfg.Get("ACQ_REGULAR_INTERVAL", mRegularInterval);
                BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_INTERVAL : " << mRegularInterval;

                // Get regular acquisition exposure time.
                cfg.Get("ACQ_REGULAR_EXPOSURE", mRegularExposure);
                BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_EXPOSURE : " << mRegularExposure;

                // Get regular acquisition gain.
                cfg.Get("ACQ_REGULAR_GAIN", mRegularGain);
                BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_GAIN : " << mRegularGain;


                // Get regular acquisition repetition.
                cfg.Get("ACQ_REGULAR_REPETITION", mRegularRepetition);
                BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_REPETITION : " << mRegularRepetition;

                // Get option to save a jpeg of a regular capture.
                cfg.Get("ACQ_REGULAR_SAVE_JPEG", mRegularSaveJPEG);
                BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_SAVE_JPEG : " << mRegularSaveJPEG;

                // Get regular acquisition format.
                string acq_regular_format; cfg.Get("ACQ_REGULAR_FORMAT", acq_regular_format);
                EParser<CamBitDepth> cam_regular_bit_depth;
                mRegularFormat = cam_regular_bit_depth.parseEnum("ACQ_REGULAR_FORMAT", acq_regular_format);
                BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_FORMAT : " << acq_regular_format;

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

                // Get sunrise duration.
                cfg.Get("SUNRISE_DURATION", mSunriseDuration);
                BOOST_LOG_SEV(logger, notification) << "SUNRISE_DURATION : " << mSunriseDuration;

                {
                    // Compute start time of exposure control for sunrise.

                    float Hd_start = (mSunriseTime.at(0) * 3600 + mSunriseTime.at(1) * 60 - mSunriseDuration)/3600.f;

                    if(Hd_start < 0) Hd_start = Hd_start + 24;

                    cout << "Hd_start : " << Hd_start<< endl;

                    mSunriseTime.clear();

                    mSunriseTime = TimeDate::HdecimalToHMS(Hd_start);

                    cout <<  "SUNRISE_TIME_START : " << mSunriseTime.at(0) << "H" <<  mSunriseTime.at(1) << endl;


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

                 // Get sunset duration.
                cfg.Get("SUNSET_DURATION", mSunsetDuration);
                BOOST_LOG_SEV(logger, notification) << "SUNSET_DURATION : " << mSunsetDuration;

                {
                    // Compute start time of exposure control for sunset.

                    float Hd_start = (mSunsetTime.at(0) * 3600 + mSunsetTime.at(1) * 60 - mSunsetDuration)/3600.f;

                    if(Hd_start < 0) Hd_start = Hd_start + 24;

                    cout << "Hd_start : " << Hd_start<< endl;

                    mSunsetTime.clear();

                    mSunsetTime = TimeDate::HdecimalToHMS(Hd_start);

                    cout <<  "SUNSET_TIME_START : " << mSunsetTime.at(0) << "H" <<  mSunsetTime.at(1) << endl;

                }

                int timeStartSunrise = mSunriseTime.at(0) * 3600 + mSunriseTime.at(1) * 60;
                int timeStopSunrise = timeStartSunrise + mSunriseDuration * 2;
                int timeStartSunset = mSunsetTime.at(0) * 3600 + mSunsetTime.at(1) * 60;
                int timeStopSunset = timeStartSunset + mSunsetDuration * 2;

                cout << "timeStartSunrise : " << timeStartSunrise << endl;
                cout << "timeStopSunrise : " << timeStopSunrise << endl;
                cout << "timeStartSunset : " << timeStartSunset << endl;
                cout << "timeStopSunset : " << timeStopSunset << endl;

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

                            // Only keep night time.
                            if((scheduledTimeInSec > timeStopSunset) || (scheduledTimeInSec < timeStartSunrise)){

                                mSchedule.push_back(r);

                            }
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

                runContinuousAcquisition();

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

void Device::runContinuousAcquisition(){

    /// List Gige Camera to check the ID.
    BOOST_LOG_SEV(logger, notification) << "Printing Connected Gige Camera...";
    cam->listGigeCameras();

    /// Create camera according to its ID.
    BOOST_LOG_SEV(logger, notification) << "Creating Device according ID " << mCamID << " ...";
    if(!cam->createDevice(mCamID))
        throw "Fail to create device.";

    /// Set camera format.
    BOOST_LOG_SEV(logger, notification) << "Setting acquisition format...";
    if(!cam->setPixelFormat(mBitDepth))
        throw "Fail to set Format.";


    cam->getExposureBounds(mMinExposureTime, mMaxExposureTime);

    cam->getGainBounds(mMinGain, mMaxGain);

    boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
    string date = to_iso_extended_string(time);
    cout << "date : " << date << endl;
    vector<int> intDate = TimeDate::getIntVectorFromDateString(date);

    int timeStartSunrise = mSunriseTime.at(0) * 3600 + mSunriseTime.at(1) * 60;
    int timeStopSunrise = timeStartSunrise + mSunriseDuration * 2;
    int timeStartSunset = mSunsetTime.at(0) * 3600 + mSunsetTime.at(1) * 60;
    int timeStopSunset = timeStartSunset + mSunsetDuration * 2;

   // cout << "timeStartSunrise : " << timeStartSunrise << endl;
   // cout << "timeStopSunrise : " << timeStopSunrise << endl;
   // cout << "timeStartSunset : " << timeStartSunset << endl;
   // cout << "timeStopSunset : " << timeStopSunset << endl;

    int currentTimeInSec = intDate.at(3) * 3600 + intDate.at(4) * 60 + intDate.at(5);
   // cout << "currentTimeInSec : " << currentTimeInSec << endl;

    // Check sunrise and sunset time.
    if((currentTimeInSec > timeStopSunrise && currentTimeInSec < timeStartSunset)){

        cout << "DAY ! "<< endl;
        /// Set camera exposure time.
        BOOST_LOG_SEV(logger, notification) << "Setting day exposure time to " << mDayExposure << "...";
        if(!cam->setExposureTime(mDayExposure))
            throw "Fail to set Night Exposure.";

        /// Set camera gain.
        BOOST_LOG_SEV(logger, notification) << "Setting day gain to " << mDayGain << "...";
        if(!cam->setGain(mDayGain))
            throw "Fail to set Night Gain.";

    }else if((currentTimeInSec > timeStopSunset) || (currentTimeInSec < timeStartSunrise) ){

        cout << "NIGHT ! "<< endl;
        /// Set camera exposure time.
        BOOST_LOG_SEV(logger, notification) << "Setting night exposure time to " << mNightExposure << "...";
        if(!cam->setExposureTime(mNightExposure))
            throw "Fail to set Day Exposure.";

        /// Set camera gain.
        BOOST_LOG_SEV(logger, notification) << "Setting night gain to " << mNightGain << "...";
        if(!cam->setGain(mNightGain))
            throw "Fail to set Day Gain.";

    }else{

		cout << "set Day Exposure mMinExposureTime" << endl;

        if(!cam->setExposureTime(mMinExposureTime))
            throw "Fail to set Day Exposure.";

        /// Set camera gain.
        BOOST_LOG_SEV(logger, notification) << "Setting night gain to " << mMinGain << "...";
        if(!cam->setGain(mMinGain))
            throw "Fail to set Day Gain.";

    }

    /// Set camera fps.
    BOOST_LOG_SEV(logger, notification) << "Setting fps...";
    if(!cam->setFPS(mFPS))
        throw "Fail to set Fps.";

    /// Prepare grabbing.
    BOOST_LOG_SEV(logger, notification) << "Preparing camera to continuous acquisition...";
    if(!cam->grabInitialization())
        throw "Fail to start grab.";

    /// Start acquisition.
    BOOST_LOG_SEV(logger, notification) << "Starting acquisition...";
    cam->acqStart();

}

