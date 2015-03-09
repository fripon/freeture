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
#include "SMTPClient.h"
#include <iterator>



#ifdef LINUX
    #define BOOST_LOG_DYN_LINK 1
#endif

#include "Fits.h"
#include "Fits2D.h"
#include "TimeDate.h"
#include "Fits3D.h"
#include "Stack.h"
#include "Detection.h"
#include "DetectionTemporal.h"

#include "EStackMeth.h"
#include "EDetMeth.h"

#include <boost/circular_buffer.hpp>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;
using namespace cv;
using namespace std;
using namespace boost::posix_time;

class DetThread{

	private:

        static boost::log::sources::severity_logger< LogSeverityLevel > logger;

		static class _Init{

			public:
				_Init()
				{
					logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("DetThread"));
				}
		} _initializer;

        boost::thread *m_thread;
		Detection	*detTech;

        bool mustStop;
        boost::mutex mustStopMutex;

        boost::circular_buffer<Frame>   *frameBuffer;
        boost::mutex                    *frameBuffer_mutex;
        boost::condition_variable       *frameBuffer_condition;

        bool                            *detSignal;
        boost::mutex                    *detSignal_mutex;
        boost::condition_variable       *detSignal_condition;

        bool			DET_SAVE_AVI;
        bool			DET_SAVE_FITS3D ;
        bool			DET_SAVE_FITS2D;
        bool			DET_SAVE_SUM;
        double			DET_TIME_BEFORE;
		double			DET_TIME_AFTER;
		string			DATA_PATH;
        string			STATION_NAME;
		CamBitDepth		ACQ_BIT_DEPTH;
		Fits			fitsHeader;
		bool			MAIL_DETECTION_ENABLED;
        string			MAIL_SMTP_SERVER;
        string			MAIL_SMTP_HOSTNAME;
        vector<string>	MAIL_RECIPIENT;
		bool			CFG_FILECOPY_ENABLED;
		bool			STACK_REDUCTION;
		StackMeth		STACK_MTHD;
		DetMeth			detmthd;

		bool waitFramesToCompleteEvent;
		int nbWaitFrames;

		string cfg_path;
		boost::mutex *cfg_mutex;

		bool firstFrameGrabbed;

		string eventPath;
		string eventDate;

	public:

        DetThread(  boost::mutex					*cfg_m,
					string							cfg_p,
					DetMeth							m,
					boost::circular_buffer<Frame>	*fb,
					boost::mutex					*fb_m,
					boost::condition_variable		*fb_c,
					bool							*dSignal,
					boost::mutex					*dSignal_m,
					boost::condition_variable		*dSignal_c);

		~DetThread();

		void operator()();

        bool startThread();

		void stopThread();

		void join();

		bool buildEventDataDirectory(string eventDate);

		bool saveEventData(int firstEvPosInFB, int lastEvPosInFB);

		bool loadDetThreadParameters();

};


