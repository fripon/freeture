/*
                                main.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*   This file is part of:   freeture
*
*   Copyright:      (C) 2014-2015 Yoan Audureau
*                               FRIPON-GEOPS-UPSUD-CNRS
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
*   Last modified:      02/12/2015
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
        #include <termios.h>
        #include <sys/types.h>
        #include <sys/time.h>
        #define BOOST_LOG_DYN_LINK 1
    #endif
#endif

#include "EInputDeviceType.h"
#include "CameraV4l2.h"
#include "Ephemeris.h"
#include "Circle.h"
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
#include "EImgBitDepth.h"
#include "EParser.h"
#include "EDetMeth.h"
#include "DetThread.h"
#include "StackThread.h"
#include "AcqThread.h"
#include "CameraGigeTis.h"
#include "ImgProcessing.h"
#include <boost/filesystem.hpp>
#include "Logger.h"
#include "CameraWindows.h"
#include "ECamPixFmt.h"
#include "CfgParam.h"

#define BOOST_NO_SCOPED_ENUMS

namespace po        = boost::program_options;
namespace logging   = boost::log;
namespace sinks     = boost::log::sinks;
namespace attrs     = boost::log::attributes;
namespace src       = boost::log::sources;
namespace expr      = boost::log::expressions;
namespace keywords  = boost::log::keywords;

using boost::shared_ptr;
using namespace std;

bool sigTermFlag = false;

#ifdef LINUX

struct sigaction act;

void sigTermHandler(int signum, siginfo_t *info, void *ptr){
    src::severity_logger< LogSeverityLevel > lg;
    BOOST_LOG_SEV(lg, notification) << "Received signal : "<< signum << " from : "<< (unsigned long)info->si_pid;
    cout << "Received signal : "<< signum << " from : "<< (unsigned long)info->si_pid<< endl;
    sigTermFlag = true;
}

// http://cc.byexamples.com/2007/04/08/non-blocking-user-input-in-loop-without-ncurses/
int kbhit() {
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

void nonblock(int state) {

    struct termios ttystate;

    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);

    if (state==1) {
        //turn off canonical mode
        ttystate.c_lflag &= ~ICANON;
        //minimum of number input read.
        ttystate.c_cc[VMIN] = 1;
    }else if (state==0) {
        //turn on canonical mode
        ttystate.c_lflag |= ICANON;
    }
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

}

#endif

// The operator puts a human-friendly representation of the severity level to the stream
// http://www.boost.org/doc/libs/1_55_0/libs/log/example/doc/expressions_attr_fmt_tag.cpp
std::ostream& operator<< (std::ostream& strm, LogSeverityLevel level) {

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

void init_log(string path, LogSeverityLevel sev) {

    // Create a text file sink
    // Can't use rotation with text_multifile_backend
    // http://stackoverflow.com/questions/18228123/text-multifile-backend-how-to-set-rool-file-size
    typedef sinks::synchronous_sink< sinks::text_multifile_backend > file_sink;
    boost::shared_ptr< file_sink > sink(new file_sink);

    // Set up how the file names will be generated
    sink->locked_backend()->set_file_name_composer(
        sinks::file::as_file_name_composer(expr::stream << path <<expr::attr< std::string >("LogName") << ".log"));

    //sink->locked_backend()->auto_flush(true);

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
    po::options_description desc("FreeTure options");
    desc.add_options()
      ("mode,m",        po::value<int>(),                                                               "FreeTure modes :\n- MODE 1 : Check configuration file.\n- MODE 2 : Continuous acquisition.\n- MODE 3 : Meteor detection.\n- MODE 4 : Single acquisition.\n- MODE 5 : Clean logs.")
      ("time,t",        po::value<int>(),                                                               "Execution time (s) of meteor detection mode.")
      ("width",         po::value<int>(),                                                               "Image width.")
      ("height",        po::value<int>(),                                                               "Image height.")
      ("cfg,c",         po::value<string>()->default_value(string(CFG_PATH) + "configuration.cfg"),     "Configuration file's path.")
      ("format,f",      po::value<int>()->default_value(0),                                             "Index of pixel format.")
      ("bmp",                                                                                           "Save .bmp.")
      ("fits",                                                                                          "Save fits2D.")
      ("gain,g",        po::value<int>(),                                                               "Define gain.")
      ("exposure,e",    po::value<double>(),                                                            "Define exposure.")
      ("version,v",                                                                                     "Print FreeTure version.")
      ("display",                                                                                       "Display the grabbed frame.")
      ("listdevices,l",                                                                                 "List connected devices.")
      ("listformats",                                                                                   "List device's available pixel formats.")
      ("id,d",          po::value<int>(),                                                               "Camera to use. List devices to get IDs.")
      ("filename,n",    po::value<string>()->default_value("snap"),                                     "Name to use when a single frame is captured.")
      ("sendbymail,s",                                                                                  "Send single capture by mail. Require -c option.")
      ("savepath,p",    po::value<string>()->default_value("./"),                                       "Save path.");

    po::variables_map vm;

    try{

        int         mode            = -1;
        int         executionTime   = 0;
        string      configPath      = string(CFG_PATH) + "configuration.cfg";
        string      savePath        = "./";
        int         acqFormat       = 0;
        int         acqWidth        = 0;
        int         acqHeight       = 0;
        int         gain            = 0;
        double      exp             = 0;
        string      version         = string(VERSION);
        int         devID           = 0;
        string      fileName        = "snap";
        bool        listFormats     = false;

        po::store(po::parse_command_line(argc, argv, desc), vm);

        ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        ///%%%%%%%%%%%%%%%%%%%%%%% PRINT FREETURE VERSION %%%%%%%%%%%%%%%%%%%%%%%%
        ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

        if(vm.count("version")){

            std::cout << "Current version : " << version << endl;

        ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        ///%%%%%%%%%%%%%%%%%%%%%%% LIST CONNECTED DEVICES %%%%%%%%%%%%%%%%%%%%%%%%
        ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

        }else if(vm.count("listdevices")){

            Device device;
            device.listDevices(true);

        ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        ///%%%%%%%%%%%%%%%%%%%%%%% GET AVAILABLE PIXEL FORMATS %%%%%%%%%%%%%%%%%%%
        ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         }else if(vm.count("listformats")){

            if(vm.count("id")) devID = vm["id"].as<int>();

            Device *device = new Device();
            device->listDevices(false);

            if(!device->createCamera(devID, true))
                delete device;

            device->getSupportedPixelFormats();
            delete device;

        }else if(vm.count("mode")){

            mode = vm["mode"].as<int>();
            if(vm.count("cfg")) configPath = vm["cfg"].as<string>();
            CfgParam cfg(configPath);

            switch(mode){

                case 1 :

                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%% MODE 1 : TEST/CHECK CONFIGURATION FILE %%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    {

                        std::cout << "================================================" << endl;
                        std::cout << "====== FREETURE - Test/Check configuration =====" << endl;
                        std::cout << "================================================" << endl << endl;

                        cfg.showErrors = true;
                        cfg.allParamAreCorrect();

                    }

                    break;

                case 2 :

                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%% MODE 2 : CONTINUOUS ACQUISITION %%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    {

                        std::cout << "================================================" << endl;
                        std::cout << "========== FREETURE - Continuous mode ==========" << endl;
                        std::cout << "================================================" << endl << endl;

                        /// --------------------- Manage program options -----------------------

                        // Acquisition format.
                        if(vm.count("format")) acqFormat = vm["format"].as<int>();
                        // Cam id.
                        if(vm.count("id")) devID = vm["id"].as<int>();
                        // Cam width size
                        if(vm.count("width")) acqWidth = vm["width"].as<int>();
                        // Cam height size
                        if(vm.count("height")) acqHeight = vm["height"].as<int>();
                        // Gain value.
                        if(vm.count("gain")) gain = vm["gain"].as<int>();
                        // Exposure value.
                        if(vm.count("exposure")) exp = vm["exposure"].as<double>();

                        EParser<CamPixFmt> fmt;
                        string fstring = fmt.getStringEnum(static_cast<CamPixFmt>(acqFormat));
                        if(fstring == "")
                            throw ">> Pixel format specified not found.";

                        cout << "------------------------------------------------" << endl;
                        cout << "CAM ID    : " << devID << endl;
                        cout << "FORMAT    : " << fstring << endl;
                        cout << "GAIN      : " << gain << endl;
                        cout << "EXPOSURE  : " << exp << endl;
                        cout << "------------------------------------------------" << endl << endl;

                        Device *device = new Device();
                        device->listDevices(false);
                        device->mFormat = static_cast<CamPixFmt>(acqFormat);
                        if(!device->createCamera(devID, true)) {
                            delete device;
                            throw "Fail to create device.";
                        }

                        if(acqWidth != 0 && acqHeight != 0)
                            device->setCameraSize(acqWidth, acqHeight);
                        else
                            device->setCameraSize();

                        if(!device->setCameraPixelFormat()) {
                            delete device;
                            throw "Fail to set format";
                        }
                        device->setCameraFPS();
                        device->setCameraExposureTime(exp);
                        device->setCameraGain(gain);
                        device->initializeCamera();
                        device->startCamera();
                        if(vm.count("display"))
                            namedWindow("FreeTure (ESC to stop)", WINDOW_NORMAL);

                        #ifdef LINUX
                        nonblock(1);
                        #endif

                        char hitKey;
                        int interruption = 0;

                        while(!interruption) {

                            Frame frame;

                            double tacq = (double)getTickCount();
                            if(device->runContinuousCapture(frame)){
                                tacq = (((double)getTickCount() - tacq)/getTickFrequency())*1000;
                                std::cout << " >> [ TIME ACQ ] : " << tacq << " ms" << endl;

                                if(vm.count("display")) {
                                    imshow("FreeTure (ESC to stop)", frame.mImg);
                                    waitKey(1);
                                }
                            }

                            /// Stop freeture if escape is pressed.
                            #ifdef WINDOWS

                            //Sleep(1000);
                            // Exit if ESC is pressed.
                            if(GetAsyncKeyState(VK_ESCAPE)!=0)
                                interruption = 1;

                            #else
                            #ifdef LINUX

                            interruption = kbhit();
                            if(interruption !=0) {
                                hitKey = fgetc(stdin);
                                if(hitKey == 27) interruption = 1;
                                else interruption = 0;
                            }

                            #endif
                            #endif
                        }

                        #ifdef LINUX
                        nonblock(0);
                        #endif

                        device->stopCamera();
                        delete device;

                    }

                    break;

                case 3 :

                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%% MODE 3 : METEOR DETECTION %%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    {

                        std::cout << "================================================" << endl;
                        std::cout << "======= FREETURE - Meteor detection mode =======" << endl;
                        std::cout << "================================================" << endl << endl;

                        /// ------------------------------------------------------------------
                        /// --------------------- LOAD FREETURE PARAMETERS -------------------
                        /// ------------------------------------------------------------------

                        cfg.showErrors = true;
                        if(!cfg.allParamAreCorrect())
                            throw "Configuration file is not correct. Fail to launch detection mode.";

                        vector<string> logFiles;
                        logFiles.push_back("MAIN_THREAD.log");
                        logFiles.push_back("ACQ_THREAD.log");
                        logFiles.push_back("DET_THREAD.log");
                        logFiles.push_back("STACK_THREAD.log");

                        Logger logSystem(cfg.getLogParam().LOG_PATH, cfg.getLogParam().LOG_ARCHIVE_DAY, cfg.getLogParam().LOG_SIZE_LIMIT, logFiles);

                        /// ------------------------------------------------------------------
                        /// -------------------------- MANAGE LOG ----------------------------
                        /// ------------------------------------------------------------------

                        path pLog(cfg.getLogParam().LOG_PATH);

                        if(!boost::filesystem::exists(pLog)){

                            if(!create_directory(pLog))
                                throw "> Failed to create a directory for logs files.";
                            else
                                cout << "> Log directory created : " << pLog << endl;
                        }

                        init_log(cfg.getLogParam().LOG_PATH + "/", cfg.getLogParam().LOG_SEVERITY);
                        src::severity_logger< LogSeverityLevel > slg;
                        slg.add_attribute("ClassName", boost::log::attributes::constant<std::string>("main.cpp"));
                        BOOST_LOG_SCOPED_THREAD_TAG("LogName", "MAIN_THREAD");
                        BOOST_LOG_SEV(slg,notification) << "\n";
                        BOOST_LOG_SEV(slg,notification) << "==============================================";
                        BOOST_LOG_SEV(slg,notification) << "====== FREETURE - Meteor detection mode ======";
                        BOOST_LOG_SEV(slg,notification) << "==============================================";

                        /// ------------------------------------------------------------------
                        /// ------------------------- SHARED RESSOURCES ----------------------
                        /// ------------------------------------------------------------------

                        // Circular buffer to store last n grabbed frames.
                        boost::circular_buffer<Frame> frameBuffer(cfg.getDetParam().ACQ_BUFFER_SIZE * cfg.getCamParam().ACQ_FPS);
                        boost::mutex frameBuffer_m;
                        boost::condition_variable frameBuffer_c;

                        bool signalDet = false;
                        boost::mutex signalDet_m;
                        boost::condition_variable signalDet_c;

                        bool signalStack = false;
                        boost::mutex signalStack_m;
                        boost::condition_variable signalStack_c;

                        boost::mutex cfg_m;

                        /// ------------------------------------------------------------------
                        /// --------------------------- CREATE THREAD ------------------------
                        /// ------------------------------------------------------------------

                        AcqThread   *acqThread      = NULL;
                        DetThread   *detThread      = NULL;
                        StackThread *stackThread    = NULL;

                        try {

                            // Create detection thread.
                            if(cfg.getDetParam().DET_ENABLED) {

                                BOOST_LOG_SEV(slg, normal) << "Start to create detection thread.";

                                detThread = new DetThread(  &frameBuffer,
                                                            &frameBuffer_m,
                                                            &frameBuffer_c,
                                                            &signalDet,
                                                            &signalDet_m,
                                                            &signalDet_c,
                                                            cfg.getDetParam(),
                                                            cfg.getDataParam(),
                                                            cfg.getMailParam(),
                                                            cfg.getStationParam(),
                                                            cfg.getFitskeysParam(),
                                                            cfg.getCamParam().ACQ_FORMAT);

                                if(!detThread->startThread())
                                    throw "Fail to start detection thread.";

                            }

                            // Create stack thread.
                            if(cfg.getStackParam().STACK_ENABLED) {

                                BOOST_LOG_SEV(slg, normal) << "Start to create stack Thread.";

                                stackThread = new StackThread(  &signalStack,
                                                                &signalStack_m,
                                                                &signalStack_c,
                                                                &frameBuffer,
                                                                &frameBuffer_m,
                                                                &frameBuffer_c,
                                                                cfg.getDataParam(),
                                                                cfg.getStackParam(),
                                                                cfg.getStationParam(),
                                                                cfg.getCamParam().ACQ_FORMAT,
                                                                cfg.getFitskeysParam());

                                if(!stackThread->startThread())
                                    throw "Fail to start stack thread.";

                            }

                            // Create acquisition thread.
                            acqThread = new AcqThread(  &frameBuffer,
                                                        &frameBuffer_m,
                                                        &frameBuffer_c,
                                                        &signalStack,
                                                        &signalStack_m,
                                                        &signalStack_c,
                                                        &signalDet,
                                                        &signalDet_m,
                                                        &signalDet_c,
                                                        detThread,
                                                        stackThread,
                                                        cfg.getDeviceID(),
                                                        cfg.getDataParam(),
                                                        cfg.getStackParam(),
                                                        cfg.getStationParam(),
                                                        cfg.getDetParam(),
                                                        cfg.getCamParam(),
                                                        cfg.getFramesParam(),
                                                        cfg.getVidParam(),
                                                        cfg.getFitskeysParam());

                            if(!acqThread->startThread()) {

                                throw "Fail to start acquisition thread.";

                            }else {

                                BOOST_LOG_SEV(slg, normal) << "Success to start acquisition Thread.";

                                #ifdef LINUX

                                BOOST_LOG_SEV(slg, notification) << "This is the process : " << (unsigned long)getpid();

                                memset(&act, 0, sizeof(act));
                                act.sa_sigaction = sigTermHandler;
                                act.sa_flags = SA_SIGINFO;
                                sigaction(SIGTERM,&act,NULL);

                                nonblock(1);

                                #endif

                                int cptTime = 0;
                                bool waitLogTime = true;
                                char hitKey;
                                int interruption = 0;

                                /// ------------------------------------------------------------------
                                /// ----------------------------- MAIN LOOP --------------------------
                                /// ------------------------------------------------------------------

                                while(!sigTermFlag && !interruption) {

                                    /// Stop freeture if escape is pressed.
                                    #ifdef WINDOWS

                                    Sleep(1000);
                                    // Exit if ESC is pressed.
                                    if(GetAsyncKeyState(VK_ESCAPE)!=0)
                                        interruption = 1;

                                    #else
                                    #ifdef LINUX

                                    sleep(1);
                                    interruption = kbhit();
                                    if(interruption !=0) {
                                        hitKey = fgetc(stdin);
                                        if(hitKey == 27) interruption = 1;
                                        else interruption = 0;
                                    }

                                    #endif
                                    #endif

                                    /// Monitors logs.
                                    logSystem.monitorLog();

                                    /// Stop freeture according time execution option.
                                    if(executionTime != 0) {

                                        if(cptTime > executionTime){
                                            break;
                                        }

                                        cptTime ++;

                                    }

                                    /// Stop freeture if one of the thread is stopped.
                                    if(acqThread->getThreadStatus()){
                                        break;
                                    }

                                    if(detThread != NULL){
                                        if(!detThread->getRunStatus()){
                                            BOOST_LOG_SEV(slg, critical) << "DetThread not running. Stopping the process ...";
                                            break;
                                        }
                                    }

                                    if(stackThread != NULL){
                                        if(!stackThread->getRunStatus()){
                                            BOOST_LOG_SEV(slg, critical) << "StackThread not running. Stopping the process ...";
                                            break;
                                        }
                                    }
                                }

                                #ifdef LINUX
                                nonblock(0);
                                #endif
                            }

                        }catch(exception& e) {

                            cout << e.what() << endl;
                            BOOST_LOG_SEV(slg, critical) << e.what();

                        }catch(const char * msg) {

                            cout << msg << endl;
                            BOOST_LOG_SEV(slg,critical) << msg;

                        }

                        if(acqThread != NULL) {
                            acqThread->stopThread();
                            delete acqThread;
                        }

                        if(detThread != NULL) {
                            detThread->stopThread();
                            delete detThread;
                        }

                        if(stackThread != NULL){
                            stackThread->stopThread();
                            delete stackThread;

                        }

                    }

                    break;

                case 4 :

                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%% MODE 4 : SINGLE ACQUISITION %%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    {

                        std::cout << "================================================" << endl;
                        std::cout << "======== FREETURE - Single acquisition =========" << endl;
                        std::cout << "================================================" << endl << endl;

                        /// ------------------------------------------------------------------
                        /// --------------------- MANAGE PROGRAM OPTIONS ---------------------
                        /// ------------------------------------------------------------------

                        // Path where to save files.
                        if(vm.count("savepath")) savePath = vm["savepath"].as<string>();
                        // Acquisition pixel format.
                        if(vm.count("format")) acqFormat = vm["format"].as<int>();
                        // Cam width size.
                        if(vm.count("width")) acqWidth = vm["width"].as<int>();
                        // Cam height size
                        if(vm.count("height")) acqHeight = vm["height"].as<int>();
                        // Cam id.
                        if(vm.count("id")) devID = vm["id"].as<int>();
                        // Gain value.
                        if(vm.count("gain")) gain = vm["gain"].as<int>();
                        // Exposure value.
                        if(vm.count("exposure")) exp = vm["exposure"].as<double>();
                        // Filename.
                        if(vm.count("filename")) fileName = vm["filename"].as<string>();

                        EParser<CamPixFmt> fmt;
                        string fstring = fmt.getStringEnum(static_cast<CamPixFmt>(acqFormat));
                        if(fstring == "")
                            throw ">> Pixel format specified not found.";

                        cout << "------------------------------------------------" << endl;
                        cout << "CAM ID    : " << devID << endl;
                        cout << "FORMAT    : " << fstring << endl;
                        cout << "GAIN      : " << gain << endl;
                        cout << "EXPOSURE  : " << exp << endl;
                        if(acqWidth > 0 && acqHeight > 0) cout << "SIZE      : " << acqWidth << "x" << acqHeight << endl;
                        cout << "SAVE PATH : " << savePath << endl;
                        cout << "FILENAME  : " << fileName << endl;
                        cout << "------------------------------------------------" << endl << endl;

                        /// ------------------------------------------------------------------
                        /// ----------------------- MANAGE FILE NAME -------------------------
                        /// ------------------------------------------------------------------

                        int filenum = 0;
                        bool increment = false;
                        path p(savePath);

                        // Search previous captures in the directory.
                        for(directory_iterator file(p);file!= directory_iterator(); ++file) {

                            path curr(file->path());

                            if(is_regular_file(curr)) {

                                list<string> ch;
                                string fname = curr.filename().string();
                                Conversion::stringTok(ch, fname.c_str(), "-");

                                int i = 0;
                                int n = 0;

                                if(ch.front() == fileName && ch.size() == 2) {

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

                        /// ------------------------------------------------------------------
                        /// ---------------------- RUN SINGLE CAPTURE ------------------------
                        /// ------------------------------------------------------------------

                        Frame frame;
                        frame.mExposure = exp;
                        frame.mGain = gain;
                        frame.mFormat = static_cast<CamPixFmt>(acqFormat);
                        frame.mHeight = acqHeight;
                        frame.mWidth = acqWidth;

                        Device *device = new Device();
                        device->listDevices(false);

                        if(!device->createCamera(devID, false)) {
                            delete device;
                            throw ">> Fail to create device.";
                        }

                        if(!device->runSingleCapture(frame)){
                            delete device;
                            throw ">> Single capture failed.";
                        }

                        delete device;

                        cout << ">> Single capture succeed." << endl;

                        /// ------------------------------------------------------------------
                        /// ------------------- SAVE / DISPLAY CAPTURE -----------------------
                        /// ------------------------------------------------------------------

                        if(frame.mImg.data) {

                            // Save the frame in BMP.
                            if(vm.count("bmp")) {

                                cout << ">> Saving bmp file ..." << endl;

                                Mat temp1, newMat;
                                frame.mImg.copyTo(temp1);

                                if(frame.mFormat == MONO12){
                                    newMat = ImgProcessing::correctGammaOnMono12(temp1, 2.2);
                                    Mat temp = Conversion::convertTo8UC1(newMat);
                                }else {
                                    newMat = ImgProcessing::correctGammaOnMono8(temp1, 2.2);
                                }

                                SaveImg::saveBMP(newMat, savePath + fileName + "-" + Conversion::intToString(filenum));
                                cout << ">> Bmp saved : " << savePath << fileName << "-" << Conversion::intToString(filenum) << ".bmp" << endl;

                            }

                            // Save the frame in Fits 2D.
                            if(vm.count("fits")) {

                                cout << ">> Saving fits file ..." << endl;

                                Fits fh;
                                bool useCfg = false;
                                Fits2D newFits(savePath);

                                cfg.showErrors = true;
                                if(cfg.stationParamIsCorrect() && cfg.fitskeysParamIsCorrect()) {
                                    useCfg = true;
                                    double  debObsInSeconds = frame.mDate.hours*3600 + frame.mDate.minutes*60 + frame.mDate.seconds;
                                    double  julianDate      = TimeDate::gregorianToJulian(frame.mDate);
                                    double  julianCentury   = TimeDate::julianCentury(julianDate);
                                    double  sideralT        = TimeDate::localSideralTime_2(julianCentury, frame.mDate.hours, frame.mDate.minutes, (int)frame.mDate.seconds, fh.kSITELONG);
                                    newFits.kCRVAL1 = sideralT;
                                    newFits.loadKeys(cfg.getFitskeysParam(), cfg.getStationParam());

                                }

                                newFits.kGAINDB = (int)gain;
                                newFits.kELAPTIME = exp/1000000.0;
                                newFits.kEXPOSURE = exp/1000000.0;
                                newFits.kONTIME = exp/1000000.0;
                                newFits.kDATEOBS = TimeDate::getIsoExtendedFormatDate(frame.mDate);
                                newFits.kCTYPE1 = "RA---ARC";
                                newFits.kCTYPE2 = "DEC--ARC";
                                newFits.kEQUINOX = 2000.0;

                                if(frame.mFormat == MONO12){
                                    // Create FITS image with BITPIX = SHORT_IMG (16-bits signed integers), pixel with TSHORT (signed short)
                                    if(newFits.writeFits(frame.mImg, S16, fileName + "-" + Conversion::intToString(filenum)))
                                        cout << ">> Fits saved in : " << savePath << fileName << "-" << Conversion::intToString(filenum) << ".fit" << endl;
                                }else{
                                    // Create FITS image with BITPIX = BYTE_IMG (8-bits unsigned integers), pixel with TBYTE (8-bit unsigned byte)
                                    if(newFits.writeFits(frame.mImg, UC8, fileName + "-" + Conversion::intToString(filenum)))
                                        cout << ">> Fits saved in : " << savePath << fileName << "-" << Conversion::intToString(filenum) << ".fit" << endl;

                                }

                                // Send fits by mail if configuration file is correct.
                                if(vm.count("sendbymail") && useCfg) {

                                    if(cfg.mailParamIsCorrect()) {

                                        vector<string> mailAttachments;
                                        mailAttachments.push_back(savePath + fileName + "-" + Conversion::intToString(filenum) + ".fit");

                                        SMTPClient::sendMail(cfg.getMailParam().MAIL_SMTP_SERVER,
                                                            cfg.getMailParam().MAIL_SMTP_LOGIN,
                                                            cfg.getMailParam().MAIL_SMTP_PASSWORD,
                                                            "freeture@snap",
                                                            cfg.getMailParam().MAIL_RECIPIENTS,
                                                            fileName + "-" + Conversion::intToString(filenum) + ".fit",
                                                            " Exposure time : " + Conversion::intToString((int)exp) + "\n Gain : " + Conversion::intToString((int)gain),
                                                            mailAttachments,
                                                            cfg.getMailParam().MAIL_CONNECTION_TYPE);

                                    }
                                }
                            }

                            // Display the frame in an opencv window
                            if(vm.count("display")) {

                                cout << ">> Display single capture." << endl;

                                Mat temp, temp1;
                                frame.mImg.copyTo(temp1);

                                if(frame.mFormat == MONO12) {
                                    Mat gammaCorrected = ImgProcessing::correctGammaOnMono12(temp1,2.2);
                                    temp = Conversion::convertTo8UC1(gammaCorrected);
                                }else{
                                    temp = ImgProcessing::correctGammaOnMono8(temp1,2.2);
                                }

                                namedWindow("FreeTure (Press a key to close)", WINDOW_NORMAL);
                                imshow("FreeTure (Press a key to close)", temp);
                                waitKey(0);

                            }
                        }
                    }

                    break;

                case 5 :

                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%% MODE 5 : CLEAN LOGS %%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    // Simply remove all log directory contents.

                    {

                        boost::filesystem::path p(cfg.getLogParam().LOG_PATH);

                        if(boost::filesystem::exists(p)){

                            boost::filesystem::remove_all(p);

                            cout << "Clean log completed." << endl;

                        }else {

                            cout << "Log directory not found." << endl;

                        }

                    }

                    break;

                case 6 :

                    {

                    }

                    break;

                default :

                    {

                        cout << "MODE " << mode << " is not available. Correct modes are : " << endl
                                                                                        << endl;
                        cout << "[1] Check configuration file."                         << endl;
                        cout << "[2] Run continous acquisition."                        << endl;
                        cout << "[3] Run meteor detection."                             << endl;
                        cout << "[4] Run single capture."                               << endl;
                        cout << "[5] Clean logs."                                       << endl << endl;

                        cout << "Execute freeture command to see options." << endl;
                    }

                    break;

            }

        ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%% PRINT HELP %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

        }else {

            std::cout << desc;

        }

    }catch(exception& e) {

        cout << ">> Error : " << e.what() << endl;

    }catch(const char * msg) {

        cout << ">> Error : " << msg << endl;

    }

    po::notify(vm);

    return 0 ;

}

/*
case 6 :

    {
    vector<string> mMailRecipients, mailAttachments;

    mMailRecipients.push_back("fripon@ceres.geol.u-psud.fr");
    SMTPClient::sendMail(   "10.8.0.1",
                            "",
                            "",
                            "freeture@sendmail.com",
                            mMailRecipients,
                            "ORSAY-20151116T191303",
                            "message",
                            mailAttachments,
                            NO_SECURITY);
    }

    break;
*/
