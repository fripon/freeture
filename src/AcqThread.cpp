/*
                            AcqThread.cpp

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
* \file    AcqThread.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    21/01/2015
* \brief   Acquisition thread.
*/

#include "AcqThread.h"

boost::log::sources::severity_logger< LogSeverityLevel >  AcqThread::logger;

AcqThread::Init AcqThread::initializer;

AcqThread::AcqThread(   string                              cfgFile,
                        boost::circular_buffer<Frame>       *fb,
                        boost::mutex                        *fb_m,
                        boost::condition_variable           *fb_c,
                        bool                                *sSignal,
                        boost::mutex                        *sSignal_m,
                        boost::condition_variable           *sSignal_c,
                        bool                                *dSignal,
                        boost::mutex                        *dSignal_m,
                        boost::condition_variable           *dSignal_c,
                        DetThread                           *detection,
                        StackThread                         *stack):

                        mCfgPath(cfgFile), frameBuffer(fb), frameBuffer_mutex(fb_m),
                        frameBuffer_condition(fb_c), stackSignal(sSignal), stackSignal_mutex(sSignal_m),
                        stackSignal_condition(sSignal_c), detSignal(dSignal), detSignal_mutex(dSignal_m),
                        detSignal_condition(dSignal_c), pDetection(detection), pStack(stack), mThread(NULL),
                        mMustStop(false), mDevice(NULL), mNbGrabFail(0), mNbGrabSuccess(0), mThreadEndStatus(false),
                        mNextAcqIndex(0), pExpCtrl(NULL) {

    Configuration cfg;
    
    if(!cfg.Load(cfgFile))
        throw "Fail to load parameters for acq thread from configuration file.";

    // DATA LOCATION -----------------------------------------------------------

    if(!cfg.Get("DATA_PATH", mDataPath))
        throw "Fail to get DATA_PATH for acq thread.";
    BOOST_LOG_SEV(logger, notification) << "DATA_PATH : " << mDataPath;

    // STATION NAME ------------------------------------------------------------

    if(!cfg.Get("STATION_NAME", mStationName))
        throw "Fail to get STATION_NAME for acq thread.";
    BOOST_LOG_SEV(logger, notification) << "STATION_NAME : " << mStationName;

    // DETECTION MODE ----------------------------------------------------------

    string detection_mode;
    if(!cfg.Get("DET_MODE", detection_mode))
        throw "Fail to get DET_MODE for acq thread.";
    BOOST_LOG_SEV(logger, notification) << "DET_MODE : " << detection_mode;
    EParser<TimeMode> detMode;
    mDetectionMode = detMode.parseEnum("DET_MODE", detection_mode);

    // STACK MODE --------------------------------------------------------------

    string stack_mode;
    if(!cfg.Get("STACK_MODE", stack_mode))
        throw "Fail to get STACK_MODE for acq thread.";
    BOOST_LOG_SEV(logger, notification) << "STACK_MODE : " << stack_mode;
    EParser<TimeMode> stackMode;
    mStackMode = stackMode.parseEnum("STACK_MODE", stack_mode);

    // SCHEDULE STATUS ---------------------------------------------------------

    if(!cfg.Get("ACQ_SCHEDULE_ENABLED", mScheduleEnabled))
        throw "Fail to get ACQ_SCHEDULE_ENABLED for acq thread.";
    BOOST_LOG_SEV(logger, notification) << "ACQ_SCHEDULE_ENABLED : " << mScheduleEnabled;

    // SCHEDULE OUTPUT TYPE ----------------------------------------------------

    string sOutput;
    if(!cfg.Get("ACQ_SCHEDULE_OUTPUT", sOutput))
        throw "Fail to get ACQ_SCHEDULE_OUTPUT for acq thread.";
    BOOST_LOG_SEV(logger, notification) << "ACQ_SCHEDULE_OUTPUT : " << sOutput;
    EParser<ImgFormat> sOutput1;
    mScheduleOutput = sOutput1.parseEnum("ACQ_SCHEDULE_OUTPUT", sOutput);

    // SCHEDULE CFG ------------------------------------------------------------

    if(mScheduleEnabled) {

        string sACQ_SCHEDULE;
        if(!cfg.Get("ACQ_SCHEDULE", sACQ_SCHEDULE))
            throw "Fail to get ACQ_SCHEDULE for acq thread.";

        vector<string> sch1;

        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(",");
        tokenizer tokens(sACQ_SCHEDULE, sep);

        int n = 1;
        BOOST_LOG_SEV(logger, notification) << "SCHEDULE : ";
        for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter) {
            string s = *tok_iter;
            std::transform(s.begin(), s.end(),s.begin(), ::toupper);
            sch1.push_back(s);
            cout << "-> " << Conversion::intToString(n) << " - " << s << endl;
            BOOST_LOG_SEV(logger, notification) << "-> " << Conversion::intToString(n) << " - " << s;
            n++;
        }

        //23h25m00s10000000e400g12f1n
        for(int i = 0; i < sch1.size(); i++) {

            typedef boost::tokenizer<boost::char_separator<char> > tokenizer_;
            boost::char_separator<char> sep_("HMSEGFN");
            tokenizer tokens_(sch1.at(i), sep_);

            vector<string> sp;

            for(tokenizer::iterator tok_iter_ = tokens_.begin();tok_iter_ != tokens_.end(); ++tok_iter_)
                sp.push_back(*tok_iter_);
        
            if(sp.size() == 7) {

                AcqSchedule r = AcqSchedule(atoi(sp.at(0).c_str()), atoi(sp.at(1).c_str()), atoi(sp.at(2).c_str()), atoi(sp.at(3).c_str()), atoi(sp.at(4).c_str()), atoi(sp.at(5).c_str()), atoi(sp.at(6).c_str()));
                int scheduledTimeInSec = atoi(sp.at(0).c_str()) * 3600 + atoi(sp.at(1).c_str()) * 60 + atoi(sp.at(2).c_str());
                mSchedule.push_back(r);

            }
        }
    }

    // REGULAR STATUS ---------------------------------------------------------------

    if(!cfg.Get("ACQ_REGULAR_ENABLED", mRegularAcqEnabled))
        throw "Fail to get ACQ_REGULAR_ENABLED for acq thread.";
    BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_ENABLED : " << mRegularAcqEnabled;

    // REGULAR AND SCHEDULE CAN'T BE BOTH ACTIVE ------------------------------------

    if(mScheduleEnabled && mRegularAcqEnabled)
        throw "Check configuration file : \n \"You can enable ACQ_SCHEDULE_ENABLED or ACQ_REGULAR_ENABLED (not both)\"\n";

    // REGULAR ACQUISITION MODE -----------------------------------------------------

    string regular_mode;
    if(!cfg.Get("ACQ_REGULAR_MODE", regular_mode))
        throw "Fail to get ACQ_REGULAR_MODE for acq thread.";
    BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_MODE : " << regular_mode;
    EParser<TimeMode> regMode;
    mRegularMode = regMode.parseEnum("ACQ_REGULAR_MODE", regular_mode);

    // REGULAR ACQUISITION CONFIGURATION --------------------------------------------

    string regAcqParam;
    if(!cfg.Get("ACQ_REGULAR_CFG", regAcqParam))
        throw "Fail to get ACQ_REGULAR_CFG for acq thread.";
    std::transform(regAcqParam.begin(), regAcqParam.end(),regAcqParam.begin(), ::toupper);

    typedef boost::tokenizer<boost::char_separator<char> > tokenizer1;
    boost::char_separator<char> sep1("HMSEGFN");
    tokenizer1 tokens1(regAcqParam, sep1);

    vector<string> res1;
    for(tokenizer1::iterator tokIter = tokens1.begin();tokIter != tokens1.end(); ++tokIter)
        res1.push_back(*tokIter);

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

        BOOST_LOG_SEV(logger, notification) << "ACQ REGULAR : ";
        BOOST_LOG_SEV(logger, notification) << "  - Each : " << mRegularInterval << " seconds.";
        BOOST_LOG_SEV(logger, notification) << "  - " << mRegularExposure << " exposure time.";
        BOOST_LOG_SEV(logger, notification) << "  - " << mRegularGain << " gain.";
        BOOST_LOG_SEV(logger, notification) << "  - " << mRegularRepetition << " repetition.";
        BOOST_LOG_SEV(logger, notification) << "  - " << format.getStringEnum(mRegularFormat) << " format.";

    }else {
        throw "Fail to get ACQ_REGULAR_CFG for acq thread";
    }

    // REGULAR ACQUISITION OUTPUT TYPE ---------------------------------------------

    string rOutput;
    if(!cfg.Get("ACQ_REGULAR_OUTPUT", rOutput))
        throw "Fail to get ACQ_REGULAR_OUTPUT for acq thread.";
    BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_OUTPUT : " << rOutput;
    EParser<ImgFormat> rOutput1;
    mRegularOutput = rOutput1.parseEnum("ACQ_REGULAR_OUTPUT", rOutput);

    // EPHEMERIS STATUS ------------------------------------------------------------

    if(!cfg.Get("EPHEMERIS_ENABLED", mEphemerisEnabled))
        throw "Fail to get EPHEMERIS_ENABLED for acq thread.";

    if(mEphemerisEnabled)
        BOOST_LOG_SEV(logger, notification) << "EPHEMERIS_ENABLED : ON";
    else
        BOOST_LOG_SEV(logger, notification) << "EPHEMERIS_ENABLED : OFF";

    if(!mEphemerisEnabled) {

        // GET SUNRISE TIME --------------------------------------------------------

        string sunrise_time;
        if(!cfg.Get("SUNRISE_TIME", sunrise_time))
            throw "Fail to get SUNRISE_TIME for acq thread.";
        BOOST_LOG_SEV(logger, notification) << "SUNRISE_TIME : " << sunrise_time;

        {
            typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
            boost::char_separator<char> sep(":");
            tokenizer tokens(sunrise_time, sep);

            for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter)
                mSunriseTime.push_back(atoi((*tok_iter).c_str()));
        }

        // GET SUNSET TIME ---------------------------------------------------------

        string sunset_time;
        if(!cfg.Get("SUNSET_TIME", sunset_time))
            throw "Fail to get SUNSET_TIME for acq thread.";
        BOOST_LOG_SEV(logger, notification) << "SUNSET_TIME : " << sunset_time;

        {
            typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
            boost::char_separator<char> sep(":");
            tokenizer tokens(sunset_time, sep);

            for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter)
                mSunsetTime.push_back(atoi((*tok_iter).c_str()));
        }

        
        // SUNRISE DURATION (used if ephemerise = false) --------------------------------

        if(!cfg.Get("SUNRISE_DURATION", mSunriseDuration))
            throw "Fail to get SUNRISE_DURATION for acq thread.";
        BOOST_LOG_SEV(logger, notification) << "SUNRISE_DURATION : " << mSunriseDuration;

        // SUNSET DURATION (used if ephemerise = false) -----------------------------

        if(!cfg.Get("SUNSET_DURATION", mSunsetDuration))
            throw "Fail to get SUNSET_DURATION for acq thread.";
        BOOST_LOG_SEV(logger, notification) << "SUNSET_DURATION : " << mSunsetDuration;

    }else {

        // SUN HORIZON POSITION AT SUNRISE ------------------------------------------

        if(!cfg.Get("SUN_HORIZON_1", mSunHorizon1))
            throw "Fail to get SUN_HORIZON_1 for acq thread.";
        BOOST_LOG_SEV(logger, notification) << "SUN_HORIZON_1 : " << mSunHorizon1;

        // SUN HORIZON POSITION AT SUNSET -------------------------------------------

        if(!cfg.Get("SUN_HORIZON_2", mSunHorizon2))
            throw "Fail to get SUN_HORIZON_2 for acq thread.";
        BOOST_LOG_SEV(logger, notification) << "SUN_HORIZON_2 : " << mSunHorizon2;

        // LATITUDE -----------------------------------------------------------------

        if(!cfg.Get("SITELAT", mStationLatitude))
            throw "Fail to get SITELAT for acq thread.";

        // LONGITUDE ----------------------------------------------------------------

        if(!cfg.Get("SITELONG", mStationLongitude))
            throw "Fail to get SITELONG for acq thread.";

    }

    // SAVE INFOS ABOUT EXPOSURE CONTROL --------------------------------------------

    if(!cfg.Get("EXPOSURE_CONTROL_SAVE_INFOS", mExpCtrlSaveInfos))
        throw "Fail to get EXPOSURE_CONTROL_SAVE_INFOS for acq thread.";
    BOOST_LOG_SEV(logger, notification) << "EXPOSURE_CONTROL_SAVE_INFOS : " << mExpCtrlSaveInfos;

    // SAVE IMAGE ABOUT EXPOSURE CONTROL --------------------------------------------

    if(!cfg.Get("EXPOSURE_CONTROL_SAVE_IMAGE", mExpCtrlSaveImg))
        throw "Fail to get EXPOSURE_CONTROL_SAVE_IMAGE for acq thread.";
    BOOST_LOG_SEV(logger, notification) << "EXPOSURE_CONTROL_SAVE_IMAGE : " << mExpCtrlSaveImg;

    // EXPOSURE CONTROL FREQUENCY ---------------------------------------------------

    if(!cfg.Get("EXPOSURE_CONTROL_FREQUENCY", mExpCtrlFrequency))
        throw "Fail to get EXPOSURE_CONTROL_FREQUENCY for acq thread.";
    BOOST_LOG_SEV(logger, notification) << "EXPOSURE_CONTROL_FREQUENCY : " << mExpCtrlFrequency;

    // MASK STATUS ------------------------------------------------------------------

    if(!cfg.Get("ACQ_MASK_ENABLED", mMaskEnabled))
        throw "Fail to get ACQ_MASK_ENABLED for acq thread.";
    BOOST_LOG_SEV(logger, notification) << "ACQ_MASK_ENABLED : " << mMaskEnabled;

    if(mMaskEnabled) {

        // MASK PATH ----------------------------------------------------------------

        if(!cfg.Get("ACQ_MASK_PATH", mMaskPath))
            throw "Fail to get ACQ_MASK_PATH for acq thread.";
        BOOST_LOG_SEV(logger, notification) << "ACQ_MASK_PATH : " << mMaskPath;

        mMask = imread(mMaskPath, CV_LOAD_IMAGE_GRAYSCALE);

        if(!mMask.data) {

            throw "Failed to load the mask for acq thread.";

        }
    }

    // FITS KEYS ----------------------------------------------------------------

    mFitsHeader.loadKeywordsFromConfigFile(mCfgPath);

    // OTHERS INITIALIZATION ----------------------------------------------------

    mMaxGain = 0; mMaxGain = 0;
    mMinExposureTime = 0; mMaxExposureTime = 0;

    fpsBuffer = boost::circular_buffer<double>(100);
    averageFPS = 30; // Default

}

