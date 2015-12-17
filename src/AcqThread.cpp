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

AcqThread::AcqThread(   boost::circular_buffer<Frame>       *fb,
                        boost::mutex                        *fb_m,
                        boost::condition_variable           *fb_c,
                        bool                                *sSignal,
                        boost::mutex                        *sSignal_m,
                        boost::condition_variable           *sSignal_c,
                        bool                                *dSignal,
                        boost::mutex                        *dSignal_m,
                        boost::condition_variable           *dSignal_c,
                        DetThread                           *detection,
                        StackThread                         *stack,
                        int                                 cid,
                        dataParam                           dp,
                        stackParam                          sp,
                        stationParam                        stp,
                        detectionParam                      dtp,
                        cameraParam                         acq,
                        framesParam                         fp,
                        videoParam                          vp,
                        fitskeysParam                       fkp) {

    frameBuffer             = fb;
    frameBuffer_mutex       = fb_m;
    frameBuffer_condition   = fb_c;
    stackSignal             = sSignal;
    stackSignal_mutex       = sSignal_m;
    stackSignal_condition   = sSignal_c;
    detSignal               = dSignal;
    detSignal_mutex         = dSignal_m;
    detSignal_condition     = dSignal_c;
    pDetection              = detection;
    pStack                  = stack;
    mThread                 = NULL;
    mMustStop               = false;
    mDevice                 = NULL;
    mThreadTerminated       = false;
    mNextAcqIndex           = 0;
    pExpCtrl                = NULL;
    mDeviceID               = cid;
    mdp                     = dp;
    msp                     = sp;
    mstp                    = stp;
    mdtp                    = dtp;
    mcp                     = acq;
    mvp                     = vp;
    mfp                     = fp;

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

    mMustStopMutex.lock();
    mMustStop = true;
    mMustStopMutex.unlock();

    if(mThread != NULL)
        while(mThread->timed_join(boost::posix_time::seconds(2)) == false)
            mThread->interrupt();

}

bool AcqThread::startThread() {

    // Create a device.
    mDevice = new Device(mcp, mfp, mvp, mDeviceID);

    // Search available devices.
    mDevice->listDevices(false);

    // CREATE CAMERA
    if(!mDevice->createCamera())
        return false;

    // Prepare continuous acquisition.
    if(!prepareAcquisitionOnDevice())
        return false;

    // Create acquisition thread.
    mThread = new boost::thread(boost::ref(*this));

    return true;

}

bool AcqThread::getThreadStatus(){

    return mThreadTerminated;

}

