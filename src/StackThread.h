/*
								StackThread.h

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
* \file    StackThread.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    19/06/2014
* \brief   Stack frames.
*/

#pragma once

#include "config.h"

#include <iostream>
#include "EStackMeth.h"
#include "Stack.h"
#include "Fits.h"
#include "Fits2D.h"
#include "TimeDate.h"

#include "EParser.h"

#include <boost/filesystem.hpp>
#include <boost/circular_buffer.hpp>

using namespace boost::filesystem;

using namespace std;
using namespace cv;
using namespace boost::posix_time;

class StackThread{

	private:

		static boost::log::sources::severity_logger< LogSeverityLevel > logger;

		static class _Init{

			public:
				_Init()
				{
					logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("StackThread"));
				}
		} _initializer;	

		boost::thread *thread;

		string		DATA_PATH;
		bool		STACK_REDUCTION;
		int			STACK_INTERVAL;
		double		STACK_TIME;
		CamBitDepth ACQ_BIT_DEPTH;
		StackMeth	STACK_MTHD;
		string		STATION_NAME;
		bool		CFG_FILECOPY_ENABLED;
		Fits		fitsHeader;

		bool mustStop;
		boost::mutex mustStopMutex;

		string cfgPath;
		string completeDataPath;

		boost::condition_variable		*frameBuffer_condition;
        boost::mutex					*frameBuffer_mutex;
        boost::circular_buffer<Frame>	*frameBuffer;

		bool							*stackSignal;
        boost::mutex					*stackSignal_mutex;
        boost::condition_variable		*stackSignal_condition;

		boost::mutex					*cfg_mutex;

	public:

        StackThread(	boost::mutex							*cfg_m,
						string									cfg_p,
						bool									*sS,
						boost::mutex							*sS_m,
						boost::condition_variable				*sS_c,
						boost::circular_buffer<Frame>		    *fb,
						boost::mutex                            *fb_m,
						boost::condition_variable               *fb_c);

        ~StackThread(void);

		bool startThread();

		void stopThread();

		void operator()();

	private :

		bool loadStackParameters();

		bool buildStackDataDirectory(string date);

		/* /* if(ft.CFG_FILECOPY_ENABLED){

                                    namespace fs = boost::filesystem;

                                    string dateNow = TimeDate::localDateTime(second_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");
                                    vector<string> dateString;

                                    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                                    boost::char_separator<char> sep(":");
                                    tokenizer tokens(dateNow, sep);

                                    for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
                                        dateString.push_back(*tok_iter);
                                    }

                                    string root = ft.DATA_PATH + ft.STATION_NAME + "_" + dateString.at(0) + dateString.at(1) + dateString.at(2) +"/";

                                    string cFile = root + "configuration.cfg";

                                    cout << cFile << endl;

                                    path p(ft.DATA_PATH);

                                    path p1(root);

                                    path p2(cFile);

                                    // /home/fripon/data/
                                    if(fs::exists(p)){

                                        // /home/fripon/data/STATION_AAAAMMDD/
                                        if(fs::exists(p1)){

                                            path p3(configPath);

                                            if(fs::exists(p3)){

                                                fs::copy_file(p3,p2,copy_option::overwrite_if_exists);

                                            }

                                        }else{

                                            if(!fs::create_directory(p1)){

                                                cout << "Unable to create destination directory" << p1.string();

                                            }else{

                                                path p3(configPath);

                                                if(fs::exists(p3)){

                                                    fs::copy_file(p3,p2,copy_option::overwrite_if_exists);

                                                }
                                            }
                                        }

                                    }
                                }*/
								
		

};