AcqThread::~AcqThread(void){

    if(mDevice != NULL)
        delete mDevice;

    if(mThread != NULL)
        delete mThread;

    if(pExpCtrl != NULL)
        delete pExpCtrl;

}

void AcqThread::stopThread(){

    // Signal the thread to stop (thread-safe)
    mMustStopMutex.lock();
    mMustStop = true;
    mMustStopMutex.unlock();

    if(mThread != NULL){

        while(mThread->timed_join(boost::posix_time::seconds(2)) == false){

            mThread->interrupt();

        }

    }

}

bool AcqThread::startThread() {

    // CREATE DEVICE
    mDevice = new Device(mCfgPath);

    // SEARCH DEVICES
    mDevice->listDevices(false);

    // PREPARE DEVICES
    if(!configureInputDevice())
        return false;

    BOOST_LOG_SEV(logger,normal) << "Success to prepare device.";
    BOOST_LOG_SEV(logger,normal) << "Create acquisition thread.";

    mThread = new boost::thread(boost::ref(*this));

    return true;

}

bool AcqThread::getThreadEndStatus(){

    return mThreadEndStatus;

}

void AcqThread::operator()(){

    bool stop = false;

    BOOST_LOG_SCOPED_THREAD_TAG("LogName", "ACQ_THREAD");
    BOOST_LOG_SEV(logger,notification) << "\n";
    BOOST_LOG_SEV(logger,notification) << "==============================================";
    BOOST_LOG_SEV(logger,notification) << "========== START ACQUISITION THREAD ==========";
    BOOST_LOG_SEV(logger,notification) << "==============================================";

    try {

        /// Prepare scheduled long acquisition.
        sortAcquisitionSchedule(); // Order schedule times.

        boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();

        // Search next acquisition according to the current time.
        selectNextAcquisitionSchedule(TimeDate::splitIsoExtendedDate(to_iso_extended_string(time)));

        bool scheduleTaskStatus = false;
        bool scheduleTaskActive = false;

        /// Exposure adjustment variables.

        bool exposureControlStatus = false;
        bool exposureControlActive = false;
        bool cleanStatus = false;
            
        if(mDevice->getExposureStatus()) {

            pExpCtrl = new ExposureControl( mExpCtrlFrequency,
                                            mExpCtrlSaveImg,
                                            mExpCtrlSaveInfos,
                                            mDataPath,
                                            mStationName);
        }

        TimeMode previousTimeMode = NONE;

        /// Acquisition process.

        do {

            // Location of a video or frames if input type is FRAMES or VIDEO.
            string location = "";

            // Load videos file or frames directory if input type is FRAMES or VIDEO
            if(!mDevice->loadNextCameraDataSet(location)) break;

            if(pDetection != NULL) pDetection->setCurrentDataSet(location);

            /// Reference time to compute interval between regular captures.

            time = boost::posix_time::microsec_clock::universal_time();
            string cDate = to_simple_string(time);
            string dateDelimiter = ".";
            string refDate = cDate.substr(0, cDate.find(dateDelimiter));

            do {

                // ############## UPDATE FPS ##############

                // Get Fps from camera
                double fps = 0; 
                bool getFPS = mDevice->getCameraFPS(fps);

                if(getFPS == false) {
                    fps = averageFPS;
                }
        
                // Update timeBeforeEvent (in fps) and timeAfterEvent (in fps) according fps value.
                if(pDetection != NULL) {
                    if(!mDevice->mVideoFramesInput) {
                        pDetection->setTimeAfterEvent(fps);
                        pDetection->setTimeBeforeEvent(fps);
                    }else {
                        pDetection->setTimeAfterEvent(0);
                        pDetection->setTimeBeforeEvent(0);
                    }
                }

                // Container for the grabbed image.
                Frame newFrame;

                // Time counter of grabbing a frame.
                double tacq = (double)getTickCount();

                // Grab a frame.
                if(mDevice->runContinuousCapture(newFrame)) {

                    BOOST_LOG_SEV(logger, normal)   << "============= FRAME " << newFrame.mFrameNumber << " ============= ";
                    cout                            << "============= FRAME " << newFrame.mFrameNumber << " ============= " << endl;

                    // Camera type in input is FRAMES or VIDEO.
                    if(mDevice->mVideoFramesInput) {

                        // Push the new frame in the framebuffer.
                        boost::mutex::scoped_lock lock(*frameBuffer_mutex);
                        frameBuffer->push_back(newFrame);
                        lock.unlock();

                        // Notify detection thread.
                        if(pDetection != NULL) {

                            boost::mutex::scoped_lock lock2(*detSignal_mutex);
                            *detSignal = true;
                            detSignal_condition->notify_one();
                            lock2.unlock();

                        }

                        // Slow down the time in order to give more time to the detection process.

                        #ifdef WINDOWS
                            Sleep(500);
                        #else
                            #ifdef LINUX
                                usleep(500000);
                            #endif
                        #endif

                    }else {

                        // Get current time in seconds.
                        int currentTimeInSec = newFrame.mDate.hours * 3600 + newFrame.mDate.minutes * 60 + (int)newFrame.mDate.seconds;

                        // Detect day or night status.
                        TimeMode currentTimeMode = NONE;

                        if((currentTimeInSec > mStopSunsetTime) || (currentTimeInSec < mStartSunriseTime)) {
                            currentTimeMode = NIGHT;
                        }else if((currentTimeInSec > mStartSunriseTime) && (currentTimeInSec < mStopSunsetTime)) {
                            currentTimeMode = DAY;
                        }

                        // If exposure control is not active, the new frame can be shared with others threads.
                        if(!exposureControlStatus) {

                            // Push the new frame in the framebuffer.
                            boost::mutex::scoped_lock lock(*frameBuffer_mutex);
                            frameBuffer->push_back(newFrame);
                            lock.unlock();

                            // Notify detection thread.
                            if(pDetection != NULL) {

                                if(previousTimeMode != NONE && previousTimeMode != currentTimeMode && mDetectionMode != DAYNIGHT) {

                                    BOOST_LOG_SEV(logger, notification) << "TimeMode has changed ! ";
                                    boost::mutex::scoped_lock lock(*detSignal_mutex);
                                    *detSignal = false;
                                    lock.unlock();
                                    cout << "Send interruption signal to detection process " << endl;
                                    pDetection->interruptThread();

                                }else if(mDetectionMode == currentTimeMode || mDetectionMode == DAYNIGHT) {

                                    boost::mutex::scoped_lock lock2(*detSignal_mutex);
                                    *detSignal = true;
                                    detSignal_condition->notify_one();
                                    lock2.unlock();

                                }
                            }

                            // Notify stack thread.
                            if(pStack != NULL) {

                                // TimeMode has changed.
                                if(previousTimeMode != NONE && previousTimeMode != currentTimeMode && mStackMode != DAYNIGHT) {

                                    BOOST_LOG_SEV(logger, notification) << "TimeMode has changed ! ";
                                    boost::mutex::scoped_lock lock(*stackSignal_mutex);
                                    *stackSignal = false;
                                    lock.unlock();

                                    // Force interruption.
                                    cout << "Send interruption signal to stack " << endl;
                                    pStack->interruptThread();

                                }else if(mStackMode == currentTimeMode || mStackMode == DAYNIGHT) {

                                    boost::mutex::scoped_lock lock3(*stackSignal_mutex);
                                    *stackSignal = true;
                                    stackSignal_condition->notify_one();
                                    lock3.unlock();

                                }
                            }

                            cleanStatus = false;

                        }else {

                            // Exposure control is active, the new frame can't be shared with others threads.
                            if(!cleanStatus) {

                                // If stack process exists.
                                if(pStack != NULL) {

                                    boost::mutex::scoped_lock lock(*stackSignal_mutex);
                                    *stackSignal = false;
                                    lock.unlock();

                                    // Force interruption.
                                    cout << "Send interruption signal to stack " << endl;
                                    pStack->interruptThread();

                                }

                                // If detection process exists
                                if(pDetection != NULL) {

                                    boost::mutex::scoped_lock lock(*detSignal_mutex);
                                    *detSignal = false;
                                    lock.unlock();
                                    cout << "Sending interruption signal to detection process... " << endl;
                                    pDetection->interruptThread();

                                }

                                // Reset framebuffer.
                                cout << "Cleaning frameBuffer..." << endl;
                                boost::mutex::scoped_lock lock(*frameBuffer_mutex);
                                frameBuffer->clear();
                                lock.unlock();

                                cleanStatus = true;

                            }

                        }

                        previousTimeMode = currentTimeMode;

                        // Adjust exposure time.
                        if(pExpCtrl != NULL && exposureControlActive)
                            exposureControlStatus = pExpCtrl->controlExposureTime(mDevice, newFrame.mImg, newFrame.mDate, mMask, mMinExposureTime, fps);

                        // Get current date YYYYMMDD.
                        string currentFrameDate =   TimeDate::getYYYYMMDD(newFrame.mDate);

                        // If the date has changed, sun ephemeris must be updated.
                        if(currentFrameDate != mCurrentDate) {

                            BOOST_LOG_SEV(logger, notification) << "Date has changed. Former Date is " << mCurrentDate << ". New Date is " << currentFrameDate << "." ;
                            getSunTimes();

                        }

                        // Acquisition at regular time interval is enabled.
                        if(mRegularAcqEnabled && !mDevice->mVideoFramesInput) {

                            time = boost::posix_time::microsec_clock::universal_time();
                            cDate = to_simple_string(time);
                            string nowDate = cDate.substr(0, cDate.find(dateDelimiter));

                            boost::posix_time::ptime t1(boost::posix_time::time_from_string(refDate));
                            boost::posix_time::ptime t2(boost::posix_time::time_from_string(nowDate));

                            boost::posix_time::time_duration td = t2 - t1;
                            long secTime = td.total_seconds();
                            cout << "NEXT REGCAP : " << (int)(mRegularInterval - secTime) << "s" <<  endl;

                            // Check it's time to run a regular capture.
                            if(secTime >= mRegularInterval) {

                                // Current time is after the sunset stop and before the sunrise start = NIGHT
                                if((currentTimeMode == NIGHT) && (mRegularMode == NIGHT || mRegularMode == DAYNIGHT)) {

                                        BOOST_LOG_SEV(logger, notification) << "Run regular acquisition.";

                                        runImageCapture(    mRegularRepetition,
                                                            mRegularExposure,
                                                            mRegularGain,
                                                            mRegularFormat,
                                                            mRegularOutput);

                                // Current time is between sunrise start and sunset stop = DAY
                                }else if(currentTimeMode == DAY && (mRegularMode == DAY || mRegularMode == DAYNIGHT)) {

                                    BOOST_LOG_SEV(logger, notification) << "Run regular acquisition.";
                                    saveImageCaptured(newFrame, 0, mRegularOutput);

                                }

                                // Reset reference time in case a long exposure has been done.
                                time = boost::posix_time::microsec_clock::universal_time();
                                cDate = to_simple_string(time);
                                refDate = cDate.substr(0, cDate.find(dateDelimiter));

                            }

                        }

                        // Acquisiton at scheduled time is enabled.
                        if(mSchedule.size() != 0 && mScheduleEnabled && !mDevice->mVideoFramesInput) {
                            cout << newFrame.mDate.hours << newFrame.mDate.minutes << (int)newFrame.mDate.seconds<< endl;
                            cout << mNextAcq.getH() << mNextAcq.getM() << (int)mNextAcq.getS()<< endl;
                            // It's time to run scheduled acquisition.
                            if( mNextAcq.getH() == newFrame.mDate.hours &&
                                mNextAcq.getM() == newFrame.mDate.minutes &&
                                (int)newFrame.mDate.seconds == mNextAcq.getS()) {

                                mNextAcq.setDate(TimeDate::getIsoExtendedFormatDate(newFrame.mDate));

                                CamBitDepth format;
                                Conversion::intBitDepthToCamBitDepthEnum(mNextAcq.getF(), format);

                                runImageCapture(    mNextAcq.getN(),
                                                    mNextAcq.getE(),
                                                    mNextAcq.getG(),
                                                    format,
                                                    mScheduleOutput);

                                // Update mNextAcq
                                selectNextAcquisitionSchedule(newFrame.mDate);

                            }else {

                                // The current time has elapsed.
                                if(newFrame.mDate.hours > mNextAcq.getH()) {

                                   selectNextAcquisitionSchedule(newFrame.mDate);

                                }else if(newFrame.mDate.hours == mNextAcq.getH()) {

                                    if(newFrame.mDate.minutes > mNextAcq.getM()) {

                                        selectNextAcquisitionSchedule(newFrame.mDate);

                                    }else if(newFrame.mDate.minutes == mNextAcq.getM()) {

                                        if(newFrame.mDate.seconds > mNextAcq.getS()) {

                                            selectNextAcquisitionSchedule(newFrame.mDate);

                                        }

                                    }

                                }

                            }

                        }

                        // Check sunrise and sunset time.
                        if( (((currentTimeInSec > mStartSunriseTime && currentTimeInSec < mStopSunriseTime) ||
                            (currentTimeInSec > mStartSunsetTime && currentTimeInSec < mStopSunsetTime))) && !mDevice->mVideoFramesInput) {

                            exposureControlActive = true;


                        }else {

                            if(exposureControlActive) {

                                // In DAYTIME : Apply minimum available exposure time.
                                if((currentTimeInSec >= mStopSunriseTime && currentTimeInSec < mStartSunsetTime)){

                                    BOOST_LOG_SEV(logger, notification) << "Apply day exposure time : " << mDevice->getDayExposureTime();
                                    mDevice->setCameraDayExposureTime();
                                    BOOST_LOG_SEV(logger, notification) << "Apply day exposure time : " << mDevice->getDayGain();
                                    mDevice->setCameraDayGain();

                                // In NIGHTTIME : Apply maximum available exposure time.
                                }else if((currentTimeInSec >= mStopSunsetTime) || (currentTimeInSec < mStartSunriseTime)){

                                    BOOST_LOG_SEV(logger, notification) << "Apply night exposure time." << mDevice->getNightExposureTime();
                                    mDevice->setCameraNightExposureTime();
                                    BOOST_LOG_SEV(logger, notification) << "Apply night exposure time." << mDevice->getNightGain();
                                    mDevice->setCameraNightGain();

                                }
                            }

                            exposureControlActive = false;
                            exposureControlStatus = false;

                        }

                    }

                }else {

                    mNbGrabFail++;

                }

                tacq = (((double)getTickCount() - tacq)/getTickFrequency())*1000;

                if(!getFPS) {

                    fpsBuffer.push_back((1.0/(tacq/1000.0)));

                    if(fpsBuffer.size() != 100) {

                        std::cout << " [ TIME ACQ ] : " << tacq << " ms   FPS("  << (1.0/(tacq/1000.0)) << ")" <<  endl;
                        averageFPS = (1.0/(tacq/1000.0));

                    }else {

                        std::cout << " [ TIME ACQ ] : " << tacq << " ms  ";
                        double ff = 0.0;
                        for(int i = 0; i< fpsBuffer.size(); i++)
                            ff += fpsBuffer.at(i);
                        averageFPS = ff / fpsBuffer.size();
                        cout << " (FPS : " << averageFPS << ")" << endl;

                    }

                }else{
                    std::cout << " [ TIME ACQ ] : " << tacq << " ms   FPS("  << fps << ")" <<  endl;
                }
 
                BOOST_LOG_SEV(logger, normal) << " [ TIME ACQ ] : " << tacq << " ms";

                mMustStopMutex.lock();
                stop = mMustStop;
                mMustStopMutex.unlock();

            }while(stop == false && !mDevice->getCameraStatus());

            // Reset detection process to prepare the analyse of a new data set.
            if(pDetection != NULL) {

                pDetection->getDetMethod()->resetDetection(true);
                pDetection->getDetMethod()->resetMask();
                pDetection->updateDetectionReport();
                if(!pDetection->getRunStatus()) break;

            }

            // Clear framebuffer.
            boost::mutex::scoped_lock lock(*frameBuffer_mutex);
            frameBuffer->clear();
            lock.unlock();

        }while(mDevice->getCameraDataSetStatus() && stop == false);

    }catch(const boost::thread_interrupted&){

        BOOST_LOG_SEV(logger,notification) << "AcqThread ended.";
        cout << "AcqThread ended." <<endl;

    }catch(exception& e){

        cout << "An exception occured : " << e.what() << endl;
        BOOST_LOG_SEV(logger, critical) << "An exception occured : " << e.what();

    }catch(const char * msg) {

        cout << endl << msg << endl;

    }

    mDevice->stopCamera();

    mThreadEndStatus = true;

    std::cout << "Acquisition Thread TERMINATED." << endl;
    BOOST_LOG_SEV(logger,notification) << "Acquisition Thread TERMINATED";

}