void AcqThread::operator()(){

    bool stop = false;

    BOOST_LOG_SCOPED_THREAD_TAG("LogName", "ACQ_THREAD");
    BOOST_LOG_SEV(logger,notification) << "\n";
    BOOST_LOG_SEV(logger,notification) << "==============================================";
    BOOST_LOG_SEV(logger,notification) << "========== START ACQUISITION THREAD ==========";
    BOOST_LOG_SEV(logger,notification) << "==============================================";

    try {

        // Search next acquisition according to the current time.
        selectNextAcquisitionSchedule(TimeDate::splitIsoExtendedDate(to_iso_extended_string(boost::posix_time::microsec_clock::universal_time())));

        // Exposure adjustment variables.
        bool exposureControlStatus = false;
        bool exposureControlActive = false;
        bool cleanStatus = false;

        // If exposure can be setted on the input device.
        if(mDevice->getExposureStatus()) {

            pExpCtrl = new ExposureControl( mcp.EXPOSURE_CONTROL_FREQUENCY,
                                            mcp.EXPOSURE_CONTROL_SAVE_IMAGE,
                                            mcp.EXPOSURE_CONTROL_SAVE_INFOS,
                                            mdp.DATA_PATH,
                                            mstp.STATION_NAME);
        }

        TimeMode previousTimeMode = NONE;

        /// Acquisition process.
        do {

            // Location of a video or frames if input type is FRAMES or VIDEO.
            string location = "";

            // Load videos file or frames directory if input type is FRAMES or VIDEO
            if(!mDevice->loadNextCameraDataSet(location)) break;

            if(pDetection != NULL) pDetection->setCurrentDataSet(location);

            // Reference time to compute interval between regular captures.
            string cDate = to_simple_string(boost::posix_time::microsec_clock::universal_time());
            string refDate = cDate.substr(0, cDate.find("."));

            do {

                // Container for the grabbed image.
                Frame newFrame;

                // Time counter of grabbing a frame.
                double tacq = (double)getTickCount();

                // Grab a frame.
                if(mDevice->runContinuousCapture(newFrame)) {

                    BOOST_LOG_SEV(logger, normal)   << "============= FRAME " << newFrame.mFrameNumber << " ============= ";
                    cout                            << "============= FRAME " << newFrame.mFrameNumber << " ============= " << endl;

                    // If camera type in input is FRAMES or VIDEO.
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
                        int twait = 100;
                        if(mvp.INPUT_TIME_INTERVAL == 0 && mfp.INPUT_TIME_INTERVAL > 0)
                            twait = mfp.INPUT_TIME_INTERVAL;
                        else if(mvp.INPUT_TIME_INTERVAL > 0 && mfp.INPUT_TIME_INTERVAL == 0)
                            twait = mvp.INPUT_TIME_INTERVAL;
                        #ifdef WINDOWS
                            Sleep(twait);
                        #else
                            #ifdef LINUX
                                usleep(twait * 1000);
                            #endif
                        #endif

                    }else {

                        // Get current time in seconds.
                        int currentTimeInSec = newFrame.mDate.hours * 3600 + newFrame.mDate.minutes * 60 + (int)newFrame.mDate.seconds;

                        // Detect day or night.
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

                                if(previousTimeMode != currentTimeMode && mdtp.DET_MODE != DAYNIGHT) {

                                    BOOST_LOG_SEV(logger, notification) << "TimeMode has changed ! ";
                                    boost::mutex::scoped_lock lock(*detSignal_mutex);
                                    *detSignal = false;
                                    lock.unlock();
                                    cout << "Send interruption signal to detection process " << endl;
                                    pDetection->interruptThread();

                                }else if(mdtp.DET_MODE == currentTimeMode || mdtp.DET_MODE == DAYNIGHT) {

                                    boost::mutex::scoped_lock lock2(*detSignal_mutex);
                                    *detSignal = true;
                                    detSignal_condition->notify_one();
                                    lock2.unlock();

                                }
                            }

                            // Notify stack thread.
                            if(pStack != NULL) {

                                // TimeMode has changed.
                                if(previousTimeMode != currentTimeMode && msp.STACK_MODE != DAYNIGHT) {

                                    BOOST_LOG_SEV(logger, notification) << "TimeMode has changed ! ";
                                    boost::mutex::scoped_lock lock(*stackSignal_mutex);
                                    *stackSignal = false;
                                    lock.unlock();

                                    // Force interruption.
                                    cout << "Send interruption signal to stack " << endl;
                                    pStack->interruptThread();

                                }else if(msp.STACK_MODE == currentTimeMode || msp.STACK_MODE == DAYNIGHT) {

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
                            exposureControlStatus = pExpCtrl->controlExposureTime(mDevice, newFrame.mImg, newFrame.mDate, mdtp.MASK, mDevice->mMinExposureTime, mcp.ACQ_FPS);

                        // Get current date YYYYMMDD.
                        string currentFrameDate =   TimeDate::getYYYYMMDD(newFrame.mDate);

                        // If the date has changed, sun ephemeris must be updated.
                        if(currentFrameDate != mCurrentDate) {

                            BOOST_LOG_SEV(logger, notification) << "Date has changed. Former Date is " << mCurrentDate << ". New Date is " << currentFrameDate << "." ;
                            computeSunTimes();

                        }

                        // Acquisition at regular time interval is enabled.
                        if(mcp.regcap.ACQ_REGULAR_ENABLED && !mDevice->mVideoFramesInput) {

                            cDate = to_simple_string(boost::posix_time::microsec_clock::universal_time());
                            string nowDate = cDate.substr(0, cDate.find("."));

                            boost::posix_time::ptime t1(boost::posix_time::time_from_string(refDate));
                            boost::posix_time::ptime t2(boost::posix_time::time_from_string(nowDate));

                            boost::posix_time::time_duration td = t2 - t1;
                            long secTime = td.total_seconds();
                            cout << "NEXT REGCAP : " << (int)(mcp.regcap.ACQ_REGULAR_CFG.interval - secTime) << "s" <<  endl;

                            // Check it's time to run a regular capture.
                            if(secTime >= mcp.regcap.ACQ_REGULAR_CFG.interval) {

                                // Current time is after the sunset stop and before the sunrise start = NIGHT
                                if((currentTimeMode == NIGHT) && (mcp.regcap.ACQ_REGULAR_MODE == NIGHT || mcp.regcap.ACQ_REGULAR_MODE == DAYNIGHT)) {

                                        BOOST_LOG_SEV(logger, notification) << "Run regular acquisition.";

                                        runImageCapture(    mcp.regcap.ACQ_REGULAR_CFG.rep,
                                                            mcp.regcap.ACQ_REGULAR_CFG.exp,
                                                            mcp.regcap.ACQ_REGULAR_CFG.gain,
                                                            mcp.regcap.ACQ_REGULAR_CFG.fmt,
                                                            mcp.regcap.ACQ_REGULAR_OUTPUT);

                                // Current time is between sunrise start and sunset stop = DAY
                                }else if(currentTimeMode == DAY && (mcp.regcap.ACQ_REGULAR_MODE == DAY || mcp.regcap.ACQ_REGULAR_MODE == DAYNIGHT)) {

                                    BOOST_LOG_SEV(logger, notification) << "Run regular acquisition.";
                                    saveImageCaptured(newFrame, 0, mcp.regcap.ACQ_REGULAR_OUTPUT);

                                }

                                // Reset reference time in case a long exposure has been done.
                                cDate = to_simple_string(boost::posix_time::microsec_clock::universal_time());
                                refDate = cDate.substr(0, cDate.find("."));

                            }

                        }

                        // Acquisiton at scheduled time is enabled.
                        if(mcp.schcap.ACQ_SCHEDULE.size() != 0 && mcp.schcap.ACQ_SCHEDULE_ENABLED && !mDevice->mVideoFramesInput) {

                            int next = (mNextAcq.hours * 3600 + mNextAcq.min * 60 + mNextAcq.sec) - (newFrame.mDate.hours * 3600 + newFrame.mDate.minutes * 60 + newFrame.mDate.seconds);

                            if(next < 0) {
                                next = (24 * 3600) - (newFrame.mDate.hours * 3600 + newFrame.mDate.minutes * 60 + newFrame.mDate.seconds) + (mNextAcq.hours * 3600 + mNextAcq.min * 60 + mNextAcq.sec);
                                cout << "next : " << next << endl;
                            }

                            vector<int>tsch = TimeDate::HdecimalToHMS(next/3600.0);

                            cout << "NEXT SCHCAP : " << tsch.at(0) << "h" << tsch.at(1) << "m" << tsch.at(2) << "s" <<  endl;

                            // It's time to run scheduled acquisition.
                            if( mNextAcq.hours == newFrame.mDate.hours &&
                                mNextAcq.min == newFrame.mDate.minutes &&
                                (int)newFrame.mDate.seconds == mNextAcq.sec) {

                                CamPixFmt format;
                                format = mNextAcq.fmt;

                                runImageCapture(    mNextAcq.rep,
                                                    mNextAcq.exp,
                                                    mNextAcq.gain,
                                                    format,
                                                    mcp.schcap.ACQ_SCHEDULE_OUTPUT);

                                // Update mNextAcq
                                selectNextAcquisitionSchedule(newFrame.mDate);

                            }else {

                                // The current time has elapsed.
                                if(newFrame.mDate.hours > mNextAcq.hours) {

                                   selectNextAcquisitionSchedule(newFrame.mDate);

                                }else if(newFrame.mDate.hours == mNextAcq.hours) {

                                    if(newFrame.mDate.minutes > mNextAcq.min) {

                                        selectNextAcquisitionSchedule(newFrame.mDate);

                                    }else if(newFrame.mDate.minutes == mNextAcq.min) {

                                        if(newFrame.mDate.seconds > mNextAcq.sec) {

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

                            // Print time before sunrise.
                            if(currentTimeInSec < mStartSunriseTime || currentTimeInSec > mStopSunsetTime ) {
                                vector<int> nextSunrise;
                                if(currentTimeInSec < mStartSunriseTime)
                                    nextSunrise = TimeDate::HdecimalToHMS((mStartSunriseTime - currentTimeInSec) / 3600.0);
                                if(currentTimeInSec > mStopSunsetTime)
                                    nextSunrise = TimeDate::HdecimalToHMS(((24*3600 - currentTimeInSec) + mStartSunriseTime ) / 3600.0);

                                cout << "NEXT SUNRISE : " << nextSunrise.at(0) << "h" << nextSunrise.at(1) << "m" << nextSunrise.at(2) << "s" << endl;
                            }

                            // Print time before sunset.
                            if(currentTimeInSec > mStopSunriseTime && currentTimeInSec < mStartSunsetTime){
                                vector<int> nextSunset;
                                nextSunset = TimeDate::HdecimalToHMS((mStartSunsetTime - currentTimeInSec) / 3600.0);
                                cout << "NEXT SUNSET : " << nextSunset.at(0) << "h" << nextSunset.at(1) << "m" << nextSunset.at(2) << "s" << endl;

                            }

                            // Reset exposure time when sunrise or sunset is finished.
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

                }

                tacq = (((double)getTickCount() - tacq)/getTickFrequency())*1000;
                std::cout << " [ TIME ACQ ] : " << tacq << " ms   ~cFPS("  << (1.0/(tacq/1000.0)) << ")" <<  endl;
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
                if(!pDetection->getRunStatus())
                    break;

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

    mThreadTerminated = true;

    std::cout << "Acquisition Thread TERMINATED." << endl;
    BOOST_LOG_SEV(logger,notification) << "Acquisition Thread TERMINATED";

}

void AcqThread::selectNextAcquisitionSchedule(TimeDate::Date date){

    if(mcp.schcap.ACQ_SCHEDULE.size() != 0){

        // Search next acquisition
        for(int i = 0; i < mcp.schcap.ACQ_SCHEDULE.size(); i++){

            if(date.hours < mcp.schcap.ACQ_SCHEDULE.at(i).hours){

               mNextAcqIndex = i;
               break;

            }else if(date.hours == mcp.schcap.ACQ_SCHEDULE.at(i).hours){

                if(date.minutes < mcp.schcap.ACQ_SCHEDULE.at(i).min){

                    mNextAcqIndex = i;
                    break;

                }else if(date.minutes == mcp.schcap.ACQ_SCHEDULE.at(i).min){

                    if(date.seconds < mcp.schcap.ACQ_SCHEDULE.at(i).sec){

                        mNextAcqIndex = i;
                        break;

                    }
                }
            }
        }

        mNextAcq = mcp.schcap.ACQ_SCHEDULE.at(mNextAcqIndex);

    }

}

bool AcqThread::buildAcquisitionDirectory(string YYYYMMDD){

    namespace fs = boost::filesystem;
    string root = mdp.DATA_PATH + mstp.STATION_NAME + "_" + YYYYMMDD +"/";

    string subDir = "captures/";
    string finalPath = root + subDir;

    mOutputDataPath = finalPath;
    BOOST_LOG_SEV(logger,notification) << "CompleteDataPath : " << mOutputDataPath;

    path p(mdp.DATA_PATH);
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

                // If fail to create DATA_PATH/STATION_YYYYMMDD/stack/
                if(!fs::create_directory(p2)){

                    BOOST_LOG_SEV(logger,critical) << "Unable to create captures directory : " << p2.string();
                    return false;

                // If success to create DATA_PATH/STATION_YYYYMMDD/stack/
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

void AcqThread::runImageCapture(int imgNumber, int imgExposure, int imgGain, CamPixFmt imgFormat, ImgFormat imgOutput) {

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
        EParser<CamPixFmt> format;
        BOOST_LOG_SEV(logger, notification) << "Format : " << format.getStringEnum(imgFormat);
        frame.mFormat = imgFormat;

        if(mcp.ACQ_RES_CUSTOM_SIZE) {
            frame.mHeight = mcp.ACQ_HEIGHT;
            frame.mWidth = mcp.ACQ_WIDTH;
        }

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

    // RECREATE CAMERA
    if(!mDevice->recreateCamera())
        throw "Fail to restart camera.";

    prepareAcquisitionOnDevice();

}

void AcqThread::saveImageCaptured(Frame &img, int imgNum, ImgFormat outputType) {

    if(img.mImg.data) {

        string  YYYYMMDD = TimeDate::getYYYYMMDD(img.mDate);

        if(buildAcquisitionDirectory(YYYYMMDD)) {

            string fileName = "CAP_" + TimeDate::getYYYYMMDDThhmmss(img.mDate) + "_UT-" + Conversion::intToString(imgNum);

            switch(outputType) {

                case JPEG :

                    {

                        switch(img.mFormat) {

                            case MONO12 :

                                {

                                    Mat temp;
                                    img.mImg.copyTo(temp);
                                    Mat newMat = ImgProcessing::correctGammaOnMono12(temp, 2.2);
                                    Mat newMat2 = Conversion::convertTo8UC1(newMat);
                                    SaveImg::saveJPEG(newMat2, mOutputDataPath + fileName);

                                }

                                break;

                            default :

                                {

                                    Mat temp;
                                    img.mImg.copyTo(temp);
                                    Mat newMat = ImgProcessing::correctGammaOnMono8(temp, 2.2);
                                    SaveImg::saveJPEG(newMat, mOutputDataPath + fileName);

                                }

                        }
                    }

                    break;

                case FITS :

                    {

                        Fits2D newFits(mOutputDataPath);
                        newFits.loadKeys(mfkp, mstp);
                        newFits.kGAINDB = img.mGain;
                        newFits.kEXPOSURE = img.mExposure/1000000.0;
                        newFits.kONTIME = img.mExposure/1000000.0;
                        newFits.kELAPTIME = img.mExposure/1000000.0;
                        newFits.kDATEOBS = TimeDate::getIsoExtendedFormatDate(img.mDate);

                        double  debObsInSeconds = img.mDate.hours*3600 + img.mDate.minutes*60 + img.mDate.seconds;
                        double  julianDate      = TimeDate::gregorianToJulian(img.mDate);
                        double  julianCentury   = TimeDate::julianCentury(julianDate);

                        newFits.kCRVAL1 = TimeDate::localSideralTime_2(julianCentury, img.mDate.hours, img.mDate.minutes, (int)img.mDate.seconds, mstp.SITELONG);
                        newFits.kCTYPE1 = "RA---ARC";
                        newFits.kCTYPE2 = "DEC--ARC";
                        newFits.kEQUINOX = 2000.0;

                        switch(img.mFormat) {

                            case MONO12 :

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
                                        cout << ">> Fits saved in : " << mOutputDataPath << fileName << endl;

                                }

                                break;

                            default :

                                {

                                   if(newFits.writeFits(img.mImg, UC8, fileName))
                                        cout << ">> Fits saved in : " << mOutputDataPath << fileName << endl;

                                }

                        }

                    }

                    break;

            }

        }
    }

}

bool AcqThread::computeSunTimes() {

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

    if(mcp.ephem.EPHEMERIS_ENABLED) {

        Ephemeris ephem1 = Ephemeris(mCurrentDate, mcp.ephem.SUN_HORIZON_1, mstp.SITELONG, mstp.SITELAT);

        if(!ephem1.computeEphemeris(sunriseStartH, sunriseStartM,sunsetStopH, sunsetStopM)) {

            return false;

        }

        Ephemeris ephem2 = Ephemeris(mCurrentDate, mcp.ephem.SUN_HORIZON_2, mstp.SITELONG, mstp.SITELAT );

        if(!ephem2.computeEphemeris(sunriseStopH, sunriseStopM,sunsetStartH, sunsetStartM)) {

            return false;

        }

    }else {

        sunriseStartH = mcp.ephem.SUNRISE_TIME.at(0);
        sunriseStartM = mcp.ephem.SUNRISE_TIME.at(1);

        double intpart1 = 0;
        double fractpart1 = modf((double)mcp.ephem.SUNRISE_DURATION/3600.0 , &intpart1);

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

        sunsetStartH = mcp.ephem.SUNSET_TIME.at(0);
        sunsetStartM = mcp.ephem.SUNSET_TIME.at(1);

        double intpart3 = 0;
        double fractpart3 = modf((double)mcp.ephem.SUNSET_DURATION/3600.0 , &intpart3);

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

bool AcqThread::prepareAcquisitionOnDevice() {


    // SET SIZE
    if(!mDevice->setCameraSize())
        return false;

    // SET FORMAT
    if(!mDevice->setCameraPixelFormat())
        return false;

    // LOAD GET BOUNDS
    mDevice->getCameraExposureBounds();
    mDevice->getCameraGainBounds();

    // Get Sunrise start/stop, Sunset start/stop. ---
    computeSunTimes();

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
        BOOST_LOG_SEV(logger, notification) << "EXPOSURE TIME   :  Minimum (" << mDevice->mMinExposureTime << ")"<< mDevice->getNightExposureTime();
        BOOST_LOG_SEV(logger, notification) << "GAIN            :  Minimum (" << mDevice->mMinGain << ")";

        if(!mDevice->setCameraExposureTime(mDevice->mMinExposureTime))
            return false;

        if(!mDevice->setCameraGain(mDevice->mMinGain))
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

