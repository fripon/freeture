/*
								main.cpp

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
 * @file    main.cpp
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 0.2
 * @date    28/08/2014
 */

#include "includes.h"
#include "CameraSimu.h"
#include "CameraVideo.h"
#include "CameraFrames.h"
#include "CameraBasler.h"
#include "CameraDMK.h"
#include "Configuration.h"
#include "Fifo.h"
#include "Frame.h"
#include "DetThread.h"
#include "ImgThread.h"
#include "AstThread.h"
#include "RecThread.h"
#include "RecEvent.h"
#include "SaveImg.h"
#include "Conversion.h"
#include "Fits2D.h"
#include "ManageFiles.h"
#include "TimeDate.h"

#include <boost/filesystem.hpp>

using namespace boost::filesystem;

#ifdef CFITSIO_H
  #include CFITSIO_H
#else
  #include "fitsio.h"
#endif

using namespace std;

namespace po        = boost::program_options;
namespace logging	= boost::log;
namespace sinks		= boost::log::sinks;
namespace attrs		= boost::log::attributes;
namespace src		= boost::log::sources;
namespace expr		= boost::log::expressions;
namespace keywords	= boost::log::keywords;

using boost::shared_ptr;

bool sigTermFlag = false;

#ifdef _LINUX_

struct sigaction act;

/**
 * \brief Terminate signal on the current processus
 */

void sigTermHandler(int signum, siginfo_t *info, void *ptr){

	src::severity_logger< logenum::severity_level > lg;

	BOOST_LOG_SEV(lg, notification) << "Received signal : "<< signum << " from : "<< (unsigned long)info->si_pid;

	sigTermFlag = true;

}

#endif

/**
 * \brief Prepare the log file's structure
 * @param path location of log's files
 */

void init_log(string path){

	// Create a text file sink
    typedef sinks::synchronous_sink< sinks::text_multifile_backend > file_sink;
     boost::shared_ptr< file_sink > sink(new file_sink);

    // Set up how the file names will be generated
    sink->locked_backend()->set_file_name_composer(sinks::file::as_file_name_composer(
        expr::stream << path <<expr::attr< std::string >("LogName") << ".log"));

    //sink->locked_backend()->auto_flush(true);

    // Set the log record formatter
    sink->set_formatter(
        expr::format("%1%: [%2%] [%3%] <%4%> - %5%")
            % expr::attr< unsigned int >("RecordID")
            % expr::attr< boost::posix_time::ptime >("TimeStamp")
            % expr::attr< attrs::current_thread_id::value_type >("ThreadID")
            //% expr::format_named_scope("Scope", keywords::format = "%n (%f:%l)")
            % expr::attr< severity_level >("Severity")
            % expr::smessage
    );

    // Add it to the core
    logging::core::get()->add_sink(sink);

    // Add some attributes too
    logging::add_common_attributes();
    logging::core::get()->add_global_attribute("TimeStamp", attrs::local_clock());
    logging::core::get()->add_global_attribute("RecordID", attrs::counter< unsigned int >());
    logging::core::get()->add_global_attribute("ThreadID", attrs::current_thread_id());
    //logging::core::get()->add_thread_attribute("Scope", attrs::named_scope());

}

/**
 * \brief Main entry of the FreeTure program
 * @param mode -m program exectution mode : scan connected devices, view configuration file, run detection, run single capture
 */