void AcqThread::selectNextAcquisitionSchedule(TimeDate::Date date){

    if(mSchedule.size() != 0){

        // Search next acquisition
        for(int i = 0; i < mSchedule.size(); i++){

            if(date.hours < mSchedule.at(i).getH()){

               mNextAcqIndex = i;
               break;

            }else if(date.hours == mSchedule.at(i).getH()){

                if(date.minutes < mSchedule.at(i).getM()){

                    mNextAcqIndex = i;
                    break;

                }else if(date.minutes == mSchedule.at(i).getM()){

                    if(date.seconds < mSchedule.at(i).getS()){

                        mNextAcqIndex = i;
                        break;

                    }
                }
            }
        }

        mNextAcq = mSchedule.at(mNextAcqIndex);

    }

}

void AcqThread::sortAcquisitionSchedule(){

    if(mSchedule.size() != 0){

        // Sort time in list.
        vector<AcqSchedule> tempSchedule;

        do{

            int minH; int minM; int minS; bool init = false;

            vector<AcqSchedule>::iterator it;
            vector<AcqSchedule>::iterator it_select;

            for(it = mSchedule.begin(); it != mSchedule.end(); ++it){

                if(!init){

                    minH = (*it).getH();
                    minM = (*it).getM();
                    minS = (*it).getS();
                    it_select = it;
                    init = true;

                }else{

                    if((*it).getH() < minH){

                        minH = (*it).getH();
                        minM = (*it).getM();
                        minS = (*it).getS();
                        it_select = it;

                    }else if((*it).getH() == minH){

                        if((*it).getM() < minM){

                            minH = (*it).getH();
                            minM = (*it).getM();
                            minS = (*it).getS();
                            it_select = it;

                        }else if((*it).getM() == minM){

                            if((*it).getS() < minS){

                                minH = (*it).getH();
                                minM = (*it).getM();
                                minS = (*it).getS();
                                it_select = it;

                            }

                        }

                    }

                }

            }

            if(init){

                tempSchedule.push_back((*it_select));
                cout << "-> " << (*it_select).getH() << "H " << (*it_select).getM() << "M " << (*it_select).getS() << "S " << endl;
                mSchedule.erase(it_select);

            }

        }while(mSchedule.size() != 0);

        mSchedule = tempSchedule;

    }

}

