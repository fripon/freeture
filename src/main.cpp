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
#include "ESmtpSecurity.h"
#include "SMTPClient.h"
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
#include "Histogram.h"
#include "HistogramGray.h"
#include "HistogramRGB.h"
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
#include "ImgProcessing.h"
#include <boost/filesystem.hpp>
#include "Logger.h"

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
    // Can't use rotation with text_multifile_backend
    // http://stackoverflow.com/questions/18228123/text-multifile-backend-how-to-set-rool-file-size
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
      ("filename,n",    po::value<string>()->default_value("snap"),                                     "Name to use when a single frame is captured")
      ("sendmail,s",    po::value<string>()->default_value(""),                                         "Send a mail to the specified adress when a single frame is captured")
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
        string  fileName        = "snap";
        string  sendMail        = "";

        std::cout << "Default configuration file : " << string(CFG_PATH) << "configuration.cfg" <<endl;

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

                        path pLog("./flog/");

                        if(!boost::filesystem::exists(pLog)){

                            if(!create_directory(pLog))
                                throw "> Failed to create a directory for logs files.";
                        }

                        string log_severity = "normal";
                        EParser<LogSeverityLevel> log_sev;
                        LogSeverityLevel LOG_SEVERITY = log_sev.parseEnum("LOG_SEVERITY", log_severity);

                        init_log("./flog/", LOG_SEVERITY);
                        src::severity_logger< LogSeverityLevel > slg;
                        slg.add_attribute("ClassName", boost::log::attributes::constant<std::string>("main.cpp"));
                        BOOST_LOG_SCOPED_THREAD_TAG("LogName", "MAIN_THREAD");
                        BOOST_LOG_SEV(slg,notification) << "\n";
                        BOOST_LOG_SEV(slg,notification) << "==============================================";
                        BOOST_LOG_SEV(slg,notification) << "======== FREETURE - Available cameras ========";
                        BOOST_LOG_SEV(slg,notification) << "==============================================";


                        std::cout << "================================================" << endl;
                        std::cout << "========= FREETURE - Available cameras =========" << endl;
                        std::cout << "================================================" << endl << endl;

                        string  camtype;
                        if(vm.count("camtype"))	camtype = vm["camtype"].as<string>();
                        std::transform(camtype.begin(), camtype.end(),camtype.begin(), ::toupper);

                        std::cout << "Searching cameras..." << endl << endl;

                        EParser<CamType> cam_type;

                        Device *device = new Device(cam_type.parseEnum("CAMERA_TYPE", camtype));

                        device->getCam()->listGigeCameras();

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

                        /*The static member function boost::thread::hardware_concurrency() returns the number
                        of threads that can physically be executed at the same time, based on the underlying
                        number of CPUs or CPU cores. Calling this function on a dual-core processor returns a
                        value of 2. This function provides a simple method to identify the theoretical maximum
                        number of threads that should be used.*/
                        std::cout << "CORE DETECTED : " << boost::thread::hardware_concurrency()<< endl << endl;

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
                        // Disable stack if CAMERA_TYPE = FRAMES or VIDEO
                        if((CAMERA_TYPE == FRAMES) || (CAMERA_TYPE == VIDEO))
                            STACK_ENABLED = false;

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
                        BOOST_LOG_SEV(slg,notification) << "\n";
                        BOOST_LOG_SEV(slg,notification) << "==============================================";
                        BOOST_LOG_SEV(slg,notification) << "====== FREETURE - Meteor detection mode ======";
                        BOOST_LOG_SEV(slg,notification) << "==============================================";

                        try{

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

                            /// Create detection thread.
                            if(DET_ENABLED){

                                BOOST_LOG_SEV(slg, normal) << "Start to create detection Thread.";

                                detection  = new DetThread( configPath,
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

                            /// Create stack thread.
                            if(STACK_ENABLED && ((DET_ENABLED && detThreadCreationSuccess) || !DET_ENABLED)){

                                BOOST_LOG_SEV(slg, normal) << "Start to create stack Thread.";

                                stack = new StackThread(    configPath,
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

                            if(((DET_ENABLED && detThreadCreationSuccess) || !DET_ENABLED) &&           // Detection Thread enabled and succeed or not enabled
                              (((STACK_ENABLED && stackThreadCreationSuccess) || !STACK_ENABLED))){     // Stack Thread enabled and succeed or not enabled

                                inputDevice = new AcqThread(	CAMERA_TYPE,
                                                                configPath,
                                                                &frameBuffer,
                                                                &frameBuffer_m,
                                                                &frameBuffer_c,
                                                                &signalStack,
                                                                &signalStack_m,
                                                                &signalStack_c,
                                                                &signalDet,
                                                                &signalDet_m,
                                                                &signalDet_c,
                                                                detection,
                                                                stack);

                                if(inputDevice != NULL){

                                    if(!inputDevice->startThread()){

                                        std::cout << "Fail to start acquisition Thread." << endl;
                                        BOOST_LOG_SEV(slg, fail) << "Fail to start acquisition Thread.";

                                    }else{

                                        BOOST_LOG_SEV(slg, normal) << "Success to start acquisition Thread.";

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
                                                Sleep(1000);
                                                //cout << "wait 1 in main sec" << endl;
                                            #elif defined LINUX
                                                sleep(1);
                                            #endif

                                            string acquisitionDate = TimeDate::localDateTime(microsec_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");

                                            if(executionTime != 0){

                                                if(cptTime > executionTime){

                                                    std::cout << "Break main loop"<< endl;

                                                    break;
                                                }
                                                cptTime ++;

                                            }  

                                            if(inputDevice != NULL){

                                                if(inputDevice->getThreadEndStatus()){

                                                    std::cout << "Break main loop" << endl;
                                                    break;

                                                }
											
                                                if(detection != NULL){
                                                    if(!detection->getRunStatus()){
                                                        BOOST_LOG_SEV(slg, critical) << "DetThread not running. Stopping the process ...";
                                                        break;
                                                    }
                                                }

                                                if(stack != NULL){
                                                    if(!stack->getRunStatus()){
                                                        BOOST_LOG_SEV(slg, critical) << "StackThread not running. Stopping the process ...";
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                    }


                                    inputDevice->stopThread();

                                    delete inputDevice;

                                }


                            }

                            if(detection != NULL){

                                if(detThreadCreationSuccess)
                                    detection->stopThread();
                                delete detection;

                            }

                            if(stack != NULL){

                                if(stackThreadCreationSuccess)
                                    stack->stopThread();
                                delete stack;

                            }

                        }catch(exception& e){

                            cout << "An exception occured : " << e.what() << endl;
                            BOOST_LOG_SEV(slg, critical) << e.what();

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

                        /// -------------------- Initialize logger system --------------------

                        path pLog("./flog/");

                        if(!boost::filesystem::exists(pLog)){

                            if(!create_directory(pLog))
                                throw "> Failed to create a directory for logs files.";
                        }

                        string log_severity = "normal";
                        EParser<LogSeverityLevel> log_sev;
                        LogSeverityLevel LOG_SEVERITY = log_sev.parseEnum("LOG_SEVERITY", log_severity);

                        init_log("./flog/", LOG_SEVERITY);
                        src::severity_logger< LogSeverityLevel > slg;
                        slg.add_attribute("ClassName", boost::log::attributes::constant<std::string>("main.cpp"));
                        BOOST_LOG_SCOPED_THREAD_TAG("LogName", "MAIN_THREAD");
                        BOOST_LOG_SEV(slg,notification) << "\n";
                        BOOST_LOG_SEV(slg,notification) << "==============================================";
                        BOOST_LOG_SEV(slg,notification) << "====== FREETURE - Acquisition test mode ======";
                        BOOST_LOG_SEV(slg,notification) << "==============================================";

                        /// --------------------- Manage program options -----------------------

                        // Display or not the grabbed frame.
                        if(vm.count("display")) display = vm["display"].as<bool>();
                        BOOST_LOG_SEV(slg,notification) << "display option : " << display;

                        // Path where to save files.
                        if(vm.count("savepath")) savePath = vm["savepath"].as<string>();
                        BOOST_LOG_SEV(slg,notification) << "savepath option : " << savePath;

                        // Acquisition format.
                        if(vm.count("bitdepth")) acqFormat = vm["bitdepth"].as<int>();
                        CamBitDepth camFormat;
                        Conversion::intBitDepthToCamBitDepthEnum(acqFormat, camFormat);
                        BOOST_LOG_SEV(slg,notification) << "bitdepth option : " << acqFormat;

                        // Cam id.
                        if(vm.count("id")) camID = vm["id"].as<int>();
                        BOOST_LOG_SEV(slg,notification) << "id option : " << camID;

                        // Save bmp.
                        if(vm.count("bmp")) saveBmp = vm["bmp"].as<bool>();
                        BOOST_LOG_SEV(slg,notification) << "bmp option : " << saveBmp;

                        // Save fits.
                        if(vm.count("fits")) saveFits2D = vm["fits"].as<bool>();
                        BOOST_LOG_SEV(slg,notification) << "fits option : " << saveFits2D;

                        // Type of camera in input.
                        string camtype;
                        if(vm.count("camtype")) camtype = vm["camtype"].as<string>();
                        std::transform(camtype.begin(), camtype.end(),camtype.begin(), ::toupper);
                        BOOST_LOG_SEV(slg,notification) << "camtype option : " << camtype;

                        // Gain value.
                        if(vm.count("gain")){
                            gain = vm["gain"].as<int>();
                            BOOST_LOG_SEV(slg,notification) << "gain option : " << gain;
                        }else{
                            BOOST_LOG_SEV(slg,notification) << "Please define the gain value.";
                            throw "Please define the gain value.";
                        }

                        // Exposure value.
                        if(vm.count("exposure")){
                            exp = vm["exposure"].as<int>();
                            BOOST_LOG_SEV(slg,notification) << "exposure option : " << exp;
                        }else{
                            BOOST_LOG_SEV(slg,notification) << "Please define the exposure time value.";
                            throw "Please define the exposure time value.";
                        }

                        // Filename.
                        if(vm.count("filename")) fileName = vm["filename"].as<string>();
                        BOOST_LOG_SEV(slg,notification) << "filename option : " << fileName;

                        // Send mail.
                        if(vm.count("sendmail")) sendMail = vm["sendmail"].as<string>();
                        BOOST_LOG_SEV(slg,notification) << "sendmail option : " << sendMail;

                        /// ----------------------- Manage filename -----------------------------

                        int filenum = 0;
                        bool increment = false;

                        path p(savePath);

                        /// Search a frame in the directory.
                        for(directory_iterator file(p);file!= directory_iterator(); ++file){

                            path curr(file->path());

                            if(is_regular_file(curr)){

                                list<string> ch;
                                string fname = curr.filename().string();
                                Conversion::stringTok(ch, fname.c_str(), "-");

                                int i = 0;
                                int n = 0;

                                if(ch.front() == fileName && ch.size() == 2){

                                    cout << "File found :" << file->path().string() << endl;

                                    list<string> ch_;
                                    Conversion::stringTok(ch_, ch.back().c_str(), ".");

                                    int nn = atoi(ch_.front().c_str());

                                    if(nn >= filenum){

                                        filenum = nn;
                                        increment = true;

                                    }
                                }
                            }
                        }

                        if(increment) filenum++;

                        /// -------------------- Capture a single frame -------------------------

                        Frame frame;
                        frame.setExposure(exp);
                        frame.setGain(gain);
                        frame.setBitDepth(camFormat);

                        EParser<CamType> cam_type;

                        Device *device = new Device(cam_type.parseEnum("CAMERA_TYPE", camtype));

                        if(!device->getCam()->grabSingleImage(frame, camID)){
                            delete device;
                            throw "> Failed to grab a single frame.";
                        }
                        delete device;

                        cout << endl << ">> Single capture succeed !" << endl;

                        /// ---------------------- Save grabbed frame --------------------------

                        // Save the frame in BMP.
                        if(saveBmp && frame.getImg().data){

                            cout << ">> Saving bmp file ..." << endl;

                            Mat temp1, newMat;
                            frame.getImg().copyTo(temp1);

                            if(camFormat == MONO_12){
                                
                                newMat = ImgProcessing::correctGammaOnMono12(temp1, 2.2);
                                Mat temp = Conversion::convertTo8UC1(newMat);

                            }else {

                                newMat = ImgProcessing::correctGammaOnMono8(temp1, 2.2);
                            }

                            SaveImg::saveBMP(newMat, savePath + fileName + "-" + Conversion::intToString(filenum));

                        }

                        // Save the frame in Fits 2D.
                        if(saveFits2D && frame.getImg().data){

                            cout << ">> Saving fits file ..." << endl;

                            Fits fitsHeader;

                            // Load keywords from cfg file
                            bool useCfg = false;

                            path c(configPath);

                            if(is_regular_file(c)){

                                useCfg = true;
                                fitsHeader.loadKeywordsFromConfigFile(configPath);

                            }else
                                cout << ">> Can't load fits keywords from configuration file (not found or not exist). Try to use -c option or check your path." << endl;

                            Fits2D newFits(savePath, fitsHeader);
                            newFits.setGaindb((int)gain);
                            double exptime = exp/1000000.0;
                            newFits.setOntime(exptime);
                            newFits.setDateobs(frame.getAcqDateMicro());

                            if(useCfg){

                                vector<int> firstDateInt = TimeDate::getIntVectorFromDateString(frame.getAcqDateMicro());
                                double  debObsInSeconds = firstDateInt.at(3)*3600 + firstDateInt.at(4)*60 + firstDateInt.at(5);
                                double  julianDate      = TimeDate::gregorianToJulian_2(firstDateInt);
                                double  julianCentury   = TimeDate::julianCentury(julianDate);
                                double  sideralT        = TimeDate::localSideralTime_2(julianCentury, firstDateInt.at(3), firstDateInt.at(4), firstDateInt.at(5), fitsHeader.getSitelong());
                                newFits.setCrval1(sideralT);

                            }

                            newFits.setCtype1("RA---ARC");
                            newFits.setCtype2("DEC--ARC");
                            newFits.setEquinox(2000.0);

                            switch(camFormat){

                                case MONO_8 :

                                    {
                                        // Create FITS image with BITPIX = BYTE_IMG (8-bits unsigned integers), pixel with TBYTE (8-bit unsigned byte)
                                        if(newFits.writeFits(frame.getImg(), UC8, fileName + "-" + Conversion::intToString(filenum)))
                                            cout << ">> Fits saved in : " << savePath << fileName << "-" << Conversion::intToString(filenum) << endl;

                                    }

                                    break;

                                case MONO_12 :

                                    {

                                        // Convert unsigned short type image in short type image.
                                        Mat newMat = Mat(frame.getImg().rows, frame.getImg().cols, CV_16SC1, Scalar(0));

                                        // Set bzero and bscale for print unsigned short value in soft visualization.
                                        double bscale = 1;
                                        double bzero  = 32768;
                                        newFits.setBzero(bzero);
                                        newFits.setBscale(bscale);

                                        unsigned short * ptr;
                                        short * ptr2;

                                        for(int i = 0; i < frame.getImg().rows; i++){

                                            ptr = frame.getImg().ptr<unsigned short>(i);
                                            ptr2 = newMat.ptr<short>(i);

                                            for(int j = 0; j < frame.getImg().cols; j++){

                                                if(ptr[j] - 32768 > 32767){

                                                    ptr2[j] = 32767;

                                                }else{

                                                    ptr2[j] = ptr[j] - 32768;
                                                }
                                            }
                                        }


                                        // Create FITS image with BITPIX = SHORT_IMG (16-bits signed integers), pixel with TSHORT (signed short)
                                        if(newFits.writeFits(newMat, S16, fileName + "-" + Conversion::intToString(filenum)))
                                            cout << ">> Fits saved in : " << savePath << fileName << "-" << Conversion::intToString(filenum)<< endl;

                                    }
                            }

                            /// ---------------------- Send grabbed frame --------------------------

                            if(sendMail != ""){

                                cout << ">> Sending fits by mail to " << sendMail << "..." << endl;
                                cout << "(This option is only available inside fripon network.)" << endl;

                                vector<string> mailAttachments;
                                mailAttachments.push_back(savePath + fileName + "-" + Conversion::intToString(filenum) + ".fit");

                              //  SMTPClient mailc("10.8.0.1", 25, "u-psud.fr");

                                vector<string> to;
                                to.push_back(sendMail);
                               /* mailc.send("yoan.audureau@u-psud.fr",
                                            to,
                                            fileName + "-" + Conversion::intToString(filenum) + ".fit",
                                            " Exposure time : " + Conversion::intToString((int)exp) + "\n Gain : " + Conversion::intToString((int)gain) + "\n Format : " + Conversion::intToString(acqFormat),
                                            mailAttachments,
                                            false);*/


                                SMTPClient::sendMail("10.8.0.1", 
                                                    "",
                                                    "", 
                                                    "yoan.audureau@u-psud.fr", 
                                                    to, 
                                                    fileName + "-" + Conversion::intToString(filenum) + ".fit", 
                                                    " Exposure time : " + Conversion::intToString((int)exp) + "\n Gain : " + Conversion::intToString((int)gain) + "\n Format : " + Conversion::intToString(acqFormat),
                                                    mailAttachments,
                              
                                                    NO_SECURITY);


                            }
                        }


                        /// -------------------- Display grabbed frame --------------------------

                        // Display the frame in an opencv window
                        if(display && frame.getImg().data){

                            cout << ">> Display captured frame..." << endl;

                            Mat temp, temp1;
                            frame.getImg().copyTo(temp1);

                            if(camFormat == MONO_12){

                                temp = Conversion::convertTo8UC1(temp1);

                            }else{

                                frame.getImg().copyTo(temp);
                            }

                            namedWindow( "Frame", WINDOW_AUTOSIZE );
                            imshow("Frame", temp);
                            waitKey(0);

                        }
                    }


                    break;

                case 5 :

                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%% MODE 5 : CLEAN LOGS %%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    // Simply remove all log directory contents.

                    {

                        boost::filesystem::path pcfg(configPath);
                        if(!boost::filesystem::exists(pcfg))
                            throw "configuration.cfg not found.";

                        Configuration cfg;
                        cfg.Load(configPath);

                        // Get log path.
                        string LOG_PATH; cfg.Get("LOG_PATH", LOG_PATH);

	                    boost::filesystem::path p(LOG_PATH);

                        if(boost::filesystem::exists(p)){

                            boost::filesystem::remove_all(p);

                            cout << "Clean log completed." << endl;

                        }else {

                            cout << "Log directory not found." << endl;

                        }

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

	getchar();

    return 0 ;

}
