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

AcqThread::AcqThread(   CamType                             camType,
                        string                              cfg,
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

                        mCameraType(camType), mCfgPath(cfg), frameBuffer(fb), frameBuffer_mutex(fb_m),
                        frameBuffer_condition(fb_c), stackSignal(sSignal), stackSignal_mutex(sSignal_m),
                        stackSignal_condition(sSignal_c), detSignal(dSignal), detSignal_mutex(dSignal_m),
                        detSignal_condition(dSignal_c), pDetection(detection), pStack(stack), mThread(NULL),
                        mMustStop(false), mDevice(NULL), mNbGrabFail(0), mNbGrabSuccess(0), mThreadEndStatus(false),
                        mNextAcqIndex(0), pExpCtrl(NULL) {

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

bool AcqThread::startThread(){

    mDevice = new Device(mCameraType, mCfgPath);

    if(!mDevice->prepareDevice()){

        BOOST_LOG_SEV(logger,fail) << "Fail to prepare device.";
        return false;

    }

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

        mAcqScheduledList = mDevice->getSchedule();     // Get acquisition schedule.
        sortAcquisitionSchedule();                      // Order schedule times.

        boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();

        // Search next acquisition according to the current time.
        selectNextAcquisitionSchedule(TimeDate::splitIsoExtendedDate(to_iso_extended_string(time)));

        bool scheduleTaskStatus = false;
        bool scheduleTaskActive = false;

        /// Prepare acquisition at regular time interval.

        int regularAcqFrameInterval = 0;
        int regularAcqFrameCounter = 0;

        if(mDevice->getAcqRegularEnabled()) {

            double fps = 0.0;
            mDevice->getCam()->getFPS(fps);

            if(fps>0){

                regularAcqFrameInterval = mDevice->getAcqRegularTimeInterval() * fps;

            }else{

                BOOST_LOG_SEV(logger, critical) << "Error : Camera fps value is " << fps;
                throw ">> Error on fps value.";

            }

        }

        /// Exposure adjustment variables.

        bool exposureControlStatus = false;
        bool exposureControlActive = false;
        bool cleanStatus = false;

        if(!mDevice->getVideoFramesInput()) {

            pExpCtrl = new ExposureControl( mDevice->getExposureControlFrequency(),
                                            mDevice->getExposureControlSaveImage(),
                                            mDevice->getExposureControlSaveInfos(),
                                            mDevice->getDataPath(),
                                            mDevice->getStationName());
        }

        TimeMode previousTimeMode = NONE;

        /// Acquisition process.

        do {

            // Location of a video or frames if input type is FRAMES or VIDEO.
            string location = "";

            // Load videos file or frames directory if input type is FRAMES or VIDEO
            if(!mDevice->getCam()->loadNextDataSet(location)) break;

            if(pDetection != NULL) pDetection->setCurrentDataSet(location);

            do {

                // Container for the grabbed image.
                Frame newFrame;

                // Time counter of grabbing a frame.
                double tacq = (double)getTickCount();

                // Grab a frame.
                if(mDevice->getCam()->grabImage(newFrame)) {

                    BOOST_LOG_SEV(logger, normal)   << "============= FRAME " << newFrame.mFrameNumber << " ============= ";
                    cout                            << "============= FRAME " << newFrame.mFrameNumber << " ============= " << endl;

                    // Camera type in input is FRAMES or VIDEO.
                    if(mDevice->getVideoFramesInput()) {

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
                            Sleep(1000);
                        #else
                            #ifdef LINUX
                                sleep(1);
                            #endif
                        #endif

                    }else {

                        // Get current time in seconds.
                        int currentTimeInSec = newFrame.mDate.hours * 3600 + newFrame.mDate.minutes * 60 + (int)newFrame.mDate.seconds;

                        // Detect day or night status.
                        TimeMode currentTimeMode = NONE;

                        if((currentTimeInSec > mDevice->mStopSunsetTime) || (currentTimeInSec < mDevice->mStartSunriseTime)) {
                            currentTimeMode = NIGHT;
                        }else if((currentTimeInSec > mDevice->mStartSunriseTime) && (currentTimeInSec < mDevice->mStopSunsetTime)) {
                            currentTimeMode = DAY;
                        }

                        EParser<TimeMode> mode;
                        cout << "MODE : " << mode.getStringEnum(currentTimeMode) << endl;

                        // If exposure control is not active, the new frame can be shared with others threads.
                        if(!exposureControlStatus) {

                            // Push the new frame in the framebuffer.
                            boost::mutex::scoped_lock lock(*frameBuffer_mutex);
                            frameBuffer->push_back(newFrame);
                            lock.unlock();

                            // Notify detection thread.
                            if(pDetection != NULL) {

                                if(previousTimeMode != NONE && previousTimeMode != currentTimeMode && mDevice->mDetectionMode != DAYNIGHT) {

                                    BOOST_LOG_SEV(logger, notification) << "TimeMode has changed ! ";
                                    boost::mutex::scoped_lock lock(*detSignal_mutex);
                                    *detSignal = false;
                                    lock.unlock();
                                    cout << "Send interruption signal to detection process " << endl;
                                    pDetection->interruptThread();

                                }else if(mDevice->mDetectionMode == currentTimeMode || mDevice->mDetectionMode == DAYNIGHT) {

                                    boost::mutex::scoped_lock lock2(*detSignal_mutex);
                                    *detSignal = true;
                                    detSignal_condition->notify_one();
                                    lock2.unlock();

                                }
                            }

                            // Notify stack thread.
                            if(pStack != NULL) {

                                // TimeMode has changed.
                                if(previousTimeMode != NONE && previousTimeMode != currentTimeMode && mDevice->mStackMode != DAYNIGHT) {

                                    BOOST_LOG_SEV(logger, notification) << "TimeMode has changed ! ";
                                    boost::mutex::scoped_lock lock(*stackSignal_mutex);
                                    *stackSignal = false;
                                    lock.unlock();

                                    // Force interruption.
                                    cout << "Send interruption signal to stack " << endl;
                                    pStack->interruptThread();

                                }else if(mDevice->mStackMode == currentTimeMode || mDevice->mStackMode == DAYNIGHT) {

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
                            exposureControlStatus = pExpCtrl->controlExposureTime(mDevice, newFrame.mImg, newFrame.mDate);

                        // Get current date YYYYMMDD.
                        string currentFrameDate =   TimeDate::getYYYYMMDD(newFrame.mDate);

                        // If the date has changed, sun ephemeris must be updated.
                        if(currentFrameDate != mDevice->mCurrentDate) {

                            BOOST_LOG_SEV(logger, notification) << "Date has changed. Former Date is " << mDevice->mCurrentDate << ". New Date is " << currentFrameDate << "." ;
                            mDevice->getSunTimes();

                        }

                        // Acquisition at regular time interval is enabled.
                        if(mDevice->getAcqRegularEnabled()) {

                            // Current time is after the sunset stop and before the sunrise start = NIGHT
                            if((currentTimeMode == NIGHT) && (mDevice->mRegularMode == NIGHT || mDevice->mRegularMode == DAYNIGHT)) {

                                // Check it's time to run a regular capture.
                                if(regularAcqFrameCounter >= regularAcqFrameInterval) {

                                    BOOST_LOG_SEV(logger, notification) << "Run regular acquisition.";

                                    runImageCapture(    mDevice->getAcqRegularRepetition(),
                                                        mDevice->getAcqRegularExposure(),
                                                        mDevice->getAcqRegularGain(),
                                                        mDevice->getAcqRegularFormat(),
                                                        mDevice->mRegularOutput);

                                    regularAcqFrameCounter = 0;

                                }else {

                                    //cout << "Next regular acquisition in : " << regularAcqFrameInterval - regularAcqFrameCounter << " frames." << endl;
                                    regularAcqFrameCounter++;

                                }

                            // Current time is between sunrise start and sunset stop = DAY
                            }else if(currentTimeMode == DAY && (mDevice->mRegularMode == DAY || mDevice->mRegularMode == DAYNIGHT)) {

                                // Check if it's time to run a regular capture.
                                if(regularAcqFrameCounter >= regularAcqFrameInterval) {
                                     cout << "Run regular acquisition." << endl;
                                    saveImageCaptured(newFrame, 0, mDevice->mRegularOutput);
                                    regularAcqFrameCounter = 0;

                                }else {

                                    //cout << "Next regular acquisition in : " << regularAcqFrameInterval - regularAcqFrameCounter << " frames." << endl;
                                    regularAcqFrameCounter++;

                                }

                            }else{

                                regularAcqFrameCounter = 0;

                            }

                        }

                        // Acquisiton at scheduled time is enabled.
                        if(mAcqScheduledList.size() != 0 && mDevice->getAcqScheduleEnabled()) {

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
                                                    mDevice->mScheduleOutput);

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
                        if( ((currentTimeInSec > mDevice->mStartSunriseTime && currentTimeInSec < mDevice->mStopSunriseTime) ||
                            (currentTimeInSec > mDevice->mStartSunsetTime && currentTimeInSec < mDevice->mStopSunsetTime))) {

                            exposureControlActive = true;

                            //BOOST_LOG_SEV(logger, notification) << "SUNSET or SUNRISE detected. ";

                        }else {

                            if(exposureControlActive) {

                                // In DAYTIME : Apply minimum available exposure time.
                                if((currentTimeInSec >= mDevice->mStopSunriseTime && currentTimeInSec < mDevice->mStartSunsetTime)){

                                    BOOST_LOG_SEV(logger, notification) << "Apply day exposure time : " << mDevice->getDayExposureTime();
                                    mDevice->getCam()->setExposureTime(mDevice->getDayExposureTime());
                                    BOOST_LOG_SEV(logger, notification) << "Apply day exposure time : " << mDevice->getDayGain();
                                    mDevice->getCam()->setGain(mDevice->getDayGain());

                                // In NIGHTTIME : Apply maximum available exposure time.
                                }else if((currentTimeInSec >= mDevice->mStopSunsetTime) || (currentTimeInSec < mDevice->mStartSunriseTime)){

                                    BOOST_LOG_SEV(logger, notification) << "Apply night exposure time." << mDevice->getNightExposureTime();
                                    mDevice->getCam()->setExposureTime(mDevice->getNightExposureTime());
                                    BOOST_LOG_SEV(logger, notification) << "Apply night exposure time." << mDevice->getNightGain();
                                    mDevice->getCam()->setGain(mDevice->getNightGain());

                                }
                            }

                            exposureControlActive = false;
                            exposureControlStatus = false;

                        }

                    }

                }else {

                    BOOST_LOG_SEV(logger, fail) << "Fail to grab frame " << newFrame.mFrameNumber;
                    mNbGrabFail++;

                }

                tacq = (((double)getTickCount() - tacq)/getTickFrequency())*1000;
                std::cout << " [ TIME ACQ ] : " << tacq << " ms" << endl;
                BOOST_LOG_SEV(logger, normal) << " [ TIME ACQ ] : " << tacq << " ms";

                if(!mDevice->getVideoFramesInput() && tacq > 60.0)
                    BOOST_LOG_SEV(logger, warning) << "FRAME " << newFrame.mFrameNumber << "  [ TIME ACQ ] : " << tacq << " ms";

                mMustStopMutex.lock();
                stop = mMustStop;
                mMustStopMutex.unlock();

            }while(stop == false && !mDevice->getCam()->getStopStatus());

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

        }while(mDevice->getCam()->getDataSetStatus());

    }catch(const boost::thread_interrupted&){

        BOOST_LOG_SEV(logger,notification) << "Acquisition Thread INTERRUPTED";
        cout << "Acquisition Thread INTERRUPTED" <<endl;

    }catch(exception& e){

        cout << "An exception occured : " << e.what() << endl;
        BOOST_LOG_SEV(logger, critical) << "An exception occured : " << e.what();

    }catch(const char * msg) {

        cout << endl << msg << endl;

    }

    mDevice->getCam()->acqStop();
    mDevice->getCam()->grabCleanse();

    mThreadEndStatus = true;

    std::cout << "Acquisition Thread TERMINATED." << endl;
    BOOST_LOG_SEV(logger,notification) << "Acquisition Thread TERMINATED";

}

void AcqThread::selectNextAcquisitionSchedule(TimeDate::Date date){

    if(mAcqScheduledList.size() != 0){

        // Search next acquisition
        for(int i = 0; i < mAcqScheduledList.size(); i++){

            if(date.hours < mAcqScheduledList.at(i).getH()){

               mNextAcqIndex = i;
               break;

            }else if(date.hours == mAcqScheduledList.at(i).getH()){

                if(date.minutes < mAcqScheduledList.at(i).getM()){

                    mNextAcqIndex = i;
                    break;

                }else if(date.minutes == mAcqScheduledList.at(i).getM()){

                    if(date.seconds < mAcqScheduledList.at(i).getS()){

                        mNextAcqIndex = i;
                        break;

                    }
                }
            }
        }

        mNextAcq = mAcqScheduledList.at(mNextAcqIndex);

    }

}

void AcqThread::sortAcquisitionSchedule(){

    if(mAcqScheduledList.size() != 0){

        // Sort time in list.
        vector<AcqSchedule> tempSchedule;

        do{

            int minH; int minM; int minS; bool init = false;

            vector<AcqSchedule>::iterator it;
            vector<AcqSchedule>::iterator it_select;

            for(it = mAcqScheduledList.begin(); it != mAcqScheduledList.end(); ++it){

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
                mAcqScheduledList.erase(it_select);

            }

        }while(mAcqScheduledList.size() != 0);

        mAcqScheduledList = tempSchedule;

    }

}

bool AcqThread::buildAcquisitionDirectory(string YYYYMMDD){

    namespace fs = boost::filesystem;
    string root = mDevice->getDataPath() + mDevice->getStationName() + "_" + YYYYMMDD +"/";

    string subDir = "captures/";
    string finalPath = root + subDir;

    mDataLocation = finalPath;
    BOOST_LOG_SEV(logger,notification) << "CompleteDataPath : " << mDataLocation;

    path p(mDevice->getDataPath());
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

    return false;
}

void AcqThread::runImageCapture(int imgNumber, int imgExposure, int imgGain, CamBitDepth imgFormat, ImgFormat imgOutput) {

    // Stop camera
    BOOST_LOG_SEV(logger, notification) << "Stopping camera...";
    mDevice->getCam()->acqStop();
    mDevice->getCam()->grabCleanse();

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
        if(mDevice->getCam()->grabSingleImage(frame, mDevice->getCameraId())) {

            BOOST_LOG_SEV(logger, notification) << "Single capture succeed !";
            cout << "Single capture succeed !" << endl;
            saveImageCaptured(frame, i, imgOutput);

        }else{

            BOOST_LOG_SEV(logger, fail) << "Single capture failed !";

        }

        mDevice->getCam()->acqStop();
        mDevice->getCam()->grabCleanse();
    }

    #ifdef WINDOWS
        Sleep(1000);
    #else
        #ifdef LINUX
            sleep(1);
        #endif
    #endif

    BOOST_LOG_SEV(logger, notification) << "Restarting camera in continuous mode...";
    mDevice->runContinuousAcquisition();

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
                        newFits.copyKeywords(mDevice->getFitsHeader());
                        newFits.kGAINDB = img.mGain;
                        newFits.kEXPOSURE = img.mExposure/1000000.0;
                        newFits.kONTIME = img.mExposure/1000000.0;
                        newFits.kELAPTIME = img.mExposure/1000000.0;
                        newFits.kDATEOBS = TimeDate::getIsoExtendedFormatDate(img.mDate);

                        double  debObsInSeconds = img.mDate.hours*3600 + img.mDate.minutes*60 + img.mDate.seconds;
                        double  julianDate      = TimeDate::gregorianToJulian(img.mDate);
                        double  julianCentury   = TimeDate::julianCentury(julianDate);

                        newFits.kCRVAL1 = TimeDate::localSideralTime_2(julianCentury, img.mDate.hours, img.mDate.minutes, (int)img.mDate.seconds, mDevice->getFitsHeader().kSITELONG);
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