bool AcqThread::buildAcquisitionDirectory(string YYYYMMDD){

    namespace fs = boost::filesystem;
    string root = mDataPath + mStationName + "_" + YYYYMMDD +"/";

    string subDir = "captures/";
    string finalPath = root + subDir;

    mDataLocation = finalPath;
    BOOST_LOG_SEV(logger,notification) << "CompleteDataPath : " << mDataLocation;

    path p(mDataPath);
    path p1(root);
    path p2(root + subDir);

    // If DATA_PATH exists
    if(fs::exists(p)){

        // If DATA_PATH/STATI ON_YYYYMMDD/ exists
        if(fs::exists(p1)){

            // If DATA_PATH/STATION_YYYYMMDD/captures/ doesn't exists
            if(!fs::exists(p2)){

                // If fail to create DATA_PATH/STATION_YYYYMMDD/captures/
                if(!fs::create_directory(p2)){

                    BOOST_LOG_SEV(logger,critical) << "Unable to create captures directory : " << p2.string();
                    return false;

                // If success to create DATA_PATH/STATION_YYYYMMDD/captures/
                }else{

                   BOOST_LOG_SEV(logger,notification) << "Success to create captures directory : " << p2.string();
                   return true;

                }
            }

        // If DATA_PATH/STATION_YYYYMMDD/ doesn't exists
        }else{

            // If fail to create DATA_PATH/STATION_YYYYMMDD/
            if(!fs::create_directory(p1)){

                BOOST_LOG_SEV(logger,fail) << "Unable to create STATION_YYYYMMDD directory : " << p1.string();
                return false;

            // If success to create DATA_PATH/STATION_YYYYMMDD/
            }else{

                BOOST_LOG_SEV(logger,notification) << "Success to create STATION_YYYYMMDD directory : " << p1.string();

                // If fail to create DATA_PATH/STATION_YYYYMMDD/astro/
                if(!fs::create_directory(p2)){

                    BOOST_LOG_SEV(logger,critical) << "Unable to create captures directory : " << p2.string();
                    return false;

                // If success to create DATA_PATH/STATION_YYYYMMDD/astro/
                }else{

                    BOOST_LOG_SEV(logger,notification) << "Success to create captures directory : " << p2.string();
                    return true;

                }
            }
        }

    // If DATA_PATH doesn't exists
    }else{

        // If fail to create DATA_PATH
        if(!fs::create_directory(p)){

            BOOST_LOG_SEV(logger,fail) << "Unable to create DATA_PATH directory : " << p.string();
            return false;

        // If success to create DATA_PATH
        }else{

            BOOST_LOG_SEV(logger,notification) << "Success to create DATA_PATH directory : " << p.string();

            // If fail to create DATA_PATH/STATION_YYYYMMDD/
            if(!fs::create_directory(p1)){

                BOOST_LOG_SEV(logger,fail) << "Unable to create STATION_YYYYMMDD directory : " << p1.string();
                return false;

            // If success to create DATA_PATH/STATION_YYYYMMDD/
            }else{

                BOOST_LOG_SEV(logger,notification) << "Success to create STATION_YYYYMMDD directory : " << p1.string();

                // If fail to create DATA_PATH/STATION_YYYYMMDD/captures/
                if(!fs::create_directory(p2)){

                    BOOST_LOG_SEV(logger,critical) << "Unable to create captures directory : " << p2.string();
                    return false;

                // If success to create DATA_PATH/STATION_YYYYMMDD/captures/
                }else{

                    BOOST_LOG_SEV(logger,notification) << "Success to create captures directory : " << p2.string();
                    return true;

                }
            }
        }
    }

    return true;
}

