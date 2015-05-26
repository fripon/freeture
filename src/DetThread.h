/*
                                DetThread.h

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
*	Last modified:		20/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    DetThread.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Detection thread.
*/

#pragma once

#include "config.h"

#ifdef LINUX
    #define BOOST_LOG_DYN_LINK 1
#endif

#include "SMTPClient.h"
#include <iterator>
#include "Fits.h"
#include "Fits2D.h"
#include "TimeDate.h"
#include "Fits3D.h"
#include "Stack.h"
#include "Detection.h"
#include "DetectionTemporal.h"
#include "DetectionTemplate.h"
#include "EStackMeth.h"
#include "EDetMeth.h"
#include <boost/circular_buffer.hpp>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;
using namespace cv;
using namespace std;
using namespace boost::posix_time;

class DetThread {

    private :

        static boost::log::sources::severity_logger< LogSeverityLevel > logger;

        static class Init{

            public :

                Init() {

                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("DetThread"));

                }

        }initializer;

        boost::thread                   *pThread;                   // Pointer on detection thread.
        Detection                       *pDetMthd;                  // Pointer on detection method.
        bool                            mMustStop;
        boost::mutex                    mMustStopMutex;
        bool                            mSaveAvi;                   // Save event as avi                (parameter from configuration file).
        bool                            mSaveFits3D;                // Save event as fits 3D            (parameter from configuration file).
        bool                            mSaveFits2D;                // Save event as fits 2D            (parameter from configuration file).
        bool                            mSaveSum;                   // Save sum of the event            (parameter from configuration file).
        double                          mTimeBeforeEvent;           // Time to keep before an event     (parameter from configuration file).
        double                          mTimeAfterEvent;            // Time to keep after an event      (parameter from configuration file).
        string                          mDataPath;                  // Where to save data               (parameter from configuration file).
        string                          mStationName;               // Name of the station              (parameter from configuration file).
        CamBitDepth                     mBitDepth;                  // Acquisition bit depth            (parameter from configuration file).
        Fits                            mFitsHeader;
        bool                            mMailAlertEnabled;          // Enable to send mail alert        (parameter from configuration file).
        string                          mMailSmtpServer;            // SMTP server                      (parameter from configuration file).
        string                          mMailSmtpHostname;          // SMTP hostname                    (parameter from configuration file).
        vector<string>                  mMailRecipients;            // Mail recipients                  (parameter from configuration file).
        bool                            mStackReduction;            // Reduce sum fits to 16 bits.      (parameter from configuration file).
        StackMeth                       mStackMthd;                 // Reduction method                 (parameter from configuration file).
        DetMeth                         mDetMthd;                   // Enumeration of the detection method used.
        bool                            mIsRunning;                 // Detection thread running status.
        bool                            mWaitFramesToCompleteEvent;
        int                             mNbWaitFrames;
        string                          mCfgPath;
        string                          mEventPath;                 // Path of the last detected event.
        string                          mEventDate;                 // Date of the last detected event.
        int                             mNbDetection;               // Number of detection.
        bool                            mInterruptionStatus;
        boost::mutex                    mInterruptionStatusMutex;
        boost::circular_buffer<Frame>   *frameBuffer;
        boost::mutex                    *frameBuffer_mutex;
        boost::condition_variable       *frameBuffer_condition;
        bool                            *detSignal;
        boost::mutex                    *detSignal_mutex;
        boost::condition_variable       *detSignal_condition;

    public :

        /**
        * Constructor.
        *
        */
        DetThread(  string                          cfg_p,
                    DetMeth                         m,
                    boost::circular_buffer<Frame>   *fb,
                    boost::mutex                    *fb_m,
                    boost::condition_variable       *fb_c,
                    bool                            *dSignal,
                    boost::mutex                    *dSignal_m,
                    boost::condition_variable       *dSignal_c);

        /**
        * Destructor.
        *
        */
        ~DetThread();

        /**
        * Detection Thread loop.
        *
        */
        void operator()();

        /**
        * Start thread.
        *
        * @return Success status to start thread.
        */
        bool startThread();

        /**
        * Stop thread.
        *
        */
        void stopThread();

        /**
        * Build directory "events".
        *
        * @return Success to create directory.
        */
        bool buildEventDataDirectory();

        /**
        * Save an event in the directory "events".
        *
        * @param firstEvPosInFB First frame's number of the event.
        * @param lastEvPosInFB Last frame's number of the event.
        * @return Success to save an event.
        */
        bool saveEventData(int firstEvPosInFB, int lastEvPosInFB);

        /**
        * Load detection thread parameters.
        *
        * @return Success to load parameters.
        */
        bool loadDetThreadParameters();

        /**
        * Run status of detection thread.
        *
        * @return Is running or not.
        */
        bool getRunStatus();

        /**
        * Get detection method used by detection thread.
        *
        * @return Detection method.
        */
        Detection* getDetMethod();

        /**
        * Interrupt detection thread.
        *
        */
        void interruptThread();

};


