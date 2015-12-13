/*
                                StackThread.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*   This file is part of:   freeture
*
*   Copyright:      (C) 2014-2015 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
*   Last modified:      20/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    StackThread.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    19/06/2014
* \brief   Stack frames.
*/

#pragma once

#include "config.h"

#ifdef LINUX
    #define BOOST_LOG_DYN_LINK 1
#endif

#include <iostream>
#include "EStackMeth.h"
#include "Stack.h"
#include "Fits.h"
#include "Fits2D.h"
#include "TimeDate.h"
#include "EParser.h"
#include <boost/filesystem.hpp>
#include <boost/circular_buffer.hpp>
#include <assert.h>
#include "SParam.h"

using namespace boost::filesystem;
using namespace std;
using namespace cv;
using namespace boost::posix_time;

class StackThread {

    private :

        static boost::log::sources::severity_logger< LogSeverityLevel > logger;

        static class Init {

            public :

                Init() {

                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("StackThread"));

                }

        }initializer;

        boost::thread   *mThread;
        bool            mustStop;
        boost::mutex    mustStopMutex;
        bool            isRunning;
        bool            interruptionStatus;
        boost::mutex    interruptionStatusMutex;
 
        boost::condition_variable       *frameBuffer_condition;
        boost::mutex                    *frameBuffer_mutex;
        boost::circular_buffer<Frame>   *frameBuffer;
        bool                            *stackSignal;
        boost::mutex                    *stackSignal_mutex;
        boost::condition_variable       *stackSignal_condition;

        stationParam    mstp;
        fitskeysParam   mfkp;
        dataParam       mdp;
        stackParam      msp;
        CamPixFmt       mPixfmt;

        string completeDataPath;

    public :

        StackThread(    bool                            *sS,
                        boost::mutex                    *sS_m,
                        boost::condition_variable       *sS_c,
                        boost::circular_buffer<Frame>   *fb,
                        boost::mutex                    *fb_m,
                        boost::condition_variable       *fb_c,
                        dataParam       dp,
                        stackParam      sp,
                        stationParam    stp,
                        CamPixFmt       pfmt,
                        fitskeysParam   fkp);

        ~StackThread(void);

        bool startThread();

        void stopThread();

        void operator()();

        bool getRunStatus();

        bool interruptThread();

    private :

        bool buildStackDataDirectory(TimeDate::Date date);

};