void AcqThread::runImageCapture(int imgNumber, int imgExposure, int imgGain, CamBitDepth imgFormat, ImgFormat imgOutput) {

    // Stop camera
    mDevice->stopCamera();

    // Stop stack process.
    if(pStack != NULL){

        boost::mutex::scoped_lock lock(*stackSignal_mutex);
        *stackSignal = false;
        lock.unlock();

        // Force interruption.
        BOOST_LOG_SEV(logger, notification) << "Send reset signal to stack. ";
        pStack->interruptThread();

    }

    // Stop detection process.
    if(pDetection != NULL){

        boost::mutex::scoped_lock lock(*detSignal_mutex);
        *detSignal = false;
        lock.unlock();
        BOOST_LOG_SEV(logger, notification) << "Send reset signal to detection process. ";
        pDetection->interruptThread();

    }

    // Reset framebuffer.
    BOOST_LOG_SEV(logger, notification) << "Cleaning frameBuffer...";
    boost::mutex::scoped_lock lock(*frameBuffer_mutex);
    frameBuffer->clear();
    lock.unlock();

    for(int i = 0; i < imgNumber; i++) {

        BOOST_LOG_SEV(logger, notification) << "Prepare capture n° " << i;

        // Configuration for single capture.
        Frame frame;
        BOOST_LOG_SEV(logger, notification) << "Exposure time : " << imgExposure;
        frame.mExposure  = imgExposure;
        BOOST_LOG_SEV(logger, notification) << "Gain : " << imgGain;
        frame.mGain = imgGain;
        EParser<CamBitDepth> format;
        BOOST_LOG_SEV(logger, notification) << "Format : " << format.getStringEnum(imgFormat);
        frame.mBitDepth = imgFormat;

        // Run single capture.
        BOOST_LOG_SEV(logger, notification) << "Run single capture.";
        if(mDevice->runSingleCapture(frame)) {

            BOOST_LOG_SEV(logger, notification) << "Single capture succeed !";
            cout << "Single capture succeed !" << endl;
            saveImageCaptured(frame, i, imgOutput);

        }else{

            BOOST_LOG_SEV(logger, fail) << "Single capture failed !";

        }

    }

    #ifdef WINDOWS
        Sleep(1000);
    #else
        #ifdef LINUX
            sleep(1);
        #endif
    #endif

    BOOST_LOG_SEV(logger, notification) << "Restarting camera in continuous mode...";
    configureInputDevice();

}

