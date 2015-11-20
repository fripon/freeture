/*
                            StackThread.cpp

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
* \file    StackThread.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    19/06/2014
* \brief   Stack frames.
*/

#include "StackThread.h"

boost::log::sources::severity_logger< LogSeverityLevel >  StackThread::logger;

StackThread::Init StackThread::initializer;

StackThread::StackThread(   string                          cfgPath,
                            bool                            *sS,
                            boost::mutex                    *sS_m,
                            boost::condition_variable       *sS_c,
                            boost::circular_buffer<Frame>   *fb,
                            boost::mutex                    *fb_m,
                            boost::condition_variable       *fb_c){

    thread = NULL;
    mustStop = false;
    frameBuffer = fb;
    frameBuffer_mutex = fb_m;
    frameBuffer_condition = fb_c;
    stackSignal = sS;
    stackSignal_mutex = sS_m;
    stackSignal_condition = sS_c;
    completeDataPath = "";
    isRunning = false;
    interruptionStatus = false;

    Configuration cfg;

    if(!cfg.Load(cfgPath))
        throw "Fail to load parameters for stackThread from configuration file.";

    if(!cfg.Get("FITS_COMPRESSION", mFitsCompression)) {

        mFitsCompression = false;
        mFitsCompressionMethod = "";
        BOOST_LOG_SEV(logger, warning) << "Fail to load FITS_COMPRESSION from configuration file. Set to false.";

    }else{

        if(mFitsCompression) {

            if(!cfg.Get("FITS_COMPRESSION_METHOD", mFitsCompressionMethod)) {
                mFitsCompressionMethod = "[compress]";
                BOOST_LOG_SEV(logger, warning) << "Fail to load FITS_COMPRESSION_METHOD from configuration file. Set to [compress].";
            }

        }else{

            mFitsCompressionMethod = "";

        }
    }

    // Get camera format.
    string acq_bit_depth;
    if(!cfg.Get("ACQ_BIT_DEPTH", acq_bit_depth)){
        throw "Fail to load ACQ_BIT_DEPTH from configuration file !";
    }
    EParser<CamBitDepth> cam_bit_depth;
    mAcqBitDepth = cam_bit_depth.parseEnum("ACQ_BIT_DEPTH", acq_bit_depth);
    BOOST_LOG_SEV(logger,notification) << "Load ACQ_BIT_DEPTH : " << acq_bit_depth;

    // Get time of stacking frames.
    if(!cfg.Get("STACK_TIME", mStackTime)){
        throw "Fail to load STACK_TIME from configuration file !";
    }

    // Get time to wait before a new stack.
    if(!cfg.Get("STACK_INTERVAL", mStackInterval)){
        throw "Fail to load STACK_INTERVAL from configuration file !";
    }

    // Get stack method to use.
    string stack_method;
    if(!cfg.Get("STACK_MTHD", stack_method)){
        stack_method = "SUM";
        BOOST_LOG_SEV(logger,notification) << "Fail to load STACK_MTHD from configuration file. Set to SUM.";
    }
    EParser<StackMeth> stack_mth;
    mStackMthd = stack_mth.parseEnum("STACK_MTHD", stack_method);

    // Use reduction method or not.
    if(!cfg.Get("STACK_REDUCTION", mStackReduction)){
        mStackReduction = true;
        BOOST_LOG_SEV(logger,notification) << "Fail to load STACK_REDUCTION from configuration file. Set to true.";
    }

    // Get station name.
    if(!cfg.Get("STATION_NAME", mStationName)){
        mStationName = "STATION";
        BOOST_LOG_SEV(logger,notification) << "Fail to load STATION_NAME from configuration file. Set to STATION.";
    }

    // Get location where to store data.
    if(!cfg.Get("DATA_PATH", mDataPath))
        throw "Fail to load DATA_PATH from configuration file !";

    // Get fits keywords.
    mFitsHeader.loadKeywordsFromConfigFile(cfgPath);

}

StackThread::~StackThread(void){

    BOOST_LOG_SEV(logger,notification) << "Cleaning ressources and deleting StackThread...";

    if(thread!=NULL)
        delete thread;

}

bool StackThread::startThread(){

    BOOST_LOG_SEV(logger,notification) << "Creating StackThread...";
    thread = new boost::thread(boost::ref(*this));
    return true;
}

void StackThread::stopThread(){

    // Signal the thread to stop (thread-safe)
    mustStopMutex.lock();
    mustStop = true;
    mustStopMutex.unlock();

    while(thread->timed_join(boost::posix_time::seconds(1)) == false) {
        thread->interrupt();
    }

    interruptThread();

}

bool StackThread::interruptThread(){

    interruptionStatusMutex.lock();
    interruptionStatus = true;
    BOOST_LOG_SEV(logger,notification) << "StackThread interruption.";
    interruptionStatusMutex.unlock();
    return true;

}

