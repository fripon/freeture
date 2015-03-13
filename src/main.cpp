/*
								main.cpp

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
*	Last modified:		02/03/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    main.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    02/03/2015
*/

#include "config.h"
//#include "SMTPClient.h"
#ifdef WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <process.h>
#else
	#ifdef LINUX
		#include <signal.h>
		#include <unistd.h>
		#define BOOST_LOG_DYN_LINK 1
	#endif
#endif

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

#include <boost/circular_buffer.hpp>
#include <boost/program_options.hpp>

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>

#include <string>
#include <iostream>
#include <stdio.h>
#include <memory>

#include "Frame.h"

#include "SaveImg.h"
#include "Conversion.h"
#include "Fits2D.h"
#include "ECamType.h"
#include "EImgBitDepth.h"
#include "EParser.h"
#include "EDetMeth.h"
#include "DetThread.h"
#include "StackThread.h"
#include "AcqThread.h"
#include "CameraGigeSdkIc.h"

#define BOOST_NO_SCOPED_ENUMS

namespace po        = boost::program_options;
namespace logging	= boost::log;
namespace sinks		= boost::log::sinks;
namespace attrs		= boost::log::attributes;
namespace src		= boost::log::sources;
namespace expr		= boost::log::expressions;
namespace keywords	= boost::log::keywords;

using boost::shared_ptr;
using namespace std;

bool sigTermFlag = false;

#ifdef LINUX

struct sigaction act;

/**
 * \brief Terminate signal on the current processus
 */

void sigTermHandler(int signum, siginfo_t *info, void *ptr){

	src::severity_logger< LogSeverityLevel > lg;

	BOOST_LOG_SEV(lg, notification) << "Received signal : "<< signum << " from : "<< (unsigned long)info->si_pid;

	sigTermFlag = true;

}

#endif

// The operator puts a human-friendly representation of the severity level to the stream
// http://www.boost.org/doc/libs/1_55_0/libs/log/example/doc/expressions_attr_fmt_tag.cpp
std::ostream& operator<< (std::ostream& strm, LogSeverityLevel level){

    static const char* strings[] =
    {
        "NORM",
        "NTFY",
        "WARN",
        "FAIL",
        "CRIT"
    };

    if (static_cast< std::size_t >(level) < sizeof(strings) / sizeof(*strings))
        strm << strings[level];
    else
        strm << static_cast< int >(level);

    return strm;
}

void init_log(string path, LogSeverityLevel sev){

	// Create a text file sink
    typedef sinks::synchronous_sink< sinks::text_multifile_backend > file_sink;
    boost::shared_ptr< file_sink > sink(new file_sink);

    // Set up how the file names will be generated
    sink->locked_backend()->set_file_name_composer(
        sinks::file::as_file_name_composer(expr::stream << path <<expr::attr< std::string >("LogName") << ".log"));

//    sink->locked_backend()->auto_flush(true);

    // Set the log record formatter
    sink->set_formatter(
        expr::format("[%1%] <%2%> (%3%) - %4%")
            % expr::attr< boost::posix_time::ptime >("TimeStamp")
            % expr::attr< LogSeverityLevel>("Severity")
            % expr::attr<std::string>("ClassName")
            % expr::smessage
    );

    //sink->set_filter(severity >= warning
     boost::log::core::get()->set_filter(boost::log::expressions::attr< LogSeverityLevel >("Severity") >= sev);
    // Add it to the core
    logging::core::get()->add_sink(sink);


    // Add some attributes too
    logging::add_common_attributes();
    logging::core::get()->add_global_attribute("TimeStamp", attrs::local_clock());

}