void AcqThread::saveImageCaptured(Frame &img, int imgNum, ImgFormat outputType) {

    if(img.mImg.data) {

        string  YYYYMMDD = TimeDate::getYYYYMMDD(img.mDate);

        if(buildAcquisitionDirectory(YYYYMMDD)) {

            string fileName = "CAP_" + TimeDate::getYYYYMMDDThhmmss(img.mDate) + "_UT-" + Conversion::intToString(imgNum);

            switch(outputType) {

                case JPEG :

                    {

                        switch(img.mBitDepth) {

                            case MONO_8 :

                                {

                                    Mat temp;
                                    img.mImg.copyTo(temp);
                                    Mat newMat = ImgProcessing::correctGammaOnMono8(temp, 2.2);
                                    SaveImg::saveJPEG(newMat, mDataLocation + fileName);

                                }

                                break;

                            case MONO_12 :

                                {

                                    Mat temp;
                                    img.mImg.copyTo(temp);
                                    Mat newMat = ImgProcessing::correctGammaOnMono12(temp, 2.2);
                                    Mat newMat2 = Conversion::convertTo8UC1(newMat);
                                    SaveImg::saveJPEG(newMat2, mDataLocation + fileName);

                                }

                                break;
                        }
                    }

                    break;

                case FITS :

                    {

                        Fits2D newFits(mDataLocation);
                        newFits.copyKeywords(mFitsHeader);
                        newFits.kGAINDB = img.mGain;
                        newFits.kEXPOSURE = img.mExposure/1000000.0;
                        newFits.kONTIME = img.mExposure/1000000.0;
                        newFits.kELAPTIME = img.mExposure/1000000.0;
                        newFits.kDATEOBS = TimeDate::getIsoExtendedFormatDate(img.mDate);

                        double  debObsInSeconds = img.mDate.hours*3600 + img.mDate.minutes*60 + img.mDate.seconds;
                        double  julianDate      = TimeDate::gregorianToJulian(img.mDate);
                        double  julianCentury   = TimeDate::julianCentury(julianDate);

                        newFits.kCRVAL1 = TimeDate::localSideralTime_2(julianCentury, img.mDate.hours, img.mDate.minutes, (int)img.mDate.seconds, mFitsHeader.kSITELONG);
                        newFits.kCTYPE1 = "RA---ARC";
                        newFits.kCTYPE2 = "DEC--ARC";
                        newFits.kEQUINOX = 2000.0;

                        switch(img.mBitDepth) {

                            case MONO_8 :

                                {

                                   if(newFits.writeFits(img.mImg, UC8, fileName))
                                        cout << ">> Fits saved in : " << mDataLocation << fileName << endl;

                                }

                                break;

                            case MONO_12 :

                                {

                                    // Convert unsigned short type image in short type image.
                                    Mat newMat = Mat(img.mImg.rows, img.mImg.cols, CV_16SC1, Scalar(0));

                                    // Set bzero and bscale for print unsigned short value in soft visualization.
                                    newFits.kBZERO = 32768;
                                    newFits.kBSCALE = 1;

                                    unsigned short *ptr = NULL;
                                    short *ptr2 = NULL;

                                    for(int i = 0; i < img.mImg.rows; i++){

                                        ptr = img.mImg.ptr<unsigned short>(i);
                                        ptr2 = newMat.ptr<short>(i);

                                        for(int j = 0; j < img.mImg.cols; j++){

                                            if(ptr[j] - 32768 > 32767){

                                                ptr2[j] = 32767;

                                            }else{

                                                ptr2[j] = ptr[j] - 32768;
                                            }
                                        }
                                    }

                                    // Create FITS image with BITPIX = SHORT_IMG (16-bits signed integers), pixel with TSHORT (signed short)
                                    if(newFits.writeFits(newMat, S16, fileName))
                                        cout << ">> Fits saved in : " << mDataLocation << fileName << endl;

                                }

                                break;
                        }

                    }

                    break;

            }

        }
    }

}