bool StackThread::buildStackDataDirectory(TimeDate::Date date){

    namespace fs = boost::filesystem;
    string YYYYMMDD = TimeDate::getYYYYMMDD(date);
    string root = mDataPath + mStationName + "_" + YYYYMMDD +"/";
    string subDir = "astro/";
    string finalPath = root + subDir;
    completeDataPath = finalPath;

    if(YYYYMMDD == "00000000")
        return false;


    BOOST_LOG_SEV(logger,notification) << "Stack data path : " << completeDataPath;

    path p(mDataPath);
    path p1(root);
    path p2(root + subDir);

    // If DATA_PATH exists
    if(fs::exists(p)){

        // If DATA_PATH/STATION_YYYYMMDD/ exists
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

    return true;
}

bool StackThread::getRunStatus(){

    return isRunning;

}

void StackThread::operator()(){

    bool stop = false;
    isRunning = true;

    BOOST_LOG_SCOPED_THREAD_TAG("LogName", "STACK_THREAD");
    BOOST_LOG_SEV(logger,notification) << "\n";
    BOOST_LOG_SEV(logger,notification) << "==============================================";
    BOOST_LOG_SEV(logger,notification) << "============== Start stack thread ============";
    BOOST_LOG_SEV(logger,notification) << "==============================================";

    try{

        do{

            try {

                // Thread is sleeping...
                boost::this_thread::sleep(boost::posix_time::millisec(mStackInterval*1000));

                // Create a stack to accumulate n frames.
                Stack frameStack = Stack(mFitsCompressionMethod);

                // First reference date.
                boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
                string cDate = to_simple_string(time);
                string dateDelimiter = ".";
                string refDate = cDate.substr(0, cDate.find(dateDelimiter));

                long secTime = 0;

                do {

                    // Communication with AcqThread. Wait for a new frame.
                    boost::mutex::scoped_lock lock(*stackSignal_mutex);
                    while(!(*stackSignal)) stackSignal_condition->wait(lock);
                    *stackSignal = false;
                    lock.unlock();

                    double t = (double)getTickCount();

                    // Check interruption signal from AcqThread.
                    bool forceToSave = false;
                    interruptionStatusMutex.lock();
                    if(interruptionStatus) forceToSave = true;
                    interruptionStatusMutex.unlock();

                    if(!forceToSave){

                        // Fetch last frame grabbed.
                        boost::mutex::scoped_lock lock2(*frameBuffer_mutex);
                        if(frameBuffer->size() == 0) {
                            throw "SHARED CIRCULAR BUFFER SIZE = 0 -> STACK INTERRUPTION.";
                        }
                        Frame newFrame = frameBuffer->back();
                        lock2.unlock();

                        // Add the new frame to the stack.
                        frameStack.addFrame(newFrame);

                        t = (((double)getTickCount() - t)/getTickFrequency())*1000;
                        std::cout << "[ TIME STACK ] : " << std::setprecision(5) << std::fixed << t << " ms" << endl;
                        BOOST_LOG_SEV(logger,normal) << "[ TIME STACK ] : " << std::setprecision(5) << std::fixed << t << " ms" ;

                    }else{

                        // Interruption is active.
                        BOOST_LOG_SEV(logger,notification) << "Interruption status : " << forceToSave;

                        // Interruption operations terminated. Rest interruption signal.
                        interruptionStatusMutex.lock();
                        interruptionStatus = false;
                        interruptionStatusMutex.unlock();

                        break;

                    }

                    time = boost::posix_time::microsec_clock::universal_time();
                    cDate = to_simple_string(time);
                    string nowDate = cDate.substr(0, cDate.find(dateDelimiter));
                    boost::posix_time::ptime t1(boost::posix_time::time_from_string(refDate));
                    boost::posix_time::ptime t2(boost::posix_time::time_from_string(nowDate));
                    boost::posix_time::time_duration td = t2 - t1;
                    secTime = td.total_seconds();
                    cout << "NEXT STACK : " << (int)(mStackTime - secTime) << "s" <<  endl;

                }while(secTime <= mStackTime);

                // Stack finished. Save it.
                if(buildStackDataDirectory(frameStack.getDateFirstFrame())) {

                    if(!frameStack.saveStack(mFitsHeader, completeDataPath, mStackMthd, mStationName, mStackReduction)){

                        BOOST_LOG_SEV(logger,fail) << "Fail to save stack.";

                    }

                    BOOST_LOG_SEV(logger,notification) << "Stack saved : " << completeDataPath;

                }else{

                    BOOST_LOG_SEV(logger,fail) << "Fail to build stack directory. ";

                }

            }catch(const boost::thread_interrupted&){

                BOOST_LOG_SEV(logger,notification) << "Stack thread INTERRUPTED";
                cout << "Stack thread INTERRUPTED" << endl;

            }

            // Get the "must stop" state (thread-safe)
            mustStopMutex.lock();
            stop = mustStop;
            mustStopMutex.unlock();

        }while(!stop);

    }catch(const char * msg){

        BOOST_LOG_SEV(logger,critical) << msg;

    }catch(exception& e){

        BOOST_LOG_SEV(logger, critical) << e.what();

    }

    isRunning = false;

    BOOST_LOG_SEV(logger,notification) << "StackThread ended.";

}

