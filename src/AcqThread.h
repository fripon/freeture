/*
								AcqThread.h

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

#include "ECamBitDepth.h"
#include "AcqSchedule.h"
#include "DetThread.h"
#include "StackThread.h"
#include "Device.h"
#include "ExposureControl.h"

using namespace cv;
using namespace std;

class AcqThread{

	private:

		static boost::log::sources::severity_logger< LogSeverityLevel > logger;

		static class _Init{

            public:
                _Init()
                {
                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("AcqThread"));
                }
        } _initializer;

        bool mustStop;

		boost::mutex mustStopMutex;

		boost::thread *acquisitionThread;

		Device *cam;

		int srcExposure;

		int srcID;

        int srcGain;

        int srcFPS;

		string srcPath;

        CamBitDepth srcFormat;

        CamType srcType;

		int frameCpt;

		int nbFailGrabbedFrames;

		int nbSuccessGrabbedFrames;

		// Communication with the shared framebuffer.
        boost::condition_variable		*frameBuffer_condition;
        boost::mutex					*frameBuffer_mutex;
        boost::circular_buffer<Frame>	*frameBuffer;

		// Communication with DetThread.
        bool						*stackSignal;
        boost::mutex				*stackSignal_mutex;
        boost::condition_variable	*stackSignal_condition;

		// Communication with StackThread.
        bool						*detSignal;
        boost::mutex				*detSignal_mutex;
        boost::condition_variable	*detSignal_condition;

		// Mutex on configuration file.
		boost::mutex *cfg_mutex;
		string cfg_path;

		bool threadTerminated;

		DetThread	*detectionProcess;
        StackThread	*stackProcess;

        ExposureControl * autoExposure;

        bool enableStackThread;

        vector<string> schedule;

        string completeDataPath;

        vector<AcqSchedule> ACQ_SCHEDULE;

        // Next acquisition to achieve.
        AcqSchedule nextTask;

        // Index of the next acquisition to achieve in the schedule table.
        int indexNextTask = 0;

	public:

        AcqThread(	CamType									camType,
					boost::mutex							*cfg_m,
					string									cfg_p,
					boost::circular_buffer<Frame>           *fb,
					boost::mutex                            *fb_m,
					boost::condition_variable               *fb_c,
					bool									*sSignal,
					boost::mutex                            *sSignal_m,
					boost::condition_variable               *sSignal_c,
					bool                                    *dSignal,
					boost::mutex                            *dSignal_m,
					boost::condition_variable               *dSignal_c,
					DetThread	                            *detection,
                    StackThread	                            *stack);

		~AcqThread(void);

		//! Wait the end of the acquisition thread.
		void	join();

        //! Acquisition thread main loop.
		void	operator()();

		//! Stop the acquisition thread.
		void	stopThread();

		//! Start the acquisition thread.
		bool	startThread();

		bool	getThreadTerminatedStatus();

    private :

        bool runScheduledAcquisition(AcqSchedule task);

        bool runRegularAcquisition(string frameDate);

        bool buildRegularAcquisitionDirectory(string YYYYMMDD);

        void sortAcquisitionSchedule();

        void selectNextAcquisitionSchedule();


};

#endif
