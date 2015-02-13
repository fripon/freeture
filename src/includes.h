/*
				includes.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau
*                               FRIPON-GEOPS-UPSUD-CNRS
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

#define HAVE_CONFIG_H 1

#ifdef    HAVE_CONFIG_H
    #include "config.h"
   // #define USE_PYLON
#else

#endif

//#define _WIN64_
#define _LINUX_

#ifdef _WIN64_
	// Si vous incluez SDKDDKVer.h, cela définit la dernière plateforme Windows disponible.
	// Si vous souhaitez générer votre application pour une plateforme Windows précédente, incluez WinSDKVer.h et
	// définissez la macro _WIN32_WINNT à la plateforme que vous souhaitez prendre en charge avant d'inclure SDKDDKVer.h.
	#include <SDKDDKVer.h>
	#include <windows.h>
	#include <chrono>
#elif defined _LINUX_
	#include <unistd.h>
	#define BOOST_LOG_DYN_LINK 1
	//#define BOOST_THREAD_USE_LIB
#endif

// standard
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <ctime>
#include <time.h>
#include <ostream>

#include <sstream>
#include <queue>
#include <fstream>
#include <algorithm>
#include <map>
#include <math.h>
//#include <process.h>
#include <signal.h>

// opencv
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>

// boost log management
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/record_ordering.hpp>
#include <boost/log/core.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

// boost date and time management
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

// boost thread management
#include <boost/thread.hpp>
#include <boost/lambda/lambda.hpp>

//boost program's options management
#include "boost/program_options.hpp"

//boost tokenizer
#include <boost/tokenizer.hpp>