bool AcqThread::getSunTimes() {

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

        }else {

            sunriseStopH = sunriseStartH;

        }

        double intpart2 = 0;
        double fractpart2 = modf(fractpart1 * 60 , &intpart2);

        if(sunriseStartM + intpart2 < 60) {

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

        }else {

            sunsetStopH = sunsetStartH;

        }

        double intpart4 = 0;
        double fractpart4 = modf(fractpart3 * 60 , &intpart4);

        if(sunsetStartM + intpart4 < 60) {

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

bool AcqThread::configureInputDevice() {

    // CREATE CAMERA
    if(!mDevice->createCamera())
        return false;

    // SET SIZE
    if(!mDevice->setCameraSize())
        return false;
    
    // SET FORMAT
    if(!mDevice->setCameraPixelFormat())
        return false;

    // GET BOUNDS
    mDevice->getCameraExposureBounds(mMinExposureTime, mMaxExposureTime);
    mDevice->getCameraGainBounds(mMinGain, mMaxGain);

    // Get Sunrise start/stop, Sunset start/stop. ---
    getSunTimes();

    // CHECK SUNRISE AND SUNSET TIMES.

    if((mCurrentTime > mStopSunsetTime) || (mCurrentTime < mStartSunriseTime)) {

        BOOST_LOG_SEV(logger, notification) << "DAYTIME         :  NO";
        BOOST_LOG_SEV(logger, notification) << "AUTO EXPOSURE   :  NO";
        BOOST_LOG_SEV(logger, notification) << "EXPOSURE TIME   :  " << mDevice->getNightExposureTime();
        BOOST_LOG_SEV(logger, notification) << "GAIN            :  " << mDevice->getNightGain();

        if(!mDevice->setCameraNightExposureTime())
            return false;

        if(!mDevice->setCameraNightGain())
           return false;

    }else if((mCurrentTime > mStopSunriseTime && mCurrentTime < mStartSunsetTime)) {

        BOOST_LOG_SEV(logger, notification) << "DAYTIME         :  YES";
        BOOST_LOG_SEV(logger, notification) << "AUTO EXPOSURE   :  NO";
        BOOST_LOG_SEV(logger, notification) << "EXPOSURE TIME   :  " << mDevice->getDayExposureTime();
        BOOST_LOG_SEV(logger, notification) << "GAIN            :  " << mDevice->getDayGain();

        if(!mDevice->setCameraDayExposureTime())
            return false;

        if(!mDevice->setCameraDayGain())
            return false;

    }else{

        BOOST_LOG_SEV(logger, notification) << "DAYTIME         :  NO";
        BOOST_LOG_SEV(logger, notification) << "AUTO EXPOSURE   :  YES";
        BOOST_LOG_SEV(logger, notification) << "EXPOSURE TIME   :  Minimum (" << mMinExposureTime << ")"<< mDevice->getNightExposureTime();
        BOOST_LOG_SEV(logger, notification) << "GAIN            :  Minimum (" << mMinGain << ")";

        if(!mDevice->setCameraExposureTime(mMinExposureTime))
            return false;

        if(!mDevice->setCameraGain(mMinGain))
            return false;

    }

    // SET FPS.
    if(!mDevice->setCameraFPS())
        return false;

    // INIT CAMERA.
    if(!mDevice->initializeCamera())
        return false;

    // START CAMERA.
    if(!mDevice->startCamera())
        return false;

    return true;

}