int main(int argc, const char ** argv){

    // declaration of supported program options
    po::options_description desc("Available options");
    desc.add_options()
      ("mode,m",        po::value<int>(),                                                               "Run mode of the program : list connected devices, view configuration file parameters, meteor dectection, single capture")
      ("exectime,t",    po::value<int>(),                                                               "Execution time of the program in seconds")
      ("help,h",                                                                                        "Print help messages")
      ("config,c",      po::value<string>()->default_value(string(CFG_PATH) + "configuration.cfg"), "Define configuration file's path")
      ("bitdepth,i",    po::value<int>()->default_value(8),                                             "Bit depth of a frame")
      ("bmp,b",         po::value<bool>()->default_value(false),                                        "Save in .bmp")
      ("fits,f",        po::value<bool>()->default_value(false),                                        "Save in fits2D")
      ("gain,g",        po::value<int>(),                                                               "Define gain")
      ("exposure,e",    po::value<int>(),                                                               "Define exposure")
      ("version,v",                                                                                     "Get program version")
      ("device,d",      po::value<string>(),                                                            "Name of a device")
      ("savepath,p",    po::value<string>()->default_value("./"),                                       "Save path");

    po::variables_map vm;

    try{

        int mode  = 0;
        int executionTime   = 0;
        string configPath   = string(CFG_PATH) + "configuration.cfg";
        string savePath     = "./";
        string device       = "";
        int acqFormat       = 8;
        bool saveBmp        = false;
        bool saveFits2D     = false;
        int gain            = 300;
        int exp             = 100;
        string version      =  "1";//;string(PACKAGE_VERSION);  //

        po::store(po::parse_command_line(argc, argv, desc), vm);

        if(vm.count("mode"))        mode            = vm["mode"].as<int>();

        if(vm.count("exectime"))    executionTime   = vm["exectime"].as<int>();

        if(vm.count("config"))      configPath      = vm["config"].as<string>();

        // load configuration file and parameters
        Configuration initConfig;

        string	inputDevice;
        string  inputDeviceName;
        string	inputVideoPath;
        string	detMethod;
        string  acqMaskPath;
        string  imgAstroMethod;
        string  dataRecordPath          = "./data/";
        string  stationName             = "STATION";
        string  acqFramesDirectory;
        string  logPath                 = "/tmp/";
        string  logDirName              = "logFiles/";
        string  debugPath               = "./";
        bool    saveMaskedMoon          = false;
        int     acqFrameNumStart;
        int     acqFrameNumStop;
        int		inputIdDevice           = 0;
        int		acqFormatPix            = 8;
        int		acqFPS                  = 30;
        int		imgCapInterval;
        int		imgAstroInterval;
        int     gePrevTime              = 1;
        int     geAfterTime             = 1;
        int     geMax                   = 10;
        int     geMaxTime               = 10;
        bool	detEnable;
        bool    detRecFits3D            = false;
        bool    detRecFits2D            = false;
        bool    detRecPositions         = true;
        bool    detRecBmp               = false;
        bool    detRecAvi               = false;
        bool    detRecMapGE             = true;
        bool    detRecShape             = false;
        bool    debug                   = false;
        bool    detRecTrail             = false;
        bool    detMaskMoon             = false;
        bool    detDownsample           = false;
        bool    acqMaskEnable           = false;
        bool	imgCapEnable            = false;
        bool	imgCapGammaCorrEnable;
        bool	imgAstroEnable;
        double	imgCapExpTime;
        double	imgCapGammaCorrValue;
        double	imgAstroExpTime;
        double  longitude               = 0.0;
        int     acqInitialExposure	    = 0;
        int     acqInitialGain		    = 0;
        bool    saveConfigFileCopy      = false;

        /****************************************************************
        *************** MODE 1 : LIST CONNECTED CAMERAS *****************
        ****************************************************************/

        if(vm.count("version")){

            cout << "Current version : " << version << endl;

        }else if(vm.count("help")){

            cout << desc;

        }else if(mode == 1){

            initConfig.Load(configPath);

            initConfig.Get("logPath", logPath);

            logPath = logPath + logDirName;

            if(!ManageFiles::createDirectory( logPath )){

                throw "Can't create location for log files";

            }

            // log configuration
            init_log(logPath);
            src::severity_logger< severity_level > slg;
            BOOST_LOG_SCOPED_THREAD_TAG("LogName", "mainThread");

            initConfig.Get("inputDevice", inputDevice);

            if ( inputDevice == "BASLER" ){

                CameraBasler *baslerCam = new CameraBasler();
                baslerCam->getListCameras();

                if(baslerCam!= NULL) delete baslerCam;

            }else{

                CameraDMK *dmkCam = new CameraDMK();
                dmkCam->getListCameras();
                if(dmkCam!= NULL) delete dmkCam;

            }

        /***************************************************************
        *************** MODE 2 : VIEW CONFIGURATION FILE ***************
        ****************************************************************/

        }else if(mode == 2){

            initConfig.Load(configPath);

            cout<<"=> inputDevice               : "<<inputDevice            <<endl;
            cout<<"=> inputDeviceName           : "<<inputDeviceName        <<endl;
            cout<<"=> inputIdDevice             : "<<inputIdDevice          <<endl;
            cout<<"=> inputVideoPath            : "<<inputVideoPath         <<endl;
            cout<<"=> acqFormatPix			    : "<<acqFormatPix           <<endl;
            cout<<"=> detEnable                 : "<<detEnable              <<endl;
            cout<<"=> detMethod                 : "<<detMethod              <<endl;
            cout<<"=> detRecFits3D              : "<<detRecFits3D           <<endl;
            cout<<"=> detRecFits2D              : "<<detRecFits2D           <<endl;
            cout<<"=> detRecBmp                 : "<<detRecBmp              <<endl;
            cout<<"=> detRecAvi                 : "<<detRecAvi              <<endl;
            cout<<"=> detRecMapGE               : "<<detRecMapGE            <<endl;
            cout<<"=> detRecShape               : "<<detRecShape            <<endl;
            cout<<"=> detRecTrail               : "<<detRecTrail            <<endl;
            cout<<"=> acqMaskPath               : "<<acqMaskPath            <<endl;
            cout<<"=> imgCapEnable              : "<<imgCapEnable           <<endl;
            cout<<"=> imgCapGammaCorrEnable     : "<<imgCapGammaCorrEnable  <<endl;
            cout<<"=> imgCapGammaCorrValue      : "<<imgCapGammaCorrValue   <<endl;
            cout<<"=> imgCapExpTime             : "<<imgCapExpTime          <<endl;
            cout<<"=> imgCapInterval            : "<<imgCapInterval         <<endl;
            cout<<"=> imgAstroEnable            : "<<imgAstroEnable         <<endl;
            cout<<"=> imgAstroExpTime           : "<<imgAstroExpTime        <<endl;
            cout<<"=> imgAstroInterval          : "<<imgAstroInterval       <<endl;
            cout<<"=> imgAstroMethod            : "<<imgAstroMethod         <<endl;

        /***************************************************************
        ****************** MODE 3 : RUN DETECTION MODE *****************
        ****************************************************************/

        }else if(mode == 3){

            initConfig.Load(configPath);

            initConfig.Get("logPath", logPath);

            logPath = logPath + logDirName;

            if(!ManageFiles::createDirectory( logPath )){

                throw "Can't create location for log files";

            }

            Fits fitsHeader;
            fitsHeader.loadKeywordsFromConfigFile(configPath);

            // log configuration
            init_log(logPath);
            src::severity_logger< severity_level > slg;
            BOOST_LOG_SCOPED_THREAD_TAG("LogName", "mainThread");

            if( initConfig.Get("acqMaskEnable",  acqMaskEnable)&&
                initConfig.Get("acqMaskPath",    acqMaskPath)&&
                initConfig.Get("inputDevice",    inputDevice)&&
                initConfig.Get("imgAstroEnable", imgAstroEnable)&&
                initConfig.Get("detEnable",      detEnable)){

                initConfig.Get("imgCapEnable",   imgCapEnable);
                initConfig.Get("gePrevTime",     gePrevTime);

                bool detModeStatus = true;

                boost::mutex m_threadEnd;

                boost::mutex m_queueRAM;
                boost::condition_variable   c_queueRAM_Full;
                boost::condition_variable   c_queueRAM_New;

                vector<RecEvent>            queueEvToRec;
                boost::mutex                m_queueEvToRec;
                boost::condition_variable   c_queueEvToRecNew;

                // The shared buffer
                Fifo<Frame> queueRAM((gePrevTime * acqFPS), imgCapEnable, imgAstroEnable, detEnable);

                // Input device type
                Camera *inputCam = NULL;
                bool stopFlag = false;
                bool loopBroken = false;

                ///******************************** Load Mask ***********************************\\

                Mat mask;

                if(acqMaskEnable){

                    mask = imread(acqMaskPath,CV_LOAD_IMAGE_GRAYSCALE);

                }

                if((acqMaskEnable && mask.data) || (!acqMaskEnable)){

                    ///************************* Create acquisition thread **************************\\

                    if(inputDevice == "BASLER"){

                        debug = false;

                        if( initConfig.Get("inputDeviceName", inputDeviceName)&&
                            initConfig.Get("acqInitialExposure",    acqInitialExposure)&&
                            initConfig.Get("acqInitialGain",        acqInitialGain)){

                            initConfig.Get("acqFPS",                acqFPS);
                            initConfig.Get("acqFormatPix",          acqFormatPix);
                            initConfig.Get("inputIdDevice",         inputIdDevice);

                            BOOST_LOG_SEV(slg, notification) << " Basler in input ";

                            inputCam = new CameraBasler(&queueRAM,
                                                        &m_queueRAM,
                                                        &c_queueRAM_Full,
                                                        &c_queueRAM_New,
                                                        acqInitialExposure,
                                                        acqInitialGain,
                                                        mask,
                                                        acqMaskEnable);

                            inputCam->getListCameras();

                            if(!inputCam->setSelectedDevice(inputIdDevice, inputDeviceName)){

                                throw "ERROR : Connection failed to the first BASLER camera.";

                            }else{

                                BOOST_LOG_SEV(slg, fail)    << " Connection success to the first BASLER camera. ";
                                cout                        << " Connection success to the first BASLER camera. " << endl;

                                if(acqFormatPix == 12){

                                    inputCam->setCameraPixelFormat(12);

                                }else if(acqFormatPix == 8){

                                    inputCam->setCameraPixelFormat(8);

                                }else if(acqFormatPix == 16){

                                    inputCam->setCameraPixelFormat(12);

                                }else{

                                    throw "ERROR : Bad definition of acqFormatPix in the configuration file";

                                }

                                //Control the mask for the frame of camera
                              /*  if(acqMaskEnable){

                                    if(mask.rows != inputCam->getCameraHeight() || mask.cols!= inputCam->getCameraWidth()){

                                        cout << "mask.rows : " << mask.rows << endl;

                                        cout << "inputCam->getCameraHeight() : " << inputCam->getCameraHeight() << endl;

                                        cout << "mask.cols : " << mask.cols << endl;

                                        cout << "inputCam->getCameraWidth() : " << inputCam->getCameraWidth() << endl;

                                        throw "ERROR : The size of the mask doesn't match to the size of the camera's frame";

                                    }
                                }*/

                            }

                        }else{

                            throw "ERROR : Can't load inputDeviceName from configuration file";

                        }

                     }else if(inputDevice == "DMK"){

                        debug = false;

                        if(initConfig.Get("inputDeviceName", inputDeviceName)){

                            initConfig.Get("acqFormatPix", acqFormatPix);

                            cout << "DMK in input: "<< inputDeviceName <<endl;
                            BOOST_LOG_SEV(slg, notification) << "DMK in input ";

                            if(acqFormatPix == 12 || acqFormatPix == 8){

                                inputCam = new CameraDMK(   inputDeviceName,
                                                            acqFormatPix,
                                                            &queueRAM,
                                                            &m_queueRAM,
                                                            &c_queueRAM_Full,
                                                            &c_queueRAM_New,
                                                            &m_threadEnd,
                                                            &stopFlag);

                            }else{

                                throw "ERROR : Bad definition of acqFormatPix in the configuration file";
                            }

                            //Control the mask for the frame of camera
                            if(acqMaskEnable){

                                if(mask.rows != inputCam->getCameraHeight() || mask.cols!= inputCam->getCameraWidth()){

                                    cout << "mask.rows : " << mask.rows << endl;

                                    cout << "inputCam->getCameraHeight() : " << inputCam->getCameraHeight() << endl;

                                    cout << "mask.cols : " << mask.cols << endl;

                                    cout << "inputCam->getCameraWidth() : " << inputCam->getCameraWidth() << endl;

                                    throw "ERROR : The size of the mask doesn't match to the size of the camera's frame";

                                }
                            }

                        }else{

                            throw "ERROR : Can't load inputDeviceName from configuration file";

                        }

                    }else if(inputDevice == "SIMU"){

                        cout << "SIMU in input"<<endl;
                        BOOST_LOG_SEV(slg, notification) << "SIMU in input ";
                        inputCam = new CameraSimu(&queueRAM,&m_queueRAM, &c_queueRAM_Full,&c_queueRAM_New);

                        acqFormatPix = 8;

                    }else if(inputDevice == "VIDEO"){

                        initConfig.Get("debug", debug);

                        if(initConfig.Get("inputVideoPath", inputVideoPath)){

                            cout << "Video in input : " << inputVideoPath <<endl;
                            BOOST_LOG_SEV(slg, notification) << "Video in input : " << inputVideoPath;

                            inputCam = new CameraVideo( inputVideoPath,
                                                        &queueRAM,
                                                        &m_queueRAM,
                                                        &c_queueRAM_Full,
                                                        &c_queueRAM_New);


                            //Control the mask for the frame of camera
                            if(acqMaskEnable){

                                if(mask.rows != inputCam->getCameraHeight() && mask.cols!= inputCam->getCameraWidth()){

                                    cout << "mask.rows : " << mask.rows << endl;

                                    cout << "inputCam->getCameraHeight() : " << inputCam->getCameraHeight() << endl;

                                    cout << "mask.cols : " << mask.cols << endl;

                                    cout << "inputCam->getCameraWidth() : " << inputCam->getCameraWidth() << endl;

                                    throw "ERROR : The size of the mask doesn't match to the size of the video's frame";

                                }
                            }

                        }else{

                            throw "ERROR : Can't find the path defined in inputVideoPath in the configuration file";

                        }

                    }else if(inputDevice == "FRAME"){

                        initConfig.Get("debug", debug);

                        if( initConfig.Get("acqFramesDirectory", acqFramesDirectory)&&
                            initConfig.Get("acqFrameNumStart", acqFrameNumStart)&&
                            initConfig.Get("acqFrameNumStop", acqFrameNumStop)){

                            inputCam = new CameraFrames( acqFramesDirectory,
                                                         acqFrameNumStart,
                                                         acqFrameNumStop,
                                                        &queueRAM,
                                                        &m_queueRAM,
                                                        &c_queueRAM_Full,
                                                        &c_queueRAM_New,
                                                        fitsHeader);

                            //Control the mask for the frame of camera
                            /*if(acqMaskEnable){

                                if(mask.rows != inputCam->getCameraHeight() && mask.cols!= inputCam->getCameraWidth()){

                                    throw "The size of the mask doesn't match to the size of the frame";

                                }
                            }*/


                        }else{

                            if(!initConfig.Get("acqFramesDirectory", acqFramesDirectory))
                                throw "Can't find the path defined in acqFramesDirectory in the configuration file";

                            if(!initConfig.Get("acqFrameNumStart", acqFrameNumStart))
                                throw "Can't load acqFrameNumStart from the configuration file";

                            if(!initConfig.Get("acqFrameNumStop", acqFrameNumStop))
                                throw "Can't load acqFrameNumStop from the configuration file";

                        }


                    }else{

                        throw "Bad definition of inputDevice parameters in the configuration file. Possibilities : BASLER, VIDEO, FRAME";

                    }

                    if( inputCam!=NULL ){

                        BOOST_LOG_SEV(slg, notification) << "Program starting.";

                        // start acquisition thread
                        inputCam->startThread();

                        DetThread *det  = NULL;
                        RecThread *rec  = NULL;
                        AstThread *ast  = NULL;

                        initConfig.Get("dataRecordPath", dataRecordPath);

                        ///************************* Create astrometry thread **************************\\

                        if(imgAstroEnable){

                            if( initConfig.Get("imgAstroExpTime",    imgAstroExpTime)&&
                                initConfig.Get("imgAstroInterval",      imgAstroInterval)&&
                                initConfig.Get("imgAstroMethod",        imgAstroMethod)&&
                                initConfig.Get("SITELONG",              longitude)){

                                initConfig.Get("stationName", stationName);

                                ast = new AstThread(dataRecordPath,
                                                    stationName,
                                                    imgAstroMethod,
                                                    configPath,
                                                    &queueRAM,
                                                    imgAstroInterval,
                                                    imgAstroExpTime,
                                                    acqFormatPix,
                                                    longitude,
                                                    &m_queueRAM,
                                                    &c_queueRAM_Full,
                                                    &c_queueRAM_New,
                                                    fitsHeader);

                                ast->startCapture();

                            }else{

                                if(!initConfig.Get("imgAstroExpTime", imgAstroExpTime))
                                    throw "Can't load imgAstroExpTime from the configuration file";

                                if(!initConfig.Get("imgAstroInterval", imgAstroInterval))
                                    throw "Can't load imgAstroInterval from the configuration file";

                                if(!initConfig.Get("imgAstroMethod", imgAstroMethod))
                                    throw "Can't load imgAstroMethod from the configuration file";

                                if(!initConfig.Get("SITELONG", longitude))
                                    throw "Can't load SITELONG from the configuration file";

                            }
                        }

                        ///************************* Create detection thread **************************\\

                        if(detEnable){

                            if( initConfig.Get("detMethod",             detMethod)){

                                initConfig.Get("detRecFits2D",          detRecFits2D);
                                initConfig.Get("detRecPositions",       detRecPositions);
                                initConfig.Get("detRecFits3D",          detRecFits3D);
                                initConfig.Get("detRecBmp",             detRecBmp);
                                initConfig.Get("detRecAvi",             detRecAvi);
                                initConfig.Get("detRecMapGE",           detRecMapGE);
                                initConfig.Get("detRecShape",           detRecShape);
                                initConfig.Get("detRecTrail",           detRecTrail);
                                initConfig.Get("geAfterTime",           geAfterTime);
                                initConfig.Get("geMax",                 geMax);
                                initConfig.Get("geMaxTime",             geMaxTime);
                                initConfig.Get("stationName",           stationName);
                                initConfig.Get("detMaskMoon",           detMaskMoon);
                                initConfig.Get("detDownsample",         detDownsample);
                                initConfig.Get("debugPath",             debugPath);
                                initConfig.Get("saveMaskedMoon",        saveMaskedMoon);

                                det  = new DetThread(   mask,
                                                        1,
                                                        acqFormatPix,
                                                        &queueRAM,
                                                        &m_queueRAM,
                                                        &c_queueRAM_Full,
                                                        &c_queueRAM_New,
                                                        &c_queueEvToRecNew,
                                                        &m_queueEvToRec,
                                                        &queueEvToRec,
                                                        geAfterTime,
                                                        geMax,
                                                        geMaxTime,
                                                        dataRecordPath,
                                                        stationName,
                                                        debug,
                                                        debugPath,
                                                        detMaskMoon,
                                                        saveMaskedMoon,
                                                        detDownsample,
                                                        fitsHeader);

                                det->startDetectionThread();

                                ///************************* Create record thread **************************\\

                                initConfig.Get("detRecSum", detRecSum);

                                rec = new RecThread(    dataRecordPath,
                                                        &queueEvToRec,
                                                        &m_queueEvToRec,
                                                        &c_queueEvToRecNew,
                                                        acqFormatPix,
                                                        detRecAvi,
                                                        detRecFits3D,
                                                        detRecFits2D,
                                                        detRecSum,
                                                        detRecPositions,
                                                        detRecBmp,
                                                        detRecTrail,
                                                        detRecShape,
                                                        detRecMapGE,
                                                        fitsHeader);

                                rec->start();

                            }else{

                                if(!initConfig.Get("detMethod", detMethod))
                                    throw "Can't load detMethod from the configuration file";

                            }
                        }

                        ///************************* Create additionnal thread **************************\\

                        // start image capture thread
                        ImgThread * imgCap = NULL;

                        if(imgCapEnable){

                            if( initConfig.Get("imgCapGammaCorrEnable", imgCapGammaCorrEnable)&&
                                initConfig.Get("imgCapGammaCorrValue",  imgCapGammaCorrValue)&&
                                initConfig.Get("imgCapExpTime",         imgCapExpTime)&&
                                initConfig.Get("imgCapInterval",        imgCapInterval)){

                                imgCap = new ImgThread( dataRecordPath,
                                                        imgCapInterval,
                                                        imgCapExpTime,
                                                        imgCapGammaCorrEnable,
                                                        imgCapGammaCorrValue,
                                                        &queueRAM,
                                                        acqFormatPix,
                                                        &m_queueRAM,
                                                        &c_queueRAM_Full,
                                                        &c_queueRAM_New);

                                imgCap->startCapture();

                            }
                        }

                        // main thread loop
                        BOOST_LOG_SEV(slg, notification) << "FreeTure is working...";
                        cout << "FreeTure is working..."<<endl;

                        if(inputDevice == "BASLER" || inputDevice == "DMK"){

                            initConfig.Get("saveConfigFileCopy", saveConfigFileCopy);
                            initConfig.Get("stationName",           stationName);




                            #ifdef _WIN64_
                                BOOST_LOG_SEV(slg, notification) << "This is the process : " << (unsigned long)_getpid();
                            #elif defined _LINUX_
                                BOOST_LOG_SEV(slg, notification) << "This is the process : " << (unsigned long)getpid();

                                memset(&act, 0, sizeof(act));
                                act.sa_sigaction = sigTermHandler;
                                act.sa_flags = SA_SIGINFO;
                                sigaction(SIGTERM,&act,NULL);

                            #endif

                            int cptTime = 0;

                            while(!sigTermFlag){

                                if(saveConfigFileCopy){

                                    namespace fs = boost::filesystem;

                                    string dateNow = TimeDate::localDateTime(second_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");
                                    vector<string> dateString;

                                    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                                    boost::char_separator<char> sep(":");
                                    tokenizer tokens(dateNow, sep);

                                    for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
                                        dateString.push_back(*tok_iter);
                                    }

                                    string root = dataRecordPath + stationName + "_" + dateString.at(0) + dateString.at(1) + dateString.at(2) +"/";

                                    string cFile = root + "configuration.cfg";

                                    cout << cFile << endl;

                                    path p(dataRecordPath);

                                    path p1(root);

                                    path p2(cFile);

                                    if(fs::exists(p)){

                                        if(fs::exists(p1)){

                                            BOOST_LOG_SEV(slg,notification) << "Destination directory " << p1.string() << " already exists.";

                                            if(!fs::exists(p2)){

                                                path p3(configPath);

                                                if(fs::exists(p3)){

                                                    fs::copy_file(p3,p2,copy_option::overwrite_if_exists);

                                                }else{

                                                    //cout << "Failed to copy configuration file : " << p3.string() << " not exists." << endl;
                                                    BOOST_LOG_SEV(slg,notification) << "Failed to copy configuration file : " << p3.string() << " not exists.";

                                                }

                                            }

                                        }else{

                                            if(!fs::create_directory(p1)){

                                                BOOST_LOG_SEV(slg,notification) << "Unable to create destination directory" << p1.string();

                                            }else{

                                                path p3(configPath);

                                                if(fs::exists(p3)){

                                                    fs::copy_file(p3,p2,copy_option::overwrite_if_exists);

                                                }else{

                                                    cout << "Failed to copy configuration file : " << p3.string() << " not exists." << endl;
                                                    BOOST_LOG_SEV(slg,notification) << "Failed to copy configuration file : " << p3.string() << " not exists.";

                                                }
                                            }
                                        }

                                    }else{

                                        if(!fs::create_directory(p)){

                                            BOOST_LOG_SEV(slg,notification) << "Unable to create destination directory" << p.string();

                                        }else{

                                            if(!fs::create_directory(p1)){

                                                BOOST_LOG_SEV(slg,notification) << "Unable to create destination directory" << p1.string();

                                            }else{

                                                path p3(configPath);

                                                if(fs::exists(p3)){

                                                    fs::copy_file(p3,p2,copy_option::overwrite_if_exists);

                                                }else{

                                                    cout << "Failed to copy configuration file : " << p3.string() << " not exists." << endl;
                                                    BOOST_LOG_SEV(slg,notification) << "Failed to copy configuration file : " << p3.string() << " not exists.";

                                                }
                                            }
                                        }
                                    }
                                }

                                #ifdef _WIN64_
                                    Sleep(1000);
                                #elif defined _LINUX_
                                    sleep(1);
                                #endif

                                //	BOOST_LOG_SEV(slg, notification) << "Sleep 1 second";

                                if(executionTime != 0){
                                    if(cptTime > executionTime){
                                        BOOST_LOG_SEV(slg, notification) << "Break main loop";
                                        loopBroken = true;
                                        break;
                                    }
                                    cptTime ++;

                                    //cout << cptTime << " SEC" << endl;
                                }

                            }

                        }else{
                         //   cout << "ONE SEC" << endl;
                            inputCam->join();
                           // sleep(3000);
                        }

                        cout << "\nFreeTure terminated"<<endl;

                        BOOST_LOG_SEV(slg, notification) << "FreeTure terminated";

                        if(det != NULL){
                            cout << "Send signal to stop detection thread" << endl;
                            BOOST_LOG_SEV(slg, notification) << "Send signal to stop detection thread.";
                            det->stopDetectionThread();
                            BOOST_LOG_SEV(slg, notification) << "detection thread stopped";
                            BOOST_LOG_SEV(slg, notification) << "delete thread.";
                            delete det;

                            if(rec != NULL){
                                BOOST_LOG_SEV(slg, notification) << "Send signal to stop rec thread.";
                                rec->stop();
                                delete rec;
                            }
                        }

                        if(imgCap!=NULL){

                            BOOST_LOG_SEV(slg, notification) << "Send signal to stop image capture thread.";
                            imgCap->stopCapture();
                            delete imgCap;

                        }

                        if(ast!=NULL){
                            cout << "Send signal to stop image capture thread for astrometry." << endl;
                            BOOST_LOG_SEV(slg, notification) << "Send signal to stop image capture thread for astrometry.";
                            ast->stopCapture();
                            delete ast;

                        }

                        //stop acquisition thread
                        BOOST_LOG_SEV(slg, notification) << "Send signal to stop acquisition thread.";
                        inputCam->stopThread();
                        //inputCam->~Camera();
                        if (inputCam!=NULL)
                            delete inputCam;

                    }

                }else{

                    if(!mask.data)
                        throw "Can't load the mask defined in acqMaskPath (see configuration file)";

                }

            }else{

                if(!initConfig.Get("acqMaskEnable", acqMaskEnable))
                    throw "acqMaskEnable not defined in the configuration file";

                if(!initConfig.Get("acqMaskPath", acqMaskPath))
                    throw "acqMaskPath not defined in the configuration file";

                if(!initConfig.Get("inputDevice", inputDevice))
                    throw "inputDevice not defined in the configuration file";

                if(!initConfig.Get("imgAstroEnable", imgAstroEnable))
                    throw "imgAstroEnable not defined in the configuration file";

                if(!initConfig.Get("detEnable", detEnable))
                    throw "detEnable not defined in the configuration file";

            }

        /***************************************************************
        ******************** MODE 4 : RUN ACQ TEST MODE ****************
        ****************************************************************/

        //capture of a single image
        }else if(mode == 4){

            if(vm.count("savepath"))    savePath        = vm["savepath"].as<string>();

            if(vm.count("device")){

                  device = vm["device"].as<string>();

            }else{

                throw "Please define the name of the camera device to capture a single frame";

            }

            if(vm.count("format"))      acqFormat       = vm["format"].as<int>();

            if(vm.count("bmp"))         saveBmp         = vm["bmp"].as<bool>();

            if(vm.count("fits2D"))      saveFits2D      = vm["fits2D"].as<bool>();

            if(vm.count("gain")){

                gain = vm["gain"].as<int>();

            }else{

                throw "Please define the gain value";

            }

            if(vm.count("exposure")){

               exp = vm["exposure"].as<int>();

            }else{

                throw "Please define the exposure time value";

            }

            initConfig.Load(configPath);

            initConfig.Get("logPath", logPath);

            logPath = logPath + logDirName;

            if(!ManageFiles::createDirectory( logPath )){

                throw "Can't create location for log files";

            }

            // log configuration
            init_log(logPath);
            src::severity_logger< severity_level > slg;
            BOOST_LOG_SCOPED_THREAD_TAG("LogName", "mainThread");

            Fits fitsHeader;
            fitsHeader.loadKeywordsFromConfigFile(configPath);

            cout << "FORMAT :        " << acqFormat     << endl;
            cout << "EXPOSURE TIME : " << exp           << endl;
            cout << "GAIN :          " << gain          << endl;
            cout << "SAVE FITS2D :   " << saveFits2D    << endl;
            cout << "SAVE BMP :      " << saveBmp       << endl;
            cout << "SAVE LOCATION : " << savePath      << endl;

            if(acqFormat==16){

                acqFormat = 12;

            }else if(acqFormat!=8 && acqFormat!=16 && acqFormat!=12){

                acqFormat = 8;

            }

            Camera *inputCam = NULL;

            inputCam = new CameraBasler(exp,
                                        gain,
                                        saveFits2D,
                                        saveBmp,
                                        acqFormat,
                                        configPath,
                                        savePath,
                                        fitsHeader);

            //list connected basler cameras
            inputCam->getListCameras();

            cout<< "Try to connect to the first in the list..."<<endl;

            if(!inputCam->setSelectedDevice(0, device)){

                throw "Connection to the camera failed";

            }else{

                cout <<"Connection success"<<endl;

                inputCam->setCameraPixelFormat(acqFormat);

                inputCam->startGrab();

                inputCam->grabOne();

            }

            BOOST_LOG_SEV(slg, notification) << "End single capture";

        }else{

            cout << "Please choose an existing mode (example for choose first mode : -m 1 )" << endl << endl;
            cout << "Available modes are :"                              << endl;
            cout << "-> 1 : list connected devices"                      << endl;
            cout << "-> 2 : print configuration file"                    << endl;
            cout << "-> 3 : run meteor detection"                        << endl;
            cout << "-> 4 : test a camera by single capture"             << endl;

        }

    }catch(exception& e){

        cout << "An exception occured : " << e.what() << endl;

    }catch(const char * msg){

        cout << msg << endl;

    }

    po::notify(vm);

	return 0 ;
}

