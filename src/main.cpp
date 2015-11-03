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
*   Last modified:      20/07/2015
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
    po::options_description desc("Available options");
    desc.add_options()
      ("mode,m",        po::value<int>()->default_value(1),                                             "Execution mode of the program")
      ("time,t",        po::value<int>(),                                                               "Execution time of the program in seconds")
      ("width",         po::value<int>(),                                                               "Camera image width.")
      ("height",        po::value<int>(),                                                               "Camera image height")
      ("help,h",                                                                                        "Print help messages")
      ("cfg,c",         po::value<string>()->default_value(string(CFG_PATH) + "configuration.cfg"),     "Configuration file's path")
      ("bitdepth,f",    po::value<int>()->default_value(8),                                             "Bit depth of a frame")
      ("bmp",           po::value<bool>()->default_value(false),                                        "Save .bmp")
      ("fits",          po::value<bool>()->default_value(false),                                        "Save fits2D")
      ("gain,g",        po::value<int>(),                                                               "Define gain")
      ("exposure,e",    po::value<double>(),                                                            "Define exposure")
      ("version,v",                                                                                     "Get program version")
      ("display",       po::value<bool>()->default_value(false),                                        "In mode 4 : Display the grabbed frame")
      ("id,i",          po::value<int>(),                                                               "Camera ID. Run mode 1 to know the ID.")
      ("filename,n",    po::value<string>()->default_value("snap"),                                     "Name to use when a single frame is captured")
      ("sendbymail,s",  po::value<bool>()->default_value(false),                                        "Send capture in fits by mail (option --fits must be enabledd). Require -c option with correct configuration for mail.")
      ("savepath,p",    po::value<string>()->default_value("./"),                                       "Save path");

    po::variables_map vm;

    try{

        int     mode            = 1;
        int     executionTime   = 0;
        string  configPath      = string(CFG_PATH) + "configuration.cfg";
        string  savePath        = "./";
        int     acqFormat       = 8;
        int     acqWidth        = 0;
        int     acqHeight       = 0;
        bool    saveBmp         = false;
        bool    saveFits2D      = false;
        int     gain            = 0;
        double  exp             = 0;
        string  version         = string(VERSION);
        bool    display         = false;
        int     devID           = 0;
        string  fileName        = "snap";
        bool    sendByMail      = false;

        po::store(po::parse_command_line(argc, argv, desc), vm);

        if(vm.count("mode"))
            mode = vm["mode"].as<int>();

        if(vm.count("time"))
            executionTime = vm["time"].as<int>();

        if(vm.count("cfg"))
            configPath = vm["cfg"].as<string>();

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

                        Device device;
                        device.listDevices(true);

                    }

                    break;

                case 2 :

                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%% MODE 2 : TEST/CHECK CONFIGURATION %%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    {

                        std::cout << "================================================" << endl;
                        std::cout << "====== FREETURE - Test/Check configuration =====" << endl;
                        std::cout << "================================================" << endl << endl;

                        std::cout << "Mode 2 disabled in this version." << endl;

                        /*

                        //The static member function boost::thread::hardware_concurrency() returns the number
                        //of threads that can physically be executed at the same time, based on the underlying
                        //number of CPUs or CPU cores. Calling this function on a dual-core processor returns a
                        //value of 2. This function provides a simple method to identify the theoretical maximum
                        //number of threads that should be used.
                        std::cout << "CORE DETECTED : " << boost::thread::hardware_concurrency()<< endl << endl;

                        */

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

                        /// ------------------------------------------------------------------
                        /// --------------------- LOAD FREETURE PARAMETERS -------------------
                        /// ------------------------------------------------------------------

                        int ACQ_BUFFER_SIZE, ACQ_FPS, LOG_ARCHIVE_DAY, LOG_SIZE_LIMIT;
                        bool DET_ENABLED, STACK_ENABLED;
                        string LOG_PATH, log_severity;

                        boost::filesystem::path pcfg(configPath);
                        if(!boost::filesystem::exists(pcfg))
                            throw "Configuration file not found.";

                        Configuration cfg;
                        if(!cfg.Load(configPath))
                            throw "Fail to load parameters from configuration file.";

                        // Get size of the frame buffer.
                        if(!cfg.Get("ACQ_BUFFER_SIZE", ACQ_BUFFER_SIZE))
                            throw "Fail to load ACQ_BUFFER_SIZE from configuration file.";

                        // Get acquisition frequency.
                        if(!cfg.Get("ACQ_FPS", ACQ_FPS))
                            throw "Fail to load ACQ_BUFFER_SIZE from configuration file.";

                        // Detection enabled or not.
                        if(!cfg.Get("DET_ENABLED", DET_ENABLED))
                            throw "Fail to load DET_ENABLED from configuration file.";

                        // Stack enabled or not.
                        if(!cfg.Get("STACK_ENABLED", STACK_ENABLED))
                            throw "Fail to load STACK_ENABLED from configuration file.";

                        // Get log path.
                        if(!cfg.Get("LOG_PATH", LOG_PATH))
                            throw "Fail to load LOG_PATH from configuration file.";

                        if(!cfg.Get("LOG_ARCHIVE_DAY", LOG_ARCHIVE_DAY))
                            throw "Fail to load LOG_ARCHIVE_DAY from configuration file.";

                        if(!cfg.Get("LOG_SIZE_LIMIT", LOG_SIZE_LIMIT))
                            throw "Fail to load LOG_SIZE_LIMIT from configuration file.";

                        vector<string> logFiles;
                        logFiles.push_back("MAIN_THREAD.log");
                        logFiles.push_back("ACQ_THREAD.log");
                        logFiles.push_back("DET_THREAD.log");
                        logFiles.push_back("STACK_THREAD.log");

                        Logger logSystem(LOG_PATH, LOG_ARCHIVE_DAY, LOG_SIZE_LIMIT, logFiles);

                        // Get log severity.
                        if(!cfg.Get("LOG_SEVERITY", log_severity))
                            throw "Fail to load LOG_SEVERITY from configuration file.";
                        EParser<LogSeverityLevel> log_sev;
                        LogSeverityLevel LOG_SEVERITY = log_sev.parseEnum("LOG_SEVERITY", log_severity);

                        /// ------------------------------------------------------------------
                        /// -------------------------- MANAGE LOG ----------------------------
                        /// ------------------------------------------------------------------

                        path pLog(LOG_PATH);

                        if(!boost::filesystem::exists(pLog)){

                            if(!create_directory(pLog))
                                throw "> Failed to create a directory for logs files.";
                            else
                                cout << "> Log directory created : " << pLog << endl;
                        }

                        init_log(LOG_PATH + "/", LOG_SEVERITY);
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

                        /// ------------------------------------------------------------------
                        /// --------------------------- CREATE THREAD ------------------------
                        /// ------------------------------------------------------------------

                        AcqThread   *acqThread      = NULL;
                        DetThread   *detThread      = NULL;
                        StackThread *stackThread    = NULL;

                        try {

                            // Create detection thread.
                            if(DET_ENABLED) {

                                BOOST_LOG_SEV(slg, normal) << "Start to create detection thread.";

                                detThread = new DetThread(  configPath,
                                                            &frameBuffer,
                                                            &frameBuffer_m,
                                                            &frameBuffer_c,
                                                            &signalDet,
                                                            &signalDet_m,
                                                            &signalDet_c);

                                if(!detThread->startThread())
                                    throw "Fail to start detection thread.";

                            }

                            // Create stack thread.
                            if(STACK_ENABLED) {

                                BOOST_LOG_SEV(slg, normal) << "Start to create stack Thread.";

                                stackThread = new StackThread(  configPath,
                                                                &signalStack,
                                                                &signalStack_m,
                                                                &signalStack_c,
                                                                &frameBuffer,
                                                                &frameBuffer_m,
                                                                &frameBuffer_c);

                                if(!stackThread->startThread())
                                    throw "Fail to start stack thread.";

                            }

                            // Create acquisition thread.
                            acqThread = new AcqThread(  configPath,
                                                        &frameBuffer,
                                                        &frameBuffer_m,
                                                        &frameBuffer_c,
                                                        &signalStack,
                                                        &signalStack_m,
                                                        &signalStack_c,
                                                        &signalDet,
                                                        &signalDet_m,
                                                        &signalDet_c,
                                                        detThread,
                                                        stackThread);

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

                                            std::cout << "Break main loop"<< endl;

                                            break;
                                        }

                                        cptTime ++;

                                    }

                                    /// Stop freeture if one of the thread is stopped.
                                    if(acqThread->getThreadEndStatus()){
                                        std::cout << "Break main loop" << endl;
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

                        // Display or not the grabbed frame.
                        if(vm.count("display")) display = vm["display"].as<bool>();
                        // Path where to save files.
                        if(vm.count("savepath")) savePath = vm["savepath"].as<string>();
                        // Acquisition format.
                        if(vm.count("bitdepth")) acqFormat = vm["bitdepth"].as<int>();
                        CamBitDepth camFormat;
                        Conversion::intBitDepthToCamBitDepthEnum(acqFormat, camFormat);
                        // Cam width size.
                        if(vm.count("width")) acqWidth = vm["width"].as<int>();
                        // Cam height size
                        if(vm.count("height")) acqHeight = vm["height"].as<int>();
                        // Cam id.
                        if(vm.count("id")) devID = vm["id"].as<int>();
                        // Save bmp.
                        if(vm.count("bmp")) saveBmp = vm["bmp"].as<bool>();
                        // Save fits.
                        if(vm.count("fits")) saveFits2D = vm["fits"].as<bool>();
                        // Gain value.
                        if(vm.count("gain")) gain = vm["gain"].as<int>();
                        // Exposure value.
                        if(vm.count("exposure")) exp = vm["exposure"].as<double>();
                        // Filename.
                        if(vm.count("filename")) fileName = vm["filename"].as<string>();
                        // Send mail.
                        if(vm.count("sendbymail")) sendByMail = vm["sendbymail"].as<bool>();

                        cout << "------------------------------------------------" << endl;
                        cout << "CAM ID    : " << devID << endl;
                        EParser<CamBitDepth> fmt;
                        cout << "FORMAT    : " << fmt.getStringEnum(camFormat) << endl;
                        cout << "GAIN      : " << gain << endl;
                        cout << "EXPOSURE  : " << exp << endl;

                        if(acqWidth > 0 && acqHeight > 0)
                            cout << "SIZE      : " << acqWidth << "x" << acqHeight << endl;

                        cout << "SAVE BMP  : " << saveBmp << endl;
                        cout << "SAVE FITS : " << saveFits2D << endl;
                        cout << "DISPLAY   : " << display << endl;
                        cout << "SAVE PATH : " << savePath << endl;
                        cout << "FILENAME  : " << fileName << endl;
                        cout << "SEND BY MAIL : " << sendByMail << endl;
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
                        frame.mBitDepth = camFormat;
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
                            if(saveBmp) {

                                cout << ">> Saving bmp file ..." << endl;

                                Mat temp1, newMat;
                                frame.mImg.copyTo(temp1);

                                if(camFormat == MONO_12){

                                    newMat = ImgProcessing::correctGammaOnMono12(temp1, 2.2);
                                    Mat temp = Conversion::convertTo8UC1(newMat);

                                }else {

                                    newMat = ImgProcessing::correctGammaOnMono8(temp1, 2.2);
                                }

                                SaveImg::saveBMP(newMat, savePath + fileName + "-" + Conversion::intToString(filenum));
                                cout << ">> Bmp saved : " << savePath << fileName << "-" << Conversion::intToString(filenum) << ".bmp" << endl;

                            }

                            // Save the frame in Fits 2D.
                            if(saveFits2D) {

                                cout << ">> Saving fits file ..." << endl;

                                Fits fitsHeader;

                                // Load keywords from cfg file
                                bool useCfg = false;

                                path c(configPath);

                                if(is_regular_file(c)){

                                    useCfg = true;
                                    fitsHeader.loadKeywordsFromConfigFile(configPath);

                                }else
                                    cout << ">> Can't load fits keywords from configuration file (not found or not exist)." << endl;

                                Fits2D newFits(savePath);
                                newFits.copyKeywords(fitsHeader);
                                newFits.kGAINDB = (int)gain;
                                newFits.kELAPTIME = exp/1000000.0;
                                newFits.kEXPOSURE = exp/1000000.0;
                                newFits.kONTIME = exp/1000000.0;
                                newFits.kDATEOBS = TimeDate::getIsoExtendedFormatDate(frame.mDate);
                                newFits.kCTYPE1 = "RA---ARC";
                                newFits.kCTYPE2 = "DEC--ARC";
                                newFits.kEQUINOX = 2000.0;

                                if(useCfg){

                                    double  debObsInSeconds = frame.mDate.hours*3600 + frame.mDate.minutes*60 + frame.mDate.seconds;
                                    double  julianDate      = TimeDate::gregorianToJulian(frame.mDate);
                                    double  julianCentury   = TimeDate::julianCentury(julianDate);
                                    double  sideralT        = TimeDate::localSideralTime_2(julianCentury, frame.mDate.hours, frame.mDate.minutes, (int)frame.mDate.seconds, fitsHeader.kSITELONG);
                                    newFits.kCRVAL1 = sideralT;

                                }

                                switch(camFormat){

                                    case MONO_8 :

                                        {
                                            // Create FITS image with BITPIX = BYTE_IMG (8-bits unsigned integers), pixel with TBYTE (8-bit unsigned byte)
                                            if(newFits.writeFits(frame.mImg, UC8, fileName + "-" + Conversion::intToString(filenum)))
                                                cout << ">> Fits saved in : " << savePath << fileName << "-" << Conversion::intToString(filenum) << ".fit" << endl;

                                        }

                                        break;

                                    case MONO_12 :

                                        {

                                            // Create FITS image with BITPIX = SHORT_IMG (16-bits signed integers), pixel with TSHORT (signed short)
                                            if(newFits.writeFits(frame.mImg, S16, fileName + "-" + Conversion::intToString(filenum)))
                                                cout << ">> Fits saved in : " << savePath << fileName << "-" << Conversion::intToString(filenum) << ".fit" << endl;

                                        }
                                }

                                // Send fits by mail if configuration file is correct.
                                if(sendByMail && useCfg) {

                                    string smtpServer, smtpLogin = "", smtpPassword = "", smtpConnectionType, mailRecipients;
                                    SmtpSecurity smtpSec;
                                    vector<string> mailAttachments, to;
                                    mailAttachments.push_back(savePath + fileName + "-" + Conversion::intToString(filenum) + ".fit");
                                    bool sendMailStatus = true;

                                    Configuration cfg;
                                    if(!cfg.Load(configPath)) {
                                        cout << "Fail to load parameters from configuration file." << endl;
                                        sendMailStatus = false;
                                    }

                                    if(!cfg.Get("MAIL_SMTP_SERVER", smtpServer)) {
                                        cout << "Fail to load MAIL_SMTP_SERVER from configuration file." << endl;
                                        sendMailStatus = false;
                                    }

                                    if(!cfg.Get("MAIL_CONNECTION_TYPE", smtpConnectionType)) {
                                        cout << "Fail to load MAIL_CONNECTION_TYPE from configuration file.";
                                        sendMailStatus = false;
                                    }else {
                                        EParser<SmtpSecurity> smtp_security;
                                        smtpSec = smtp_security.parseEnum("MAIL_CONNECTION_TYPE", smtpConnectionType);
                                    }

                                    if(smtpSec == USE_SSL) {

                                        if(!cfg.Get("MAIL_SMTP_LOGIN", smtpLogin)) {
                                            cout << "Fail to load MAIL_SMTP_LOGIN from configuration file." << endl;
                                            sendMailStatus = false;
                                        }

                                        if(!cfg.Get("MAIL_SMTP_PASSWORD", smtpPassword)) {
                                            cout << "Fail to load MAIL_SMTP_PASSWORD from configuration file." << endl;
                                            sendMailStatus = false;
                                        }

                                    }

                                    if(!cfg.Get("MAIL_RECIPIENT", mailRecipients)) {
                                        cout << "Fail to load MAIL_RECIPIENT from configuration file." << endl;
                                        sendMailStatus = false;
                                    }else {

                                        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                                        boost::char_separator<char> sep(",");
                                        tokenizer tokens(mailRecipients, sep);

                                        for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
                                            to.push_back(*tok_iter);
                                        }
                                    }

                                    cout << ">> Sending fits by mail..." << endl;

                                    if(sendMailStatus) {

                                        SMTPClient::sendMail(smtpServer,
                                                            smtpLogin,
                                                            smtpPassword,
                                                            "freeture@snap",
                                                            to,
                                                            fileName + "-" + Conversion::intToString(filenum) + ".fit",
                                                            " Exposure time : " + Conversion::intToString((int)exp) + "\n Gain : " + Conversion::intToString((int)gain) + "\n Format : " + Conversion::intToString(acqFormat),
                                                            mailAttachments,
                                                            smtpSec);

                                        cout << ">> Mail successfully sent." << endl;

                                    }else {

                                        cout << ">> Fail to send mail." << endl;

                                    }

                                }
                            }

                            // Display the frame in an opencv window
                            if(display) {

                                cout << ">> Display single capture." << endl;

                                Mat temp, temp1;
                                frame.mImg.copyTo(temp1);

                                if(camFormat == MONO_12) {

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

                        boost::filesystem::path pcfg(configPath);
                        if(!boost::filesystem::exists(pcfg))
                            throw "Configuration.cfg not found.";

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

                case 0 :

                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%% MODE 0 : RUN ACQ TEST %%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    {

                        std::cout << "================================================" << endl;
                        std::cout << "======= FREETURE - Acquisition test mode =======" << endl;
                        std::cout << "================================================" << endl << endl;

                        /// --------------------- Manage program options -----------------------

                        // Display or not the grabbed frame.
                        if(vm.count("display")) display = vm["display"].as<bool>();
                        // Acquisition format.
                        if(vm.count("bitdepth")) acqFormat = vm["bitdepth"].as<int>();
                        CamBitDepth camFormat;
                        Conversion::intBitDepthToCamBitDepthEnum(acqFormat, camFormat);
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

                        cout << "------------------------------------------------" << endl;
                        cout << "CAM ID    : " << devID << endl;
                        EParser<CamBitDepth> fmt;
                        cout << "FORMAT    : " << fmt.getStringEnum(camFormat) << endl;
                        cout << "GAIN      : " << gain << endl;
                        cout << "EXPOSURE  : " << exp << endl;
                        cout << "DISPLAY   : " << display << endl;
                        cout << "------------------------------------------------" << endl << endl;

                        Device *device = new Device();
                        device->listDevices(false);
                        if(!device->createCamera(devID, true)) {
                            delete device;
                            throw ">> Fail to create device.";
                        }

                        if(acqWidth != 0 && acqHeight != 0)
                            device->setCameraSize(acqWidth, acqHeight);
                        else
                            device->setCameraSize();

                        device->setCameraPixelFormat();
                        device->setCameraFPS();
                        device->setCameraExposureTime(exp);
                        device->setCameraGain(gain);
                        device->initializeCamera();
                        device->startCamera();
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

                                if(display) {
                                    imshow("FreeTure (ESC to stop)", frame.mImg);
                                    waitKey(10);
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

                default :

                    {

                        cout << "PLEASE CHOOSE A MODE (e.g. freeture -m 1) "            << endl
                                                                                        << endl;
                        cout << "[1] Show detected cameras."                            << endl;
                        cout << "[2] Check configuration file."                         << endl;
                        cout << "[3] Run meteor detection."                             << endl;
                        cout << "[4] Run single capture."                               << endl;
                        cout << "[5] Clean logs."                                       << endl << endl;

                        cout << "Try help option (-h) for more informations." << endl;

                    }

                    break;

            }

        }

    }catch(exception& e) {

        cout << ">> Error " << e.what() << endl;

    }catch(const char * msg) {

        cout << ">> Error " << msg << endl;

    }

    po::notify(vm);

    //cout << endl << ">> FreeTure ended." << endl << endl;

    return 0 ;

}
