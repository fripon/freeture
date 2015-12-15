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

StackThread::StackThread(   bool                            *sS,
                            boost::mutex                    *sS_m,
                            boost::condition_variable       *sS_c,
                            boost::circular_buffer<Frame>   *fb,
                            boost::mutex                    *fb_m,
                            boost::condition_variable       *fb_c,
                            dataParam dp,
                            stackParam sp,
                            stationParam stp,
                            CamPixFmt pfmt,
                            fitskeysParam fkp){

    mThread = NULL;
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
    mdp = dp;
    mstp = stp;
    msp = sp;
    mfkp = fkp;
    mPixfmt = pfmt;

}

StackThread::~StackThread(void){

    BOOST_LOG_SEV(logger,notification) << "Cleaning ressources and deleting StackThread...";

    if(mThread!=NULL)
        delete mThread;

}

bool StackThread::startThread(){

    BOOST_LOG_SEV(logger,notification) << "Creating StackThread...";
    mThread = new boost::thread(boost::ref(*this));
    return true;
}

void StackThread::stopThread(){

    // Signal the thread to stop (thread-safe)
    mustStopMutex.lock();
    mustStop = true;
    mustStopMutex.unlock();

    if(mThread != NULL)
        while(mThread->timed_join(boost::posix_time::seconds(1)) == false)
            mThread->interrupt();

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
    string root = mdp.DATA_PATH + mstp.STATION_NAME + "_" + YYYYMMDD +"/";
    string subDir = "stacks/";
    string finalPath = root + subDir;
    completeDataPath = finalPath;

    if(YYYYMMDD == "00000000")
        return false;


    BOOST_LOG_SEV(logger,notification) << "Stacks data path : " << completeDataPath;

    path p(mdp.DATA_PATH);
    path p1(root);
    path p2(root + subDir);

    // If DATA_PATH exists
    if(fs::exists(p)){

        // If DATA_PATH/STATION_YYYYMMDD/ exists
        if(fs::exists(p1)){

            // If DATA_PATH/STATION_YYYYMMDD/stacks/ doesn't exists
            if(!fs::exists(p2)){

                // If fail to create DATA_PATH/STATION_YYYYMMDD/stacks/
                if(!fs::create_directory(p2)){

                    BOOST_LOG_SEV(logger,critical) << "Unable to create stacks directory : " << p2.string();
                    return false;

                // If success to create DATA_PATH/STATION_YYYYMMDD/stacks/
                }else{

                   BOOST_LOG_SEV(logger,notification) << "Success to create stacks directory : " << p2.string();
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

                // If fail to create DATA_PATH/STATION_YYYYMMDD/stacks/
                if(!fs::create_directory(p2)){

                    BOOST_LOG_SEV(logger,critical) << "Unable to create stacks directory : " << p2.string();
                    return false;

                // If success to create DATA_PATH/STATION_YYYYMMDD/stacks/
                }else{

                    BOOST_LOG_SEV(logger,notification) << "Success to create stacks directory : " << p2.string();
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

                // If fail to create DATA_PATH/STATION_YYYYMMDD/stacks/
                if(!fs::create_directory(p2)){

                    BOOST_LOG_SEV(logger,critical) << "Unable to create stacks directory : " << p2.string();
                    return false;

                // If success to create DATA_PATH/STATION_YYYYMMDD/stacks/
                }else{

                    BOOST_LOG_SEV(logger,notification) << "Success to create stacks directory : " << p2.string();
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
                boost::this_thread::sleep(boost::posix_time::millisec(msp.STACK_INTERVAL*1000));

                // Create a stack to accumulate n frames.
                Stack frameStack = Stack(mdp.FITS_COMPRESSION_METHOD, mfkp, mstp);

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
                    cout << "NEXT STACK : " << (int)(msp.STACK_TIME - secTime) << "s" <<  endl;

                }while(secTime <= msp.STACK_TIME);

                // Stack finished. Save it.
                if(buildStackDataDirectory(frameStack.getDateFirstFrame())) {

                    if(!frameStack.saveStack(completeDataPath, msp.STACK_MTHD, msp.STACK_REDUCTION)){

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

