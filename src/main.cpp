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
#include "Histogram.h"
#include "EnumBitdepth.h"

#include <boost/filesystem.hpp>

#include "SMTPClient.h"

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
      ("config,c",      po::value<string>()->default_value(/*"./configuration.cfg"*/string(CFG_PATH) + "configuration.cfg"), "Define configuration file's path")
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
        string configPath   = /*"./configuration.cfg";*/string(CFG_PATH) + "configuration.cfg";
        string savePath     = "./";
        string device       = "";
        int acqFormat       = 8;
        bool saveBmp        = false;
        bool saveFits2D     = false;
        int gain            = 300;
        int exp             = 100;
        string version      =  /*"1";//;*/string(PACKAGE_VERSION);  //

        po::store(po::parse_command_line(argc, argv, desc), vm);

        if(vm.count("mode"))        mode            = vm["mode"].as<int>();

        if(vm.count("exectime"))    executionTime   = vm["exectime"].as<int>();

        if(vm.count("config"))      configPath      = vm["config"].as<string>();

        // load configuration file and parameters
        Configuration initConfig;

        string	INPUT;
        string  CAMERA_NAME;
        string	VIDEO_PATH;
        string	DET_METHOD;
        string  ACQ_MASK_PATH;
        string  STACK_MTHD;
        string  DATA_PATH               = "./data/";
        string  STATION_NAME            = "STATION";
        string  FRAMES_PATH;
        string  LOG_PATH                = "/tmp/";
        string  logDirName              = "logFiles/";
        string  DEBUG_PATH              = "./";
        int     FRAMES_START;
        int     FRAMES_STOP;
        int		CAMERA_ID               = 0;
        int		ACQ_BIT_DEPTH               = 8;
        int		ACQ_FPS                     = 30;
        int		imgCapInterval;
        int		STACK_INTERVAL;
        int     DET_TIME_BEFORE         = 1;
        int     DET_TIME_AFTER          = 1;
        int     DET_GE_MAX              = 10;
        int     DET_TIME_MAX            = 10;
        bool	DET_ENABLED;
        bool    DET_SAVE_FITS3D         = false;
        bool    DET_SAVE_FITS2D         = false;
        bool    DET_SAVE_POS            = true;
        bool    DET_SAVE_BMP            = false;
        bool    DET_SAVE_AVI            = false;
        bool    DET_SAVE_GEMAP          = true;
        bool    DEBUG_ENABLED           = false;
        bool    DET_SAVE_TRAIL          = false;
        bool    DET_DOWNSAMPLE_ENABLED  = false;
        bool    ACQ_MASK_ENABLED            = false;
        bool	imgCapEnable            = false;
        bool    DET_SAVE_SUM            = false;
        bool	imgCapGammaCorrEnable;
        bool	STACK_ENABLED;
        double	imgCapExpTime;
        double	imgCapGammaCorrValue;
        double	STACK_TIME;
        double  longitude               = 0.0;
        int     ACQ_EXPOSURE	            = 0;
        int     ACQ_GAIN		            = 0;
        bool    COPY_CONFIGFILE_ENABLED = false;

        /****************************************************************
        *************** MODE 1 : LIST CONNECTED CAMERAS *****************
        ****************************************************************/

        if(vm.count("version")){

            cout << "Current version : " << version << endl;

        }else if(vm.count("help")){

            cout << desc;

        }else if(mode == 1){

            initConfig.Load(configPath);

            initConfig.Get("LOG_PATH", LOG_PATH);

            LOG_PATH = LOG_PATH + logDirName;

            if(!ManageFiles::createDirectory( LOG_PATH )){

                throw "Can't create location for log files";

            }

            // log configuration
            init_log(LOG_PATH);
            src::severity_logger< severity_level > slg;
            BOOST_LOG_SCOPED_THREAD_TAG("LogName", "mainThread");

            initConfig.Get("INPUT", INPUT);

            if ( INPUT == "BASLER" ){

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

            cout<<"=> INPUT               : "<<INPUT            <<endl;
            cout<<"=> CAMERA_NAME         : "<<CAMERA_NAME        <<endl;
            cout<<"=> CAMERA_ID           : "<<CAMERA_ID          <<endl;
            cout<<"=> VIDEO_PATH          : "<<VIDEO_PATH         <<endl;
            cout<<"=> ACQ_BIT_DEPTH			  : "<<ACQ_BIT_DEPTH           <<endl;
            cout<<"=> DET_ENABLED         : "<<DET_ENABLED              <<endl;
            cout<<"=> DET_METHOD          : "<<DET_METHOD              <<endl;
            cout<<"=> DET_SAVE_FITS3D          : "<<DET_SAVE_FITS3D           <<endl;
            cout<<"=> DET_SAVE_FITS2D              : "<<DET_SAVE_FITS2D           <<endl;
            cout<<"=> DET_SAVE_BMP                 : "<<DET_SAVE_BMP              <<endl;
            cout<<"=> DET_SAVE_AVI                 : "<<DET_SAVE_AVI              <<endl;
            cout<<"=> DET_SAVE_GEMAP               : "<<DET_SAVE_GEMAP            <<endl;
            cout<<"=> DET_SAVE_TRAIL               : "<<DET_SAVE_TRAIL            <<endl;
            cout<<"=> ACQ_MASK_PATH               : "<<ACQ_MASK_PATH            <<endl;
            cout<<"=> imgCapEnable              : "<<imgCapEnable           <<endl;
            cout<<"=> imgCapGammaCorrEnable     : "<<imgCapGammaCorrEnable  <<endl;
            cout<<"=> imgCapGammaCorrValue      : "<<imgCapGammaCorrValue   <<endl;
            cout<<"=> imgCapExpTime             : "<<imgCapExpTime          <<endl;
            cout<<"=> imgCapInterval            : "<<imgCapInterval         <<endl;
            cout<<"=> STACK_ENABLED            : "<<STACK_ENABLED         <<endl;
            cout<<"=> STACK_TIME           : "<<STACK_TIME        <<endl;
            cout<<"=> STACK_INTERVAL          : "<<STACK_INTERVAL       <<endl;
            cout<<"=> STACK_MTHD            : "<<STACK_MTHD         <<endl;

        /***************************************************************
        ****************** MODE 3 : RUN DETECTION MODE *****************
        ****************************************************************/

        }else if(mode == 3){

            initConfig.Load(configPath);

            initConfig.Get("LOG_PATH", LOG_PATH);

            LOG_PATH = LOG_PATH + logDirName;

            if(!ManageFiles::createDirectory( LOG_PATH )){

                throw "Can't create location for log files";

            }

            Fits fitsHeader;
            fitsHeader.loadKeywordsFromConfigFile(configPath);


            // log configuration
            init_log(LOG_PATH);
            src::severity_logger< severity_level > slg;
            BOOST_LOG_SCOPED_THREAD_TAG("LogName", "mainThread");

            if( initConfig.Get("ACQ_MASK_ENABLED",  ACQ_MASK_ENABLED)&&
                initConfig.Get("ACQ_MASK_PATH",    ACQ_MASK_PATH)&&
                initConfig.Get("INPUT",    INPUT)&&
                initConfig.Get("STACK_ENABLED", STACK_ENABLED)&&
                initConfig.Get("DET_ENABLED",      DET_ENABLED)){

                initConfig.Get("imgCapEnable",   imgCapEnable);
                initConfig.Get("DET_TIME_BEFORE",     DET_TIME_BEFORE);

                bool detModeStatus = true;

                boost::mutex m_threadEnd;

                boost::mutex m_queueRAM;
                boost::condition_variable   c_queueRAM_Full;
                boost::condition_variable   c_queueRAM_New;

                vector<RecEvent>            queueEvToRec;
                boost::mutex                m_queueEvToRec;
                boost::condition_variable   c_queueEvToRecNew;

                // The shared buffer
                Fifo<Frame> queueRAM((DET_TIME_BEFORE * ACQ_FPS), imgCapEnable, STACK_ENABLED, DET_ENABLED);

                // Input device type
                Camera *inputCam = NULL;
                bool stopFlag = false;
                bool loopBroken = false;

                ///******************************** Load Mask ***********************************\\

                Mat mask;

                if(ACQ_MASK_ENABLED){

                    mask = imread(ACQ_MASK_PATH,CV_LOAD_IMAGE_GRAYSCALE);

                }

                if((ACQ_MASK_ENABLED && mask.data) || (!ACQ_MASK_ENABLED)){

                    ///************************* Create acquisition thread **************************\\

                    if(INPUT == "BASLER"){

                        DEBUG_ENABLED = false;

                        if( initConfig.Get("CAMERA_NAME", CAMERA_NAME)&&
                            initConfig.Get("ACQ_EXPOSURE", ACQ_EXPOSURE)&&
                            initConfig.Get("ACQ_GAIN", ACQ_GAIN)){

                            initConfig.Get("ACQ_FPS", ACQ_FPS);
                            initConfig.Get("ACQ_BIT_DEPTH", ACQ_BIT_DEPTH);
                            initConfig.Get("CAMERA_ID", CAMERA_ID);

                            BOOST_LOG_SEV(slg, notification) << " Basler in input ";

                            inputCam = new CameraBasler(&queueRAM,
                                                        &m_queueRAM,
                                                        &c_queueRAM_Full,
                                                        &c_queueRAM_New,
                                                        ACQ_EXPOSURE,
                                                        ACQ_GAIN,
                                                        mask,
                                                        ACQ_MASK_ENABLED);

                            inputCam->getListCameras();

                            if(!inputCam->setSelectedDevice(CAMERA_ID, CAMERA_NAME)){

                                throw "ERROR : Connection failed to the first BASLER camera.";

                            }else{

                                BOOST_LOG_SEV(slg, fail)    << " Connection success to the first BASLER camera. ";
                                cout                        << " Connection success to the first BASLER camera. " << endl;

                                if(ACQ_BIT_DEPTH == 12){

                                    inputCam->setCameraPixelFormat(12);

                                }else if(ACQ_BIT_DEPTH == 8){

                                    inputCam->setCameraPixelFormat(8);

                                }else if(ACQ_BIT_DEPTH == 16){

                                    inputCam->setCameraPixelFormat(12);

                                }else{

                                    throw "ERROR : Bad definition of ACQ_BIT_DEPTH in the configuration file";

                                }

                                //Control the mask for the frame of camera
                              /*  if(ACQ_MASK_ENABLED){

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

                            throw "ERROR : Can't load CAMERA_NAME from configuration file";

                        }

                     }else if(INPUT == "DMK"){

                        DEBUG_ENABLED = false;

                        if(initConfig.Get("CAMERA_NAME", CAMERA_NAME)){

                            initConfig.Get("ACQ_BIT_DEPTH", ACQ_BIT_DEPTH);

                            cout << "DMK in input: "<< CAMERA_NAME <<endl;
                            BOOST_LOG_SEV(slg, notification) << "DMK in input ";

                            if(ACQ_BIT_DEPTH == 12 || ACQ_BIT_DEPTH == 8){

                                inputCam = new CameraDMK(   CAMERA_NAME,
                                                            ACQ_BIT_DEPTH,
                                                            &queueRAM,
                                                            &m_queueRAM,
                                                            &c_queueRAM_Full,
                                                            &c_queueRAM_New,
                                                            &m_threadEnd,
                                                            &stopFlag);

                            }else{

                                throw "ERROR : Bad definition of ACQ_BIT_DEPTH in the configuration file";
                            }

                            //Control the mask for the frame of camera
                            if(ACQ_MASK_ENABLED){

                                if(mask.rows != inputCam->getCameraHeight() || mask.cols!= inputCam->getCameraWidth()){

                                    cout << "mask.rows : " << mask.rows << endl;

                                    cout << "inputCam->getCameraHeight() : " << inputCam->getCameraHeight() << endl;

                                    cout << "mask.cols : " << mask.cols << endl;

                                    cout << "inputCam->getCameraWidth() : " << inputCam->getCameraWidth() << endl;

                                    throw "ERROR : The size of the mask doesn't match to the size of the camera's frame";

                                }
                            }

                        }else{

                            throw "ERROR : Can't load CAMERA_NAME from configuration file";

                        }

                    }else if(INPUT == "SIMU"){

                        cout << "SIMU in input"<<endl;
                        BOOST_LOG_SEV(slg, notification) << "SIMU in input ";
                        inputCam = new CameraSimu(&queueRAM,&m_queueRAM, &c_queueRAM_Full,&c_queueRAM_New);

                        ACQ_BIT_DEPTH = 8;

                    }else if(INPUT == "VIDEO"){

                        initConfig.Get("DEBUG_ENABLED", DEBUG_ENABLED);

                        if(initConfig.Get("VIDEO_PATH", VIDEO_PATH)){

                            cout << "Video in input : " << VIDEO_PATH <<endl;
                            BOOST_LOG_SEV(slg, notification) << "Video in input : " << VIDEO_PATH;

                            inputCam = new CameraVideo( VIDEO_PATH,
                                                        &queueRAM,
                                                        &m_queueRAM,
                                                        &c_queueRAM_Full,
                                                        &c_queueRAM_New);


                            //Control the mask for the frame of camera
                            if(ACQ_MASK_ENABLED){

                                if(mask.rows != inputCam->getCameraHeight() && mask.cols!= inputCam->getCameraWidth()){

                                    cout << "mask.rows : " << mask.rows << endl;

                                    cout << "inputCam->getCameraHeight() : " << inputCam->getCameraHeight() << endl;

                                    cout << "mask.cols : " << mask.cols << endl;

                                    cout << "inputCam->getCameraWidth() : " << inputCam->getCameraWidth() << endl;

                                    throw "ERROR : The size of the mask doesn't match to the size of the video's frame";

                                }
                            }

                        }else{

                            throw "ERROR : Can't find the path defined in VIDEO_PATH in the configuration file";

                        }

                    }else if(INPUT == "FRAME"){

                        initConfig.Get("DEBUG_ENABLED", DEBUG_ENABLED);

                        if( initConfig.Get("FRAMES_PATH", FRAMES_PATH)&&
                            initConfig.Get("FRAMES_START", FRAMES_START)&&
                            initConfig.Get("FRAMES_STOP", FRAMES_STOP)){

                            inputCam = new CameraFrames( FRAMES_PATH,
                                                         FRAMES_START,
                                                         FRAMES_STOP,
                                                        &queueRAM,
                                                        &m_queueRAM,
                                                        &c_queueRAM_Full,
                                                        &c_queueRAM_New,
                                                        fitsHeader);

                            //Control the mask for the frame of camera
                            /*if(ACQ_MASK_ENABLED){

                                if(mask.rows != inputCam->getCameraHeight() && mask.cols!= inputCam->getCameraWidth()){

                                    throw "The size of the mask doesn't match to the size of the frame";

                                }
                            }*/


                        }else{

                            if(!initConfig.Get("FRAMES_PATH", FRAMES_PATH))
                                throw "Can't find the path defined in FRAMES_PATH in the configuration file";

                            if(!initConfig.Get("FRAMES_START", FRAMES_START))
                                throw "Can't load FRAMES_START from the configuration file";

                            if(!initConfig.Get("FRAMES_STOP", FRAMES_STOP))
                                throw "Can't load FRAMES_STOP from the configuration file";

                        }


                    }else{

                        throw "Bad definition of INPUT parameters in the configuration file. Possibilities : BASLER, VIDEO, FRAME";

                    }

                    if( inputCam!=NULL ){

                        BOOST_LOG_SEV(slg, notification) << "Program starting.";

                        // start acquisition thread
                        inputCam->startThread();

                        DetThread *det  = NULL;
                        RecThread *rec  = NULL;
                        AstThread *ast  = NULL;

                        initConfig.Get("DATA_PATH", DATA_PATH);

                        ///************************* Create astrometry thread **************************\\

                        if(STACK_ENABLED){

                            if( initConfig.Get("STACK_TIME",    STACK_TIME)&&
                                initConfig.Get("STACK_INTERVAL",      STACK_INTERVAL)&&
                                initConfig.Get("STACK_MTHD",        STACK_MTHD)&&
                                initConfig.Get("SITELONG",              longitude)){

                                initConfig.Get("STATION_NAME", STATION_NAME);

                                ast = new AstThread(DATA_PATH,
                                                    STATION_NAME,
                                                    STACK_MTHD,
                                                    configPath,
                                                    &queueRAM,
                                                    STACK_INTERVAL,
                                                    STACK_TIME,
                                                    ACQ_BIT_DEPTH,
                                                    longitude,
                                                    &m_queueRAM,
                                                    &c_queueRAM_Full,
                                                    &c_queueRAM_New,
                                                    fitsHeader);

                                ast->startCapture();

                            }else{

                                if(!initConfig.Get("STACK_TIME", STACK_TIME))
                                    throw "Can't load STACK_TIME from the configuration file";

                                if(!initConfig.Get("STACK_INTERVAL", STACK_INTERVAL))
                                    throw "Can't load STACK_INTERVAL from the configuration file";

                                if(!initConfig.Get("STACK_MTHD", STACK_MTHD))
                                    throw "Can't load STACK_MTHD from the configuration file";

                                if(!initConfig.Get("SITELONG", longitude))
                                    throw "Can't load SITELONG from the configuration file";

                            }
                        }

                        ///************************* Create detection thread **************************\\

                        if(DET_ENABLED){

                            if( initConfig.Get("DET_METHOD",             DET_METHOD)){

                                initConfig.Get("DET_SAVE_FITS2D",          DET_SAVE_FITS2D);
                                initConfig.Get("DET_SAVE_POS",       DET_SAVE_POS);
                                initConfig.Get("DET_SAVE_FITS3D",          DET_SAVE_FITS3D);
                                initConfig.Get("DET_SAVE_BMP",             DET_SAVE_BMP);
                                initConfig.Get("DET_SAVE_AVI",             DET_SAVE_AVI);
                                initConfig.Get("DET_SAVE_GEMAP",           DET_SAVE_GEMAP);

                                initConfig.Get("DET_SAVE_TRAIL",           DET_SAVE_TRAIL);
                                initConfig.Get("DET_TIME_AFTER",           DET_TIME_AFTER);
                                initConfig.Get("DET_GE_MAX",                 DET_GE_MAX);
                                initConfig.Get("DET_TIME_MAX",             DET_TIME_MAX);
                                initConfig.Get("STATION_NAME",           STATION_NAME);
                                initConfig.Get("DET_DOWNSAMPLE_ENABLED",         DET_DOWNSAMPLE_ENABLED);
                                initConfig.Get("DEBUG_PATH",             DEBUG_PATH);

                                det  = new DetThread(   mask,
                                                        1,
                                                        ACQ_BIT_DEPTH,
                                                        &queueRAM,
                                                        &m_queueRAM,
                                                        &c_queueRAM_Full,
                                                        &c_queueRAM_New,
                                                        &c_queueEvToRecNew,
                                                        &m_queueEvToRec,
                                                        &queueEvToRec,
                                                        DET_TIME_AFTER,
                                                        DET_GE_MAX,
                                                        DET_TIME_MAX,
                                                        DATA_PATH,
                                                        STATION_NAME,
                                                        DEBUG_ENABLED,
                                                        DEBUG_PATH,
                                                        DET_DOWNSAMPLE_ENABLED,
                                                        fitsHeader);

                                det->startDetectionThread();

                                ///************************* Create record thread **************************\\

                                initConfig.Get("DET_SAVE_SUM", DET_SAVE_SUM);

                                rec = new RecThread(    DATA_PATH,
                                                        &queueEvToRec,
                                                        &m_queueEvToRec,
                                                        &c_queueEvToRecNew,
                                                        ACQ_BIT_DEPTH,
                                                        DET_SAVE_AVI,
                                                        DET_SAVE_FITS3D,
                                                        DET_SAVE_FITS2D,
                                                        DET_SAVE_SUM,
                                                        DET_SAVE_POS,
                                                        DET_SAVE_BMP,
                                                        DET_SAVE_TRAIL,
                                                        DET_SAVE_GEMAP,
                                                        fitsHeader);

                                rec->start();

                            }else{

                                if(!initConfig.Get("DET_METHOD", DET_METHOD))
                                    throw "Can't load DET_METHOD from the configuration file";

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

                                imgCap = new ImgThread( DATA_PATH,
                                                        imgCapInterval,
                                                        imgCapExpTime,
                                                        imgCapGammaCorrEnable,
                                                        imgCapGammaCorrValue,
                                                        &queueRAM,
                                                        ACQ_BIT_DEPTH,
                                                        &m_queueRAM,
                                                        &c_queueRAM_Full,
                                                        &c_queueRAM_New);

                                imgCap->startCapture();

                            }
                        }

                        // main thread loop
                        BOOST_LOG_SEV(slg, notification) << "FreeTure is working...";
                        cout << "FreeTure is working..."<<endl;

                        if(INPUT == "BASLER" || INPUT == "DMK"){

                            initConfig.Get("COPY_CONFIGFILE_ENABLED", COPY_CONFIGFILE_ENABLED);
                            initConfig.Get("STATION_NAME",           STATION_NAME);




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

                                if(COPY_CONFIGFILE_ENABLED){

                                    namespace fs = boost::filesystem;

                                    string dateNow = TimeDate::localDateTime(second_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");
                                    vector<string> dateString;

                                    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                                    boost::char_separator<char> sep(":");
                                    tokenizer tokens(dateNow, sep);

                                    for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
                                        dateString.push_back(*tok_iter);
                                    }

                                    string root = DATA_PATH + STATION_NAME + "_" + dateString.at(0) + dateString.at(1) + dateString.at(2) +"/";

                                    string cFile = root + "configuration.cfg";

                                    cout << cFile << endl;

                                    path p(DATA_PATH);

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
                        throw "Can't load the mask defined in ACQ_MASK_PATH (see configuration file)";

                }

            }else{

                if(!initConfig.Get("ACQ_MASK_ENABLED", ACQ_MASK_ENABLED))
                    throw "ACQ_MASK_ENABLED not defined in the configuration file";

                if(!initConfig.Get("ACQ_MASK_PATH", ACQ_MASK_PATH))
                    throw "ACQ_MASK_PATH not defined in the configuration file";

                if(!initConfig.Get("INPUT", INPUT))
                    throw "INPUT not defined in the configuration file";

                if(!initConfig.Get("STACK_ENABLED", STACK_ENABLED))
                    throw "STACK_ENABLED not defined in the configuration file";

                if(!initConfig.Get("DET_ENABLED", DET_ENABLED))
                    throw "DET_ENABLED not defined in the configuration file";

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

            initConfig.Get("LOG_PATH", LOG_PATH);

            LOG_PATH = LOG_PATH + logDirName;

            if(!ManageFiles::createDirectory( LOG_PATH )){

                throw "Can't create location for log files";

            }

            // log configuration
            init_log(LOG_PATH);
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

        }else if(mode == 5){


            /*Mat img(960, 1280, CV_8UC3, Scalar(0,0,0));

            for(int i = 0; i<25; i++){

                for(int j = 0; j<25; j++){

                    if(i!=0)
                        img.at<Vec3b>(i,j) = Vec3b(0,0,255);

                }

            }

            img.at<Vec3b>(500,500) = Vec3b(0,0,255);
            img.at<Vec3b>(501,500) = Vec3b(0,0,255);
            img.at<Vec3b>(500,501) = Vec3b(0,0,255);
            img.at<Vec3b>(500,502) = Vec3b(0,255,255);
            img.at<Vec3b>(502,500) = Vec3b(0,255,255);
            img.at<Vec3b>(503,500) = Vec3b(0,255,255);
            img.at<Vec3b>(500,503) = Vec3b(0,0,255);
            img.at<Vec3b>(500,504) = Vec3b(0,0,255);
            img.at<Vec3b>(505,500) = Vec3b(0,0,255);

            imwrite( "/home/fripon/testImg.jpeg", img );*/


            Fits fitsHeader;
            fitsHeader.loadKeywordsFromConfigFile("/home/fripon/friponProject/friponCapture/configuration.cfg");

            Camera *inputCam = NULL;

            inputCam = new CameraBasler(200,
                                        400,
                                        false,
                                        true,
                                        8,
                                        "/home/fripon/friponProject/friponCapture/configuration.cfg",
                                        "/home/fripon/",
                                        fitsHeader);

            //list connected basler cameras
            inputCam->getListCameras();

            cout<< "Try to connect to the first in the list..."<<endl;

            if(!inputCam->setSelectedDevice(0, "Basler-21418131")){

                throw "Connection to the camera failed";

            }else{

                cout <<"Connection success"<<endl;

                inputCam->setCameraPixelFormat(acqFormat);

                inputCam->startGrab();

                inputCam->grabOne();

            }



            vector<string> to;
            to.push_back("yoan.audureau@gmail.com");
            to.push_back("yoan.audureau@yahoo.fr");

            vector<string> pathAttachments;
            pathAttachments.push_back("/home/fripon/capture.bmp");

            //pathAttachments.push_back("D:/logoFripon.png");

            string acquisitionDate = TimeDate::localDateTime(second_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");

            SMTPClient mailc("smtp.u-psud.fr", 25, "u-psud.fr");
            mailc.send("yoan.audureau@u-psud.fr", to, "station ORSAY "+acquisitionDate+" UT", "Test d'acquisition d'une frame sur la station d'Orsay à " + acquisitionDate, pathAttachments, true);



        }else if(mode == 6){


            namespace fs = boost::filesystem;

            path p("/home/fripon/Orsay_20141122_191438UT-0.fit");


            if(is_regular_file(p)){

                Fits fitsHeader;
                fitsHeader.loadKeywordsFromConfigFile("/home/fripon/friponProject/friponCapture/configuration.cfg");

               /* Mat resMat;

                Fits2D newFits("/home/fripon/Orsay_20141122_191438UT-0.fit",fitsHeader);
                newFits.readFits(resMat, "/home/fripon/Orsay_20141122_191438UT-0.fit");


                cout<< " Mat type : "<< Conversion::matTypeToString(resMat.type())<<endl;

                double minVal, maxVal;
                minMaxLoc(resMat, &minVal, &maxVal);

                cout << "max value : "<< maxVal<<endl;
                cout << "min value : "<< minVal<<endl;

                Histogram newHist(resMat, 1000000);
                imwrite("/home/fripon/hist.jpg",newHist.drawHist());

                int newMaxValue = 200000;

                Mat newMat(resMat.rows,resMat.cols, CV_16SC1, Scalar(0));


                float* ptr;
                short *ptr1;

                for(int i = 0; i < resMat.rows; i++){

                    ptr = resMat.ptr<float>(i);
                    ptr1 = newMat.ptr<short>(i);

                    for(int j = 0; j < resMat.cols; j++){

                        ptr1[j] = (ptr[j] - ((minVal*32767 - (-32767) * newMaxValue)/(32767-(-32767))))/((minVal-newMaxValue)/((-32767)-32767));

                    }
                }


                double newminVal, newmaxVal;
                minMaxLoc(newMat, &newminVal, &newmaxVal);

                cout << "max value : "<< newmaxVal<<endl;
                cout << "min value : "<< newminVal<<endl;

*/

               /* Histogram newHist2(newMat, newmaxVal);
                imwrite("/home/fripon/hist2.jpg",newHist.drawHist());
*/

                Mat newMat(960,1280,CV_16UC1, Scalar(1000));
                Fits2D newFits2("/home/fripon/testfits",fitsHeader);
                newFits2.writeFits(newMat, bit_depth_enum::US16, 0, true );



            }




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

