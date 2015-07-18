/*
								AcqThread.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
*	Last modified:		21/01/2015
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
                        DetThread	                        *detection,
                        StackThread	                        *stack):

                        mCameraType(camType), mCfgPath(cfg), frameBuffer(fb), frameBuffer_mutex(fb_m),
                        frameBuffer_condition(fb_c), stackSignal(sSignal), stackSignal_mutex(sSignal_m),
                        stackSignal_condition(sSignal_c), detSignal(dSignal), detSignal_mutex(dSignal_m),
                        detSignal_condition(dSignal_c), pDetection(detection), pStack(stack), mThread(NULL),
                        mMustStop(false), mDevice(NULL), mNbGrabFail(0), mNbGrabSuccess(0), mThreadEndStatus(false),
                        mNextAcqIndex(0), pExpCtrl(NULL), mStackThreadStatus(false){

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

    // Wait for the thread to finish.
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

    BOOST_LOG_SEV(logger,normal) << "Sucess to prepare device.";
    BOOST_LOG_SEV(logger,normal) << "Create acquisition thread.";

    mThread = new boost::thread(boost::ref(*this));

    return true;

}

bool AcqThread::getThreadEndStatus(){

    return mThreadEndStatus;

}

void AcqThread::operator()(){

    bool stop = false;
    vector<string> frameDate;   // Date in string vector : YYYY  MM  DD  hh  mm  ss
    string accurateFrameDate;   // Date in string : YYYY-MM-DDTHH:MM:SS,fffffffff

    BOOST_LOG_SCOPED_THREAD_TAG("LogName", "ACQ_THREAD");
    BOOST_LOG_SEV(logger,notification) << "\n";
    BOOST_LOG_SEV(logger,notification) << "==============================================";
    BOOST_LOG_SEV(logger,notification) << "========== Start acquisition thread ==========";
    BOOST_LOG_SEV(logger,notification) << "==============================================";

    /// Prepare scheduled long acquisition.

    mAcqScheduledList = mDevice->getSchedule();     // Get acquisition schedule.
    sortAcquisitionSchedule();                      // Order schedule times.
    selectNextAcquisitionSchedule();                // Search next acquisition according to the current time.

    bool scheduleTaskStatus = false;
    bool scheduleTaskActive = false;

    /// Prepare acquisition at regular time interval.

    int regularAcqFrameInterval = 0;
    int regularAcqFrameCounter = 0;

    if(mDevice->getAcqRegularEnabled())
        regularAcqFrameInterval = mDevice->getAcqRegularTimeInterval() * mDevice->getCam()->getFPS();

    /// Exposure adjustment variables.

    bool exposureControlStatus = false;
    bool exposureControlActive = false;
    bool cleanStatus = false;

    if(mDevice->getAcqDayEnabled())

        pExpCtrl = new ExposureControl( mDevice->getExposureControlFrequency(),
                                        mDevice->getExposureControlSaveImage(),
                                        mDevice->getExposureControlSaveInfos(),
                                        mDevice->getDataPath(),
                                        mDevice->getStationName());

    /// Enable or disable stack thread in daytime.

    if(pStack != NULL)
        mStackThreadStatus = true;
    else
        mStackThreadStatus = false;

    /// Acquisition process.

    try {

        do {

            // Load videos file or frames directory if input type is : FRAMES or VIDEO
            string location = "";
            if(!mDevice->getCam()->loadNextDataSet(location)) break;

            if(pDetection != NULL) pDetection->setCurrentDataSet(location);

            do {

                Frame newFrame;
                bool grabStatus = false;

                double tacq = (double)getTickCount();

                if(mDevice->getCam()->grabImage(newFrame)){

                    if(mDevice->getVideoFramesInput()) {

                        #ifdef WINDOWS
                            Sleep(1000);
                        #else
                            #ifdef LINUX
                            sleep(1);
                            #endif
                        #endif

                    }

                    grabStatus = true;

                    BOOST_LOG_SEV(logger, normal) << "============= FRAME " << newFrame.getNumFrame() << " ============= ";
                    cout << "============= FRAME " << newFrame.getNumFrame() << " ============= " << endl;

                    // Get date.
                    frameDate = newFrame.getDateString();
                    accurateFrameDate = newFrame.getAcqDateMicro();

                    // Check if exposure control is active.
                    if(!exposureControlStatus){

                        // Exposure control is not active, the new frame can be shared with others threads.

                        boost::mutex::scoped_lock lock(*frameBuffer_mutex);
                        frameBuffer->push_back(newFrame);
                        lock.unlock();

                        boost::mutex::scoped_lock lock2(*detSignal_mutex);
                        *detSignal = true;
                        detSignal_condition->notify_one();
                        lock2.unlock();

                        if(mStackThreadStatus){

                            boost::mutex::scoped_lock lock3(*stackSignal_mutex);
                            *stackSignal = true;
                            stackSignal_condition->notify_one();
                            lock3.unlock();

                        }

                        cleanStatus = false;

                    }else{

                        // Exposure control is active, the new frame can't be shared with others threads.
                        if(!cleanStatus){

                            // If stack process exists.
                            if(pStack != NULL){

                                boost::mutex::scoped_lock lock(*stackSignal_mutex);
                                *stackSignal = false;
                                lock.unlock();

                                // Force interruption.
                                cout << "Send interruption signal to stack " << endl;
                                pStack->interruptThread();

                            }

                            // If detection process exists
                            if(pDetection != NULL){

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

                    if(pExpCtrl != NULL && exposureControlActive)
                        exposureControlStatus = pExpCtrl->controlExposureTime(mDevice, newFrame.getImg(), accurateFrameDate);

                }else{

                    BOOST_LOG_SEV(logger, notification) << ">> Fail to grab frame";
                    mNbGrabFail++;

                }

                if(grabStatus && !mDevice->getVideoFramesInput()){


                    //! CHECK IF EPHEMERIS NEED TO BE UPDATED.

                    string currentFrameDate = frameDate.at(0) + frameDate.at(1) + frameDate.at(2);

                    // If the date has evolved, ephemeris must be updated.
                    if(currentFrameDate != mDevice->mCurrentDate) {

                        BOOST_LOG_SEV(logger, notification) << "Date has changed. Former Date is " << mDevice->mCurrentDate << ". New Date is " << currentFrameDate << "." ;

                        mDevice->getSunTimes();

                    }

                    //! CHECK IF IT'S TIME TO RUN REGULAR / SCHEDULED CAPTURES.

                    int currentTimeInSec = atoi(frameDate.at(3).c_str()) * 3600 + atoi(frameDate.at(4).c_str()) * 60 + atoi(frameDate.at(5).c_str());

                    // If acquisition at regular time interval is enabled.
                    if(mDevice->getAcqRegularEnabled()) {

                        // Current time is after the sunset stop and before the sunrise start.
                        if(((currentTimeInSec > mDevice->mStopSunsetTime) || (currentTimeInSec < mDevice->mStartSunriseTime)) &&
                            (mDevice->mRegularMode == NIGHT || mDevice->mRegularMode == DAYNIGHT)) {

                            // Check it's time to run a regular capture.
                            if(regularAcqFrameCounter >= regularAcqFrameInterval) {

                                BOOST_LOG_SEV(logger, notification) << "Run regular acquisition.";
                                runRegularAcquisition(accurateFrameDate);

                                #ifdef LINUX
                                    sleep(1);
                                #else
                                    #ifdef WINDOWS
                                        Sleep(1000);
                                    #endif
                                #endif

                                regularAcqFrameCounter = 0;

                            }else {

                                cout << "Next regular acquisition in : " << regularAcqFrameInterval - regularAcqFrameCounter << " frames." << endl;
                                regularAcqFrameCounter++;

                            }

                        //}else if() {



                        }else{

                            regularAcqFrameCounter = 0;

                        }

                    }

                    // Check schedule for long exposure time captures.
                    if(mAcqScheduledList.size() != 0 && mDevice->getAcqScheduleEnabled()){

                        // Time for a long exposure time acquisition.
                        if(mNextAcq.getH() == atoi(frameDate.at(3).c_str()) && mNextAcq.getM() == atoi(frameDate.at(4).c_str()) && atoi(frameDate.at(5).c_str()) == mNextAcq.getS()){

                            mNextAcq.setDate(accurateFrameDate);

                            // Launch single acquisition
                            bool result = runScheduledAcquisition(mNextAcq);

                            #ifdef LINUX
                                sleep(1);
                            #else
                                #ifdef LINUX
                                    Sleep(1);
                                #endif
                            #endif

                            // Update mNextAcq
                            selectNextAcquisitionSchedule();

                        }else{

                            // The current time elapsed.
                            if(atoi(frameDate.at(3).c_str()) > mNextAcq.getH()){

                               selectNextAcquisitionSchedule();

                            }else if(atoi(frameDate.at(3).c_str()) == mNextAcq.getH()){

                                if(atoi(frameDate.at(4).c_str()) > mNextAcq.getM()){

                                    selectNextAcquisitionSchedule();

                                }else if(atoi(frameDate.at(4).c_str()) == mNextAcq.getM()){

                                    if(atoi(frameDate.at(5).c_str()) > mNextAcq.getS()){

                                        selectNextAcquisitionSchedule();

                                    }

                                }

                            }

                        }

                    }

                    //! CHECK IF IT'S DAYTIME OR NOT IN ORDER TO APPLY THE CORRECT GAIN AND EXPOSURE VALUES.

                    // If day acquisition enabled, enable or disable exposure control if it's sunset or sunrise.
                    if(mDevice->getAcqDayEnabled()){

                        // Check sunrise and sunset time.
                        if( ((currentTimeInSec > mDevice->mStartSunriseTime && currentTimeInSec < mDevice->mStopSunriseTime) ||
                            (currentTimeInSec > mDevice->mStartSunsetTime && currentTimeInSec < mDevice->mStopSunsetTime)) &&
                            mDevice->mStartSunriseTime != 0 && mDevice->mStopSunriseTime != 0 && mDevice->mStartSunsetTime != 0 && mDevice->mStopSunsetTime !=0){

                            exposureControlActive = true;
                            cout << "SUNSET or SUNRISE ! "<< endl;
                            BOOST_LOG_SEV(logger, notification) << "SUNSET or SUNRISE ! ";

                        }else{

                            if(exposureControlActive){

                                if((currentTimeInSec >= mDevice->mStopSunriseTime && currentTimeInSec < mDevice->mStartSunsetTime)){

                                    cout << "DAYTIME ! "<< endl;
                                    BOOST_LOG_SEV(logger, notification) << "Apply day exposure time.";
                                    cout << mDevice->getDayExposureTime()<< endl;
                                    mDevice->getCam()->setExposureTime(mDevice->getDayExposureTime());
                                    cout << mDevice->getDayGain()<< endl;
                                    mDevice->getCam()->setGain(mDevice->getDayGain());


                                }else if((currentTimeInSec >= mDevice->mStopSunsetTime) || (currentTimeInSec < mDevice->mStartSunriseTime)){

                                    cout << "NIGHT ! "<< endl;
                                    BOOST_LOG_SEV(logger, notification) << "Apply night exposure time.";
                                    cout << mDevice->getNightExposureTime()<< endl;
                                    mDevice->getCam()->setExposureTime(mDevice->getNightExposureTime());
                                    cout << mDevice->getNightGain()<< endl;
                                    mDevice->getCam()->setGain(mDevice->getNightGain());
                                }

                            }

                            exposureControlActive = false;
                            exposureControlStatus = false;

                        }
                    }


                }

                tacq = (((double)getTickCount() - tacq)/getTickFrequency())*1000;
                std::cout << " [ TIME ACQ ] : " << tacq << " ms" << endl;
                BOOST_LOG_SEV(logger, normal) << " [ TIME ACQ ] : " << tacq << " ms";

                mMustStopMutex.lock();
                stop = mMustStop;
                mMustStopMutex.unlock();

            }while(stop == false && !mDevice->getCam()->getStopStatus());

            // Reset detection process to prepare the analyse of a new data set.
            if(pDetection != NULL) {

                BOOST_LOG_SEV(logger, normal) << " RESET DETECTION";
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

    }

    mDevice->getCam()->acqStop();
    mDevice->getCam()->grabCleanse();

    mThreadEndStatus = true;

    std::cout << "Acquisition Thread terminated." << endl;
    BOOST_LOG_SEV(logger,notification) << "Acquisition Thread TERMINATED";

}

void AcqThread::selectNextAcquisitionSchedule(){

    if(mAcqScheduledList.size() != 0){

        /// Search next acquisition according to the current date
        // Get current date.
        string currentDate = TimeDate::localDateTime(microsec_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");
        cout << endl << "current date : " << currentDate << endl;
        vector<string> currentDateSplit;

        // Split date
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(":");
        tokenizer tokens(currentDate, sep);
        for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
            currentDateSplit.push_back(*tok_iter);
        }

        // Get current Hour.
        int currentH = atoi(currentDateSplit.at(3).c_str());

        // Get current Minutes.
        int currentM = atoi(currentDateSplit.at(4).c_str());

        // Get current Seconds.
        int currentS = atoi(currentDateSplit.at(5).c_str());

        // Search next acquisition
        for(int i = 0; i < mAcqScheduledList.size(); i++){

            if(currentH < mAcqScheduledList.at(i).getH()){

               mNextAcqIndex = i;
               break;

            }else if(currentH == mAcqScheduledList.at(i).getH()){

                if(currentM < mAcqScheduledList.at(i).getM()){

                    mNextAcqIndex = i;
                    break;

                }else if(currentM == mAcqScheduledList.at(i).getM()){

                    if(currentS < mAcqScheduledList.at(i).getS()){

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

            // If DATA_PATH/STATION_YYYYMMDD/astro/ doesn't exists
            if(!fs::exists(p2)){

                // If fail to create DATA_PATH/STATION_YYYYMMDD/astro/
                if(!fs::create_directory(p2)){

                    BOOST_LOG_SEV(logger,critical) << "Unable to create astro directory : " << p2.string();
                    return false;

                // If success to create DATA_PATH/STATION_YYYYMMDD/astro/
                }else{

                   BOOST_LOG_SEV(logger,notification) << "Success to create astro directory : " << p2.string();
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

                    BOOST_LOG_SEV(logger,critical) << "Unable to create astro directory : " << p2.string();
                    return false;

                // If success to create DATA_PATH/STATION_YYYYMMDD/astro/
                }else{

                    BOOST_LOG_SEV(logger,notification) << "Success to create astro directory : " << p2.string();
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

                // If fail to create DATA_PATH/STATION_YYYYMMDD/astro/
                if(!fs::create_directory(p2)){

                    BOOST_LOG_SEV(logger,critical) << "Unable to create astro directory : " << p2.string();
                    return false;

                // If success to create DATA_PATH/STATION_YYYYMMDD/astro/
                }else{

                    BOOST_LOG_SEV(logger,notification) << "Success to create astro directory : " << p2.string();
                    return true;

                }
            }
        }
    }
}

bool AcqThread::runScheduledAcquisition(AcqSchedule task){

    // Stop camera
    cout << "Stopping camera..." << endl;
    mDevice->getCam()->acqStop();
    mDevice->getCam()->grabCleanse();

    // If stack process exists.
    if(pStack != NULL && mStackThreadStatus){

        boost::mutex::scoped_lock lock(*stackSignal_mutex);
        *stackSignal = false;
        lock.unlock();

        // Force interruption.
        cout << "Send interruption signal to stack " << endl;
        pStack->interruptThread();


    }

    // If detection process exists
    if(pDetection != NULL){

        boost::mutex::scoped_lock lock(*detSignal_mutex);
        *detSignal = false;
        lock.unlock();
        cout << "Sending interruption signal to detection process " << endl;
        pDetection->interruptThread();

    }

    // Reset framebuffer.
    cout << "Cleaning frameBuffer..." << endl;
    boost::mutex::scoped_lock lock(*frameBuffer_mutex);
    frameBuffer->clear();
    lock.unlock();

    for(int i = 0; i < task.getN(); i++){

        // Configuration pour single capture
        Frame frame;
        cout << "Exposure : " << task.getE() << endl;
        frame.setExposure(task.getE());
        cout << "Gain : " << task.getG() << endl;
        frame.setGain(task.getG());
        CamBitDepth camFormat;
        cout << "Format : " << task.getF() << endl;
        Conversion::intBitDepthToCamBitDepthEnum(task.getF(), camFormat);
        frame.setBitDepth(camFormat);

        // Single capture.
        if(mDevice->getCam()->grabSingleImage(frame, mDevice->getCameraId())){

            cout << endl << "Single capture succeed !" << endl;

            /// ---------------------- Save grabbed frame --------------------------

            // Save the frame in Fits 2D.
            if(frame.getImg().data){

                string YYYYMMDD = TimeDate::getYYYYMMDDfromDateString(task.getDate());
                cout << "YYYYMMDD : " << YYYYMMDD << endl;
                if(buildAcquisitionDirectory(YYYYMMDD)){

                    cout << "Saving fits file ..." << endl;

                    Fits2D newFits(mDataLocation);
                    newFits.copyKeywords(mDevice->getFitsHeader());
                    newFits.kGAINDB = task.getG();
                    newFits.kONTIME = task.getE()/1000000.0;
                    newFits.kDATEOBS = frame.getAcqDateMicro();

                    vector<int> firstDateInt = TimeDate::getIntVectorFromDateString(task.getDate());
                    double  debObsInSeconds = firstDateInt.at(3)*3600 + firstDateInt.at(4)*60 + firstDateInt.at(5);
                    double  julianDate      = TimeDate::gregorianToJulian(firstDateInt);
                    double  julianCentury   = TimeDate::julianCentury(julianDate);

                    // Sideral time.
                    newFits.kCRVAL1 = TimeDate::localSideralTime_2(julianCentury, firstDateInt.at(3), firstDateInt.at(4), firstDateInt.at(5), mDevice->getFitsHeader().kSITELONG);
                    newFits.kCTYPE1 = "RA---ARC";
                    newFits.kCTYPE2 = "DEC--ARC";
                    newFits.kEQUINOX = 2000.0;

                    string HHMMSS = Conversion::numbering(2, task.getH()) + Conversion::intToString(task.getH()) +
                                    Conversion::numbering(2, task.getM()) + Conversion::intToString(task.getM()) +
                                    Conversion::numbering(2, task.getS()) + Conversion::intToString(task.getS());

                    string fileName = "CAP_" + YYYYMMDD + "T" + HHMMSS + "_UT-" + Conversion::intToString(i);

                    switch(camFormat){

                        case MONO_8 :

                            {
                                // Create FITS image with BITPIX = BYTE_IMG (8-bits unsigned integers), pixel with TBYTE (8-bit unsigned byte)

                                switch(mDevice->mScheduleOutput) {

                                    case JPEG :

                                        {

                                            Mat temp;
                                            frame.getImg().copyTo(temp);
                                            Mat newMat = ImgProcessing::correctGammaOnMono8(temp, 2.2);
                                            SaveImg::saveJPEG(newMat, mDataLocation + fileName);

                                        }

                                        break;

                                    case FITS :

                                        {

                                            if(newFits.writeFits(frame.getImg(), UC8, fileName))
                                                cout << ">> Fits saved in : " << mDataLocation << fileName << endl;

                                        }

                                        break;

                                }

                            }

                            break;

                        case MONO_12 :

                            {

                                // Convert unsigned short type image in short type image.
                                Mat newMat = Mat(frame.getImg().rows, frame.getImg().cols, CV_16SC1, Scalar(0));

                                // Set bzero and bscale for print unsigned short value in soft visualization.
                                double bscale = 1;
                                double bzero  = 32768;
                                newFits.kBZERO = bzero;
                                newFits.kBSCALE = bscale;

                                unsigned short * ptr;
                                short * ptr2;

                                for(int i = 0; i < frame.getImg().rows; i++){

                                    ptr = frame.getImg().ptr<unsigned short>(i);
                                    ptr2 = newMat.ptr<short>(i);

                                    for(int j = 0; j < frame.getImg().cols; j++){

                                        if(ptr[j] - 32768 > 32767){

                                            ptr2[j] = 32767;

                                        }else{

                                            ptr2[j] = ptr[j] - 32768;
                                        }
                                    }
                                }

                                switch(mDevice->mRegularOutput) {

                                    case JPEG :

                                        {

                                            Mat temp;
                                            frame.getImg().copyTo(temp);
                                            Mat newMat = ImgProcessing::correctGammaOnMono12(temp, 2.2);
                                            Mat newMat2 = Conversion::convertTo8UC1(newMat);
                                            SaveImg::saveJPEG(newMat2, mDataLocation + fileName);

                                        }

                                        break;

                                    case FITS :

                                        {

                                            // Create FITS image with BITPIX = SHORT_IMG (16-bits signed integers), pixel with TSHORT (signed short)
                                            if(newFits.writeFits(newMat, S16, fileName))
                                                cout << ">> Fits saved in : " << mDataLocation << fileName << endl;

                                        }

                                        break;

                                }

                            }
                    }
                }

            }



        }else{

            cout << endl << "Single capture failed !" << endl;

        }


    }

    cout<< "Restarting camera in continuous mode..." << endl;
    mDevice->runContinuousAcquisition();

    return true;
}

bool AcqThread::runRegularAcquisition(string frameDate){

    // Stop camera
    cout << "Stopping camera..." << endl;
    mDevice->getCam()->acqStop();
    mDevice->getCam()->grabCleanse();

    // If stack process exists.
    if(pStack != NULL && mStackThreadStatus){

        boost::mutex::scoped_lock lock(*stackSignal_mutex);
        *stackSignal = false;
        lock.unlock();

        // Force interruption.
        cout << "Send interruption signal to stack " << endl;
        pStack->interruptThread();


    }

    // If detection process exists
    if(pDetection != NULL){

        boost::mutex::scoped_lock lock(*detSignal_mutex);
        *detSignal = false;
        lock.unlock();
        cout << "Send interruption signal to detection process " << endl;
        pDetection->interruptThread();

    }

    // Reset framebuffer.
    cout << "Cleaning frameBuffer..." << endl;
    boost::mutex::scoped_lock lock(*frameBuffer_mutex);
    frameBuffer->clear();
    lock.unlock();

    for(int i = 0; i < mDevice->getAcqRegularRepetition(); i++){

        // Configuration pour single capture
        Frame frame;
        cout << "Exposure : " << mDevice->getAcqRegularExposure() << endl;
        frame.setExposure(mDevice->getAcqRegularExposure());
        cout << "Gain : " << mDevice->getAcqRegularGain() << endl;
        frame.setGain(mDevice->getAcqRegularGain());
        cout << "Format : " << mDevice->getAcqRegularFormat() << endl;
        frame.setBitDepth(mDevice->getAcqRegularFormat());

        // Single capture.
        if(mDevice->getCam()->grabSingleImage(frame, mDevice->getCameraId())){

            cout << endl << "Single capture succeed !" << endl;

            /// ---------------------- Save grabbed frame --------------------------

            // Save the frame in Fits 2D.
            if(frame.getImg().data){

                string	YYYYMMDD = TimeDate::getYYYYMMDDfromDateString(frameDate);
                cout << "YYYYMMDD : " << YYYYMMDD << endl;
                if(buildAcquisitionDirectory(YYYYMMDD)){

                    cout << "Saving fits file ..." << endl;

                    cout << "mDataLocation : " << mDataLocation << endl;

                    Fits2D newFits(mDataLocation);
                    newFits.copyKeywords(mDevice->getFitsHeader());
                    newFits.kGAINDB = mDevice->getAcqRegularGain();
                    newFits.kONTIME = mDevice->getAcqRegularExposure()/1000000.0;
                    newFits.kDATEOBS = frame.getAcqDateMicro();

                    vector<int> firstDateInt = TimeDate::getIntVectorFromDateString(frameDate);
                    double  debObsInSeconds = firstDateInt.at(3)*3600 + firstDateInt.at(4)*60 + firstDateInt.at(5);
                    double  julianDate      = TimeDate::gregorianToJulian(firstDateInt);
                    double  julianCentury   = TimeDate::julianCentury(julianDate);

                    newFits.kCRVAL1 = TimeDate::localSideralTime_2(julianCentury, firstDateInt.at(3), firstDateInt.at(4), firstDateInt.at(5), mDevice->getFitsHeader().kSITELONG);

                    newFits.kCTYPE1 = "RA---ARC";
                    newFits.kCTYPE2 = "DEC--ARC";
                    newFits.kEQUINOX = 2000.0;

                    string HHMMSS = Conversion::numbering(2, firstDateInt.at(3)) + Conversion::intToString(firstDateInt.at(3)) +
                                    Conversion::numbering(2, firstDateInt.at(4)) + Conversion::intToString(firstDateInt.at(4)) +
                                    Conversion::numbering(2, firstDateInt.at(5)) + Conversion::intToString(firstDateInt.at(5));

                    string fileName = "CAP_" + YYYYMMDD + "T" + HHMMSS + "_UT-" + Conversion::intToString(i);

                    cout << "fileName : " << fileName << endl;

                    switch(mDevice->getAcqRegularFormat()){

                        case MONO_8 :

                            {
                                // Create FITS image with BITPIX = BYTE_IMG (8-bits unsigned integers), pixel with TBYTE (8-bit unsigned byte)

                                switch(mDevice->mRegularOutput) {

                                    case JPEG :

                                        {

                                            Mat temp;
                                            frame.getImg().copyTo(temp);
                                            Mat newMat = ImgProcessing::correctGammaOnMono8(temp, 2.2);
                                            SaveImg::saveJPEG(newMat, mDataLocation + fileName);

                                        }

                                        break;

                                    case FITS :

                                        {

                                            if(newFits.writeFits(frame.getImg(), UC8, fileName))
                                                cout << ">> Fits saved in : " << mDataLocation << fileName << endl;

                                        }

                                        break;

                                }

                            }

                            break;

                        case MONO_12 :

                            {

                                // Convert unsigned short type image in short type image.
                                Mat newMat = Mat(frame.getImg().rows, frame.getImg().cols, CV_16SC1, Scalar(0));

                                // Set bzero and bscale for print unsigned short value in soft visualization.
                                newFits.kBZERO = 1;
                                newFits.kBSCALE = 32768;

                                unsigned short * ptr;
                                short * ptr2;

                                for(int i = 0; i < frame.getImg().rows; i++){

                                    ptr = frame.getImg().ptr<unsigned short>(i);
                                    ptr2 = newMat.ptr<short>(i);

                                    for(int j = 0; j < frame.getImg().cols; j++){

                                        if(ptr[j] - 32768 > 32767){

                                            ptr2[j] = 32767;

                                        }else{

                                            ptr2[j] = ptr[j] - 32768;
                                        }
                                    }
                                }

                                switch(mDevice->mRegularOutput) {

                                    case JPEG :

                                        {

                                            Mat temp;
                                            frame.getImg().copyTo(temp);
                                            Mat newMat = ImgProcessing::correctGammaOnMono12(temp, 2.2);
                                            Mat newMat2 = Conversion::convertTo8UC1(newMat);
                                            SaveImg::saveJPEG(newMat2, mDataLocation + fileName);

                                        }

                                        break;

                                    case FITS :

                                        {

                                            // Create FITS image with BITPIX = SHORT_IMG (16-bits signed integers), pixel with TSHORT (signed short)
                                            if(newFits.writeFits(newMat, S16, fileName))
                                                cout << ">> Fits saved in : " << mDataLocation << fileName << endl;

                                        }

                                        break;

                                }

                          }
                    }
                }
            }

        }else{

            cout << endl << "Single capture failed !" << endl;

        }
    }

    cout<< "Restarting camera in continuous mode..." << endl;
    mDevice->runContinuousAcquisition();

	return true;

}


