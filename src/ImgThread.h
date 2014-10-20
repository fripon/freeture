/*
								ImgThread.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
 * @file    ImgThread.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    13/06/2014
 */

#pragma once

#include "includes.h"
#include "Fifo.h"
#include "Frame.h"
#include "Histogram.h"
#include "TimeDate.h"
#include "EnumLog.h"
using namespace std;
using namespace cv;
using namespace boost::posix_time;

namespace logging	= boost::log;
namespace sinks		= boost::log::sinks;
namespace attrs		= boost::log::attributes;
namespace src		= boost::log::sources;
namespace expr		= boost::log::expressions;
namespace keywords	= boost::log::keywords;

using namespace logenum;

class ImgThread{

	private:

		src::severity_logger< severity_level > log;

		boost::thread				*thread;
		Fifo<Frame>					*framesQueue;
		string						imgPath;
		int							interval;
		double						exposureTime;
		bool						gammaCorrectionEnable;
		double						gammaCorrectionValue;

		boost::mutex				*mutexQueue;
		bool						mustStop;				// Used to stop detection thread
		boost::mutex				mustStopMutex;			// Mutex for access mustStop flag
		boost::condition_variable	*condQueueFill;
		boost::condition_variable	*condNewFrame;

		int							formatPixel;

	public:
		ImgThread					(string path,
									int intervalCap,
									double expTime,
									bool imgCapGammaCorrEnable,
									double imgCapGammaCorrValue,
									Fifo<Frame>* sharedQueue,
									int acqFormat,
									boost::mutex *sharedQueueMutex,
									boost::condition_variable	*sharedQueueFill,
									boost::condition_variable	*sharedQueueNewElem);

		~ImgThread					(void);
	//	string		locale_formatted_datetime (::boost::posix_time::ptime pt);
		void		startCapture	();
		void		stopCapture		();
		void		join			();
		string		intToString		(int nb);
		void		saveImageToJPEG	(Mat img, string name);
		void		operator		()();

};