int main(int argc, const char ** argv){

    // Program options.
    po::options_description desc("Available options");
    desc.add_options()
      ("mode,m",        po::value<int>(),                                                               "Execution mode of the program")
      ("time,t",        po::value<int>(),                                                               "Execution time of the program in seconds")
      ("help,h",                                                                                        "Print help messages")
      ("config,c",      po::value<string>()->default_value(string(CFG_PATH) + "configuration.cfg"),     "Configuration file's path")
      ("bitdepth,d",    po::value<int>()->default_value(8),                                             "Bit depth of a frame")
      ("bmp",           po::value<bool>()->default_value(false),                                        "Save .bmp")
      ("fits",          po::value<bool>()->default_value(false),                                        "Save fits2D")
      ("gain,g",        po::value<int>(),                                                               "Define gain")
      ("exposure,e",    po::value<int>(),                                                               "Define exposure")
      ("version,v",                                                                                     "Get program version")
      ("camtype",       po::value<string>()->default_value("BASLER_GIGE"),                              "Type of camera")
      ("display",       po::value<bool>()->default_value(false),                                        "In mode 4 : Display the grabbed frame")
      ("id",            po::value<int>(),                                                               "Camera ID")
      ("fps",           po::value<int>(),                                                               "Acquisition frequency")
      ("savepath,p",    po::value<string>()->default_value("./"),                                       "Save path");

    po::variables_map vm;

    try{

        int     mode            = 0;
        int     executionTime   = 0;
        string  configPath      = string(CFG_PATH) + "configuration.cfg";
        string  savePath        = "./";
        int     acqFormat       = 8;
        bool    saveBmp         = false;
        bool    saveFits2D      = false;
        int     gain            = 100;
        int     exp             = 100;
        string  version         = string(VERSION);
        bool    display         = false;
        int     camID           = 0;

		std::cout << "cfg path read : " << string(CFG_PATH) <<endl;

        po::store(po::parse_command_line(argc, argv, desc), vm);

        if(vm.count("mode"))
            mode = vm["mode"].as<int>();

        if(vm.count("time"))
            executionTime = vm["time"].as<int>();

        if(vm.count("config"))
            configPath = vm["config"].as<string>();

        if(vm.count("version")){

            std::cout << "Current version : " << version << endl;

        }else if(vm.count("help")){

            std::cout << desc;

        }else{

            switch(mode){

                case 1 :

                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%% MODE 1 : LIST CONNECTED CAMERAS %%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    {
                        std::cout << "================================================" << endl;
                        std::cout << "========= FREETURE - Available cameras =========" << endl;
                        std::cout << "================================================" << endl << endl;

						string  camtype;
                        if(vm.count("camtype"))	camtype = vm["camtype"].as<string>();
                        std::transform(camtype.begin(), camtype.end(),camtype.begin(), ::toupper);

						std::cout << "Searching cameras..." << endl << endl;

						EParser<CamType> cam_type;

						Device *cam = new Device(cam_type.parseEnum("CAMERA_TYPE", camtype));

						cam->listGigeCameras();

                    }

                    break;

                case 2 :

                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%% MODE 2 : VIEW/CHECK CONFIGURATION FILE %%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    {
						/*
                        FreeTure ft(configPath);
                        ft.loadParameters();
                        ft.printParameters();
						*/

						cout << "Mode 2 not available in this version" << endl;

                    }

                    break;

                case 3 :

                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%% MODE 3 : RUN METEOR DETECTION %%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    {

                        std::cout << "================================================" << endl;
                        std::cout << "======= FREETURE - Meteor detection mode =======" << endl;
                        std::cout << "================================================" << endl << endl;

						/// ------------------------------------
                        /// ------ LOAD FREETURE PARAMETERS ----
						/// ------------------------------------

						boost::filesystem::path pcfg(configPath);
						if(!boost::filesystem::exists(pcfg))
							throw "configuration.cfg not found.";

						Configuration cfg;
						cfg.Load(configPath);

						// Get Camera type in input.
						string camera_type; cfg.Get("CAMERA_TYPE", camera_type);
						EParser<CamType> cam_type;
						CamType CAMERA_TYPE = cam_type.parseEnum("CAMERA_TYPE", camera_type);

						// Get size of the frame buffer.
						int ACQ_BUFFER_SIZE; cfg.Get("ACQ_BUFFER_SIZE", ACQ_BUFFER_SIZE);

						// Get acquisition frequency.
						int ACQ_FPS; cfg.Get("ACQ_FPS", ACQ_FPS);

						// Detection enabled or not.
						bool DET_ENABLED; cfg.Get("DET_ENABLED", DET_ENABLED);

						// Stack enabled or not.
						bool STACK_ENABLED; cfg.Get("STACK_ENABLED", STACK_ENABLED);

						// Get log path.
						string LOG_PATH; cfg.Get("LOG_PATH", LOG_PATH);

						// Get log severity.
						string log_severity; cfg.Get("LOG_SEVERITY", log_severity);
						EParser<LogSeverityLevel> log_sev;
						LogSeverityLevel LOG_SEVERITY = log_sev.parseEnum("LOG_SEVERITY", log_severity);

						// Get det method.
						string det_method;
						cfg.Get("DET_METHOD", det_method);
						EParser<DetMeth> det_mth;
						DetMeth DET_METHOD = det_mth.parseEnum("DET_METHOD", det_method);

						/// ------------------------------------
                        /// ------------ MANAGE LOG ------------
						/// ------------------------------------

                        path pLog(LOG_PATH);

                        if(!boost::filesystem::exists(pLog)){

                            if(!create_directory(pLog))
                                throw "> Failed to create a directory for logs files.";
                        }

                        init_log(LOG_PATH, LOG_SEVERITY);
                        src::severity_logger< LogSeverityLevel > slg;
                        slg.add_attribute("ClassName", boost::log::attributes::constant<std::string>("main.cpp"));
                        BOOST_LOG_SCOPED_THREAD_TAG("LogName", "MAIN_THREAD");
                        BOOST_LOG_SEV(slg, notification) << " FREETURE - Meteor detection mode ";

						/// ------------------------------------
						/// --------- SHARED RESSOURCES --------
						/// ------------------------------------

                        // Circular buffer to store last n grabbed frames.
                        boost::circular_buffer<Frame> frameBuffer(ACQ_BUFFER_SIZE * ACQ_FPS);
                        boost::mutex frameBuffer_m;
                        boost::condition_variable frameBuffer_c;

                        bool signalDet = false;
                        boost::mutex signalDet_m;
                        boost::condition_variable signalDet_c;

                        bool signalStack = false;
                        boost::mutex signalStack_m;
                        boost::condition_variable signalStack_c;

						boost::mutex cfg_m;

						/// -------------------------------------
						/// ---------- CREATE THREADS -----------
                        /// -------------------------------------

                        AcqThread	*inputDevice		= NULL;
                        DetThread	*detection			= NULL;
                        StackThread	*stack				= NULL;
						bool stackThreadCreationSuccess	= true;
						bool detThreadCreationSuccess	= true;

						inputDevice = new AcqThread(	CAMERA_TYPE,
														&cfg_m,
														configPath,
														&frameBuffer,
														&frameBuffer_m,
														&frameBuffer_c,
														&signalStack,
														&signalStack_m,
														&signalStack_c,
														&signalDet,
														&signalDet_m,
														&signalDet_c);

                        if(inputDevice != NULL){

                            if(!inputDevice->startThread()){

								std::cout << "Fail to start acquisition Thread." << endl;
								BOOST_LOG_SEV(slg, fail) << "Fail to start acquisition Thread.";

							}else{

								BOOST_LOG_SEV(slg, normal) << "Success to start acquisition Thread.";

								/// Create stack thread.
								if(STACK_ENABLED){

									BOOST_LOG_SEV(slg, normal) << "Start to create stack Thread.";

									stack = new StackThread(	&cfg_m,
																configPath,
																&signalStack,
																&signalStack_m,
																&signalStack_c,
																&frameBuffer,
																&frameBuffer_m,
																&frameBuffer_c);

									if(!stack->startThread()){

										cout << "Fail to start stack Thread." << endl;
										BOOST_LOG_SEV(slg, fail) << "Fail to start stack Thread.";
										stackThreadCreationSuccess = false;

									}

								}

								/// Create detection thread.
								if(DET_ENABLED){

									BOOST_LOG_SEV(slg, normal) << "Start to create detection Thread.";

									detection  = new DetThread(	&cfg_m,
																configPath,
																DET_METHOD,
																&frameBuffer,
																&frameBuffer_m,
																&frameBuffer_c,
																&signalDet,
																&signalDet_m,
																&signalDet_c);

									if(!detection->startThread()){

										cout << "Fail to start detection Thread." << endl;
										BOOST_LOG_SEV(slg, fail) << "Fail to start detection Thread.";
										detThreadCreationSuccess = false;

									}

								}

								if(detThreadCreationSuccess && stackThreadCreationSuccess){

									#ifdef WINDOWS

										 std::cout << "This is the process : " << (unsigned long)_getpid() << endl;

									#elif defined LINUX

										BOOST_LOG_SEV(slg, notification) << "This is the process : " << (unsigned long)getpid();

										memset(&act, 0, sizeof(act));
										act.sa_sigaction = sigTermHandler;
										act.sa_flags = SA_SIGINFO;
										sigaction(SIGTERM,&act,NULL);

									#endif

									int cptTime = 0;

									while(!sigTermFlag){

										#ifdef WINDOWS
											waitKey(1000);
											//cout << "wait 1 in main sec" << endl;
										#elif defined LINUX
											sleep(1);
										#endif

										if(executionTime != 0){

											if(cptTime > executionTime){

												std::cout << "Break main loop"<< endl;

												break;
											}
											cptTime ++;

										}

										if(inputDevice != NULL)
											if(inputDevice->getThreadTerminatedStatus()){

												std::cout << "Break main loop" << endl;
												break;

											}

									}
								}

								if(detection != NULL){

									if(detThreadCreationSuccess) detection->stopThread();
									delete detection;

								}

								if(stack != NULL){

									if(stackThreadCreationSuccess) stack->stopThread();
									delete stack;

								}

								inputDevice->stopThread();

							}

							delete inputDevice;
                        }
                    }

                    break;

                case 4 :

                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%% MODE 4 : RUN ACQ TEST %%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    {

                        std::cout << "================================================" << endl;
                        std::cout << "======= FREETURE - Acquisition test mode =======" << endl;
                        std::cout << "================================================" << endl << endl;

                        // Display or not the grabbed frame.
                        if(vm.count("display")) display = vm["display"].as<bool>();

                        // Path where to save files.
                        if(vm.count("savepath")) savePath = vm["savepath"].as<string>();

                        // Acquisition format.
                        if(vm.count("bitdepth")) acqFormat = vm["bitdepth"].as<int>();
						CamBitDepth camFormat;
						Conversion::intBitDepth_To_CamBitDepth(acqFormat, camFormat);

                        // Cam id.
                        if(vm.count("id")) camID = vm["id"].as<int>();

                        // Save bmp.
                        if(vm.count("bmp")) saveBmp = vm["bmp"].as<bool>();

                        // Save fits.
                        if(vm.count("fits")) saveFits2D = vm["fits"].as<bool>();

                        // Type of camera in input.
						string camtype;
                        if(vm.count("camtype")) camtype = vm["camtype"].as<string>();
                        std::transform(camtype.begin(), camtype.end(),camtype.begin(), ::toupper);

                        // Gain value.
                        if(vm.count("gain")) gain = vm["gain"].as<int>();
                        else throw "Please define the gain value.";

                        // Exposure value.
                        if(vm.count("exposure")) exp = vm["exposure"].as<int>();
                        else throw "Please define the exposure time value.";

						Frame frame;
						frame.setExposure(exp);
						frame.setGain(gain);
						frame.setBitDepth(camFormat);

						EParser<CamType> cam_type;

						Device *cam = new Device(cam_type.parseEnum("CAMERA_TYPE", camtype));

						if(!cam->grabSingleImage(frame, camID)){
							delete cam;
							throw "> Failed to grab a single frame.";
						}
						delete cam;

						// Display the frame in an opencv window
						if(display && frame.getImg().data){

                            cout << "Display captured frame..." << endl;

							Mat temp, temp1;
							frame.getImg().copyTo(temp1);

							if(camFormat == MONO_12){

								temp = Conversion::convertTo8UC1(temp1);

							}else{

								frame.getImg().copyTo(temp);
							}

							namedWindow( "Frame", WINDOW_AUTOSIZE );
							imshow( "Frame", temp);
							waitKey(0);

						}

						// Save the frame in BMP.
						if(saveBmp && frame.getImg().data){

							Mat temp, temp1;
							frame.getImg().copyTo(temp1);

							if(camFormat == MONO_12){

								temp = Conversion::convertTo8UC1(temp1);

							}else

								frame.getImg().copyTo(temp);

							SaveImg::saveBMP(temp, savePath + "frame");

						}

						// Save the frame in Fits 2D.
						if(saveFits2D && frame.getImg().data){

							Fits fitsHeader;
							fitsHeader.setGaindb((int)gain);
							fitsHeader.setExposure(exp);

							Fits2D newFits(savePath + "frame", fitsHeader);

							switch(camFormat){

								case MONO_8 :

									{

										if(newFits.writeFits(frame.getImg(), UC8, "capture" ))
											cout << "> Fits saved in " << savePath << endl;
										else
											cout << "Failed to save Fits." << endl;

									}

									break;

								case MONO_12 :

									{

										if(newFits.writeFits(frame.getImg(), S16, "capture" ))
											cout << "> Fits saved in " << savePath << endl;
										else
											cout << "Failed to save Fits." << endl;


									}

									break;

							}
						}

                    }

                    break;

				case 5 :

					{

						CameraGigeSdkIc *c = new CameraGigeSdkIc();
						Frame n;
						c->grabSingleImage(n, 0 );
						//c->listGigeCameras();
						
						delete c;
						getchar();


					}

					break;

                default :

                    {

                        cout << "Please choose a mode (example : -m 1 )"            << endl
                                                                                    << endl;
                        cout << "Available modes are :"                             << endl;
                        cout << "1 : List connected devices"                        << endl;
                        cout << "2 : Check and print configuration file"            << endl;
                        cout << "3 : Run meteor detection"                          << endl;
                        cout << "4 : Test a camera by single capture"               << endl;

                    }

                    break;

            }

        }


    }catch(exception& e){

        cout << "An exception occured : " << e.what() << endl;

    }catch(const char * msg){

        cout << msg << endl;

    }

    po::notify(vm);

	cout << "Program terminated (press a key)." << endl;
	getchar();
	return 0 ;

}
