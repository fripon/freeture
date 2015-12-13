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
#include "EImgFormat.h"
#include "DetThread.h"
#include "StackThread.h"
#include "Device.h"
#include "ExposureControl.h"
#include "ImgProcessing.h"
#include "Ephemeris.h"
#include "Fits2D.h"
#include "SParam.h"

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
        bool                mThreadTerminated;      // Terminated status of the thread.
        Device              *mDevice;               // Device used for acquisition.
        int                 mDeviceID;              // Index of the device to use.
        scheduleParam       mNextAcq;               // Next scheduled acquisition.
        int                 mNextAcqIndex;
        DetThread           *pDetection;            // Pointer on detection thread in order to stop it or reset it when a regular capture occurs.
        StackThread         *pStack;                // Pointer on stack thread in order to save and reset a stack when a regular capture occurs.
        ExposureControl     *pExpCtrl;              // Pointer on exposure time control object while sunrise and sunset.
        string              mOutputDataPath;        // Dynamic location where to save data (regular captures etc...).
        string              mCurrentDate;
        int                 mStartSunriseTime;      // In seconds.
        int                 mStopSunriseTime;       // In seconds.
        int                 mStartSunsetTime;       // In seconds.
        int                 mStopSunsetTime;        // In seconds.
        int                 mCurrentTime;           // In seconds.

        // Parameters from configuration file.
        stackParam          msp;
        stationParam        mstp;
        detectionParam      mdtp;
        cameraParam         mcp;
        dataParam           mdp;
        fitskeysParam       mfkp;
        framesParam         mfp;
        videoParam          mvp;

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

        AcqThread(  boost::circular_buffer<Frame>   *fb,
                    boost::mutex                    *fb_m,
                    boost::condition_variable       *fb_c,
                    bool                            *sSignal,
                    boost::mutex                    *sSignal_m,
                    boost::condition_variable       *sSignal_c,
                    bool                            *dSignal,
                    boost::mutex                    *dSignal_m,
                    boost::condition_variable       *dSignal_c,
                    DetThread                       *detection,
                    StackThread                     *stack,
                    int                                 cid,
                    dataParam                           dp,
                    stackParam                          sp,
                    stationParam                        stp,
                    detectionParam                      dtp,
                    cameraParam                         acq,
                    framesParam                         fp,
                    videoParam                          vp,
                    fitskeysParam                       fkp);

        ~AcqThread(void);

        void operator()();

        void stopThread();

        bool startThread();

        // Return activity status.
        bool getThreadStatus();

    private :

        // Compute in seconds the sunrise start/stop times and the sunset start/stop times.
        bool computeSunTimes();

        // Build the directory where the data will be saved.
        bool buildAcquisitionDirectory(string YYYYMMDD);

        // Analyse the scheduled acquisition list to find the next one according to the current time.
        void selectNextAcquisitionSchedule(TimeDate::Date date);

        // Save a capture on disk.
        void saveImageCaptured(Frame &img, int imgNum, ImgFormat outputType);

        // Run a regular or scheduled acquisition.
        void runImageCapture(int imgNumber, int imgExposure, int imgGain, CamPixFmt imgFormat, ImgFormat imgOutput);

        // Prepare the device for a continuous acquisition.
        bool prepareAcquisitionOnDevice();
};

#endif
