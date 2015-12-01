/*
                                AcqThread.h

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
*   Last modified:      21/01/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    AcqThread.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    21/01/2015
* \brief   Acquisition thread.
*/

#ifndef ACQTHREAD_H
#define ACQTHREAD_H

#include "config.h"

#ifdef LINUX
    #define BOOST_LOG_DYN_LINK 1
#endif

#include "ECamPixFmt.h"
#include "AcqSchedule.h"
#include "DetThread.h"
#include "StackThread.h"
#include "Device.h"
#include "ExposureControl.h"
#include "ImgProcessing.h"
#include "EImgFormat.h"
#include "Ephemeris.h"

using namespace cv;
using namespace std;

class AcqThread {

    private :

        static boost::log::sources::severity_logger< LogSeverityLevel > logger;

        static class Init {

            public:

                Init() {

                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("AcqThread"));

                }

        }initializer;

        bool                mMustStop;              // Signal to stop thread.
        boost::mutex        mMustStopMutex;
        boost::thread       *mThread;               // Acquisition thread.
        Device              *mDevice;               // Input device.
        int                 mNbGrabFail;            // Fail number to grab frames.
        int                 mNbGrabSuccess;         // Success number to grab frames.
        bool                mThreadEndStatus;
        AcqSchedule         mNextAcq;               // Next scheduled acquisition.
        int                 mNextAcqIndex;
        DetThread           *pDetection;            // Pointer on detection thread.
        StackThread         *pStack;                // Pointer on stack thread.
        string              mDataLocation;          // Complete dynamic location where to save data.
        ExposureControl     *pExpCtrl;              // Pointer on exposure time adjustment object.
        string              mCfgPath;               // Configuration file path.



        vector<AcqSchedule> mSchedule;
        bool                mScheduleEnabled;
        string              mDataPath;
        string              mStationName;
        Mat                 mMask;
        bool                mMaskEnabled;
        string              mMaskPath;
        bool                mExpCtrlSaveImg;
        bool                mExpCtrlSaveInfos;
        int                 mExpCtrlFrequency;
        vector<int>         mSunriseTime;
        vector<int>         mSunsetTime;
        int                 mSunsetDuration;
        int                 mSunriseDuration;
        bool                mRegularAcqEnabled;
        int                 mRegularInterval;
        int                 mRegularExposure;
        int                 mRegularGain;
        CamPixFmt           mRegularFormat;
        int                 mRegularRepetition;
        bool                mVideoFramesInInput;
        bool                mDetectionEnabled;
        double              mMinExposureTime;
        double              mMaxExposureTime;
        int                 mMinGain;
        int                 mMaxGain;
        Fits                mFitsHeader;
        bool                mDebugEnabled;
        double              mSunHorizon1;
        double              mSunHorizon2;
        double              mStationLongitude;
        double              mStationLatitude;

        bool                mEphemerisUpdated;
        bool                mEphemerisEnabled;
        string              mCurrentDate;

        int mStartSunriseTime;
        int mStopSunriseTime;
        int mStartSunsetTime;
        int mStopSunsetTime;
        int mCurrentTime; // In seconds.

        TimeMode            mRegularMode;
        ImgFormat           mRegularOutput;
        ImgFormat           mScheduleOutput;
        TimeMode            mDetectionMode;
        TimeMode            mStackMode;

        boost::circular_buffer<double> fpsBuffer;
        double averageFPS;

        // Communication with the shared framebuffer.
        boost::condition_variable *frameBuffer_condition;
        boost::mutex *frameBuffer_mutex;
        boost::circular_buffer<Frame> *frameBuffer;

        // Communication with DetThread.
        bool *stackSignal;
        boost::mutex *stackSignal_mutex;
        boost::condition_variable *stackSignal_condition;

        // Communication with StackThread.
        bool *detSignal;
        boost::mutex *detSignal_mutex;
        boost::condition_variable *detSignal_condition;



    public :

        /**
         * Constructor.
         *
         * @param camType Camera type in input.
         * @param cfg Configuration file.
         * @param fb Pointer on framebuffer.
         * @param fb_m Pointer on framebuffer's mutex.
         * @param fb_c Pointer on framebuffer's condition.
         * @param sSignal Pointer on stack's signal.
         * @param sSignal_m Pointer on stack's signal mutex.
         * @param sSignal_c Pointer on stack's signal condition.
         * @param dSignal Pointer on detection's signal.
         * @param dSignal_m Pointer on detection's signal mutex.
         * @param dSignal_c Pointer on detection's signal condition.
         * @param detection Pointer on detection thread.
         * @param stack Pointer on stack thread.
         */
        AcqThread(  string                          cfgFile,
                    boost::circular_buffer<Frame>   *fb,
                    boost::mutex                    *fb_m,
                    boost::condition_variable       *fb_c,
                    bool                            *sSignal,
                    boost::mutex                    *sSignal_m,
                    boost::condition_variable       *sSignal_c,
                    bool                            *dSignal,
                    boost::mutex                    *dSignal_m,
                    boost::condition_variable       *dSignal_c,
                    DetThread                       *detection,
                    StackThread                     *stack);

        /**
         * Destructor.
         */
        ~AcqThread(void);

        /**
         * Acquisition thread loop.
         */
        void operator()();

        /**
         * Stop acquisition thread.
         */
        void stopThread();

        /**
         * Start acquisition thread.
         *
         * @return Success status to start thread.
         */
        bool startThread();

        /**
         * Get thread status.
         *
         * @return Acquisition thread running status.
         */
        bool getThreadEndStatus();

        bool getSunTimes();


    private :

        /**
         * Build single acquisition directory.
         *
         * @param YYYYMMDD Acquisition date.
         * @return Success to create directory.
         */
        bool buildAcquisitionDirectory(string YYYYMMDD);

        /**
         * Sort schedule acquisition date.
         *
         */
        void sortAcquisitionSchedule();

        /**
         * Select next scheduled acquisition.
         *
         */
        void selectNextAcquisitionSchedule(TimeDate::Date date);

        void saveImageCaptured(Frame &img, int imgNum, ImgFormat outputType);

        void runImageCapture(int imgNumber, int imgExposure, int imgGain, CamPixFmt imgFormat, ImgFormat imgOutput);

        bool configureInputDevice();
};

#endif
