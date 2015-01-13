/*
								main.cpp

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
*	Last modified:		01/12/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
 * @file    main.cpp
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 0.2
 * @date    01/12/2014
 */

#include "includes.h"
#include <algorithm>
#include "ImgReduction.h"
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
#include "SMTPClient.h"
#include "FreeTure.h"
#include "ECamType.h"
#include "EnumParser.h"
#define BOOST_NO_SCOPED_ENUMS
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

void showHistogram(Mat& img)
{
	int bins = 256;             // number of bins
	int nc = img.channels();    // number of channels

	vector<Mat> hist(nc);       // histogram arrays

	// Initalize histogram arrays
	for (int i = 0; i < hist.size(); i++)
		hist[i] = Mat::zeros(1, bins, CV_32SC1);

	// Calculate the histogram of the image
	for (int i = 0; i < img.rows; i++)
	{
		for (int j = 0; j < img.cols; j++)
		{
			for (int k = 0; k < nc; k++)
			{
				uchar val = nc == 1 ? img.at<uchar>(i,j) : img.at<Vec3b>(i,j)[k];
				hist[k].at<int>(val) += 1;
			}
		}
	}

	// For each histogram arrays, obtain the maximum (peak) value
	// Needed to normalize the display later
	int hmax[3] = {0,0,0};
	for (int i = 0; i < nc; i++)
	{
		for (int j = 0; j < bins-1; j++)
			hmax[i] = hist[i].at<int>(j) > hmax[i] ? hist[i].at<int>(j) : hmax[i];
	}

	const char* wname[3] = { "blue", "green", "red" };
	Scalar colors[3] = { Scalar(255,0,0), Scalar(0,255,0), Scalar(0,0,255) };

	vector<Mat> canvas(nc);

	// Display each histogram in a canvas
	for (int i = 0; i < nc; i++)
	{
		canvas[i] = Mat::ones(125, bins, CV_8UC3);

		for (int j = 0, rows = canvas[i].rows; j < bins-1; j++)
		{
			line(
				canvas[i],
				Point(j, rows),
				Point(j, rows - (hist[i].at<int>(j) * rows/hmax[i])),
				nc == 1 ? Scalar(200,200,200) : colors[i],
				1, 8, 0
			);
		}

		imshow(nc == 1 ? "value" : wname[i], canvas[i]);
	}
}

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
      ("camtype",       po::value<string>()->default_value("basler"),                                   "Type of camera")
      ("display",       po::value<bool>()->default_value(false),                                        "In mode 4 : Display the grabbed frame")
      ("id",            po::value<int>(),                                                               "Camera ID")
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
        string  version         = string(PACKAGE_VERSION);
        string  camtype         = "basler";
        bool    display         = false;
        int     camID           = 0;

        po::store(po::parse_command_line(argc, argv, desc), vm);

        if(vm.count("mode"))
            mode = vm["mode"].as<int>();

        if(vm.count("time"))
            executionTime = vm["time"].as<int>();

        if(vm.count("config"))
            configPath = vm["config"].as<string>();

        if(vm.count("version")){

            cout << "Current version : " << version << endl;

        }else if(vm.count("help")){

            cout << desc;

        }else{

            namespace fs = boost::filesystem;

            path cPath(configPath);

            if(!fs::exists(cPath) && mode != 4){

                throw runtime_error("Configuration file not found. Check path : " + configPath);

            }

            switch(mode){

                case 1 :

                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%% MODE 1 : LIST CONNECTED CAMERAS %%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    {
                        cout << "================================================" << endl;
                        cout << "========= FREETURE - List device mode ==========" << endl;
                        cout << "================================================" << endl << endl;

                        FreeTure ft(configPath);
                        ft.loadParameters();

                        EnumParser<CamType> cam_type;
                        cout << "Type of device in configuration file : " << cam_type.getStringEnum(ft.CAMERA_TYPE) << endl;

                        switch(ft.CAMERA_TYPE){

                            case BASLER :

                                {

                                    CameraBasler *baslerCam = new CameraBasler();
                                    baslerCam->getListCameras();

                                    if(baslerCam != NULL)
                                        delete baslerCam;

                                }

                                break;

                            case DMK :

                                {

                                    CameraDMK *dmkCam = new CameraDMK();
                                    dmkCam->getListCameras();

                                    if(dmkCam != NULL)
                                        delete dmkCam;

                                }

                                break;

                        }
                    }

                    break;

                case 2 :

                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%% MODE 2 : VIEW/CHECK CONFIGURATION FILE %%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    {

                        FreeTure ft(configPath);
                        ft.loadParameters();
                        ft.printParameters();

                    }

                    break;

                case 3 :

                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%% MODE 3 : RUN METEOR DETECTION %%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    {

                        cout << "================================================" << endl;
                        cout << "======= FREETURE - Meteor detection mode =======" << endl;
                        cout << "================================================" << endl << endl;

                        /// ------------------------------------------------
                        /// Load FreeTure parameters from configuration file
                        /// ------------------------------------------------

                        FreeTure ft(configPath);
                        ft.loadParameters();

                        Fits fitsHeader;
                        fitsHeader.loadKeywordsFromConfigFile(configPath);

                        /// ------------------------------------------------
                        ///                   Manage Log
                        /// ------------------------------------------------

                        string logpath = ft.LOG_PATH;

                        if(!ManageFiles::createDirectory( logpath )){

                            throw "Can't create location for log files";

                        }

                        // log configuration
                        init_log(logpath);
                        src::severity_logger< severity_level > slg;
                        BOOST_LOG_SCOPED_THREAD_TAG("LogName", "mainThread");

                        boost::mutex                m_threadEnd;
                        boost::mutex                m_queueRAM;
                        boost::mutex                m_queueEvToRec;
                        boost::condition_variable   c_queueRAM_Full;
                        boost::condition_variable   c_queueRAM_New;
                        boost::condition_variable   c_queueEvToRecNew;
                        vector<RecEvent>            queueEvToRec;

                        // The shared buffer
                        Fifo<Frame> queueRAM;
                        if(ft.DET_TIME_BEFORE == 0 )
                            queueRAM = Fifo<Frame>(2, false, ft.STACK_ENABLED, ft.DET_ENABLED);
                        else
                            queueRAM = Fifo<Frame>((ft.DET_TIME_BEFORE * ft.ACQ_FPS), false, ft.STACK_ENABLED, ft.DET_ENABLED);

                        // Input device type
                        Camera      *inputCam   = NULL;
                        DetThread   *det        = NULL;
                        RecThread   *rec        = NULL;
                        AstThread   *ast        = NULL;

                        Mat mask;

                        bool stopFlag = false;
                        bool loopBroken = false;

                        /// ------------------------------------------------
                        ///                   Load Mask
                        /// ------------------------------------------------

                        if(ft.ACQ_MASK_ENABLED){

                            mask = imread(ft.ACQ_MASK_PATH, CV_LOAD_IMAGE_GRAYSCALE);

                        }

                        /// ------------------------------------------------
                        ///            Create acquisition thread
                        /// ------------------------------------------------

                        switch(ft.CAMERA_TYPE){

                            case BASLER :

                                BOOST_LOG_SEV(slg, notification) << " Basler in input ";

                                inputCam = new CameraBasler(&queueRAM,
                                                            &m_queueRAM,
                                                            &c_queueRAM_Full,
                                                            &c_queueRAM_New,
                                                            ft.ACQ_EXPOSURE,
                                                            ft.ACQ_GAIN,
                                                            ft.ACQ_BIT_DEPTH,
                                                            ft.ACQ_FPS
                                                            );

                                inputCam->getListCameras();

                                if(!inputCam->setSelectedDevice(ft.CAMERA_NAME)){

                                    throw runtime_error("Connection failed to the camera : " + ft.CAMERA_NAME);

                                }else{

                                    BOOST_LOG_SEV(slg, fail)    << " Connection success to the first BASLER camera. ";
                                    cout                        << " Connection success to the first BASLER camera. " << endl;

                                    switch(ft.ACQ_BIT_DEPTH){

                                       case MONO_8 :

                                            if(!inputCam->setCameraPixelFormat(MONO_8))
                                                throw "ERROR :Failed to set camera with MONO_8";

                                            break;

                                       case MONO_12 :

                                            if(!inputCam->setCameraPixelFormat(MONO_12))
                                                throw "ERROR :Failed to set camera with MONO_12";

                                            break;

                                    }

                                    if(!inputCam->setCameraExposureTime(ft.ACQ_EXPOSURE))
                                        throw "> Failed to set camera exposure.";

                                    if(!inputCam->setCameraGain(ft.ACQ_GAIN))
                                        throw "> Failed to set camera gain.";

                                    if(!inputCam->setCameraFPS(ft.ACQ_FPS))
                                        throw "> Failed to set camera fps.";

                                }

                                break;

                            case DMK:

                                cout                                << " INPUT = DMK ";
                                BOOST_LOG_SEV(slg, notification)    << " INPUT = DMK ";

                                inputCam = new CameraDMK(   &queueRAM,
                                                            &m_queueRAM,
                                                            &c_queueRAM_Full,
                                                            &c_queueRAM_New,
                                                            &m_threadEnd,
                                                            &stopFlag);

                                inputCam->getListCameras();

                                if(!inputCam->setSelectedDevice(ft.CAMERA_NAME)){

                                    throw "ERROR : Connection failed to " + ft.CAMERA_NAME;

                                }else{

                                    BOOST_LOG_SEV(slg, fail)    << " Connection success to DMK. ";
                                    cout                        << " Connection success to DMK. " << endl;

                                    // Set bit depth.
                                    switch(ft.ACQ_BIT_DEPTH){

                                       case MONO_8 :

                                            if(!inputCam->setCameraPixelFormat(MONO_8))
                                                throw "ERROR :Failed to set camera with MONO_8";

                                            break;

                                       case MONO_12 :

                                            if(!inputCam->setCameraPixelFormat(MONO_12))
                                                throw "ERROR :Failed to set camera with MONO_12";

                                            break;

                                    }

                                    if(!inputCam->setCameraExposureTime(ft.ACQ_EXPOSURE))
                                        throw "> Failed to set camera exposure.";

                                    if(!inputCam->setCameraGain(ft.ACQ_GAIN))
                                        throw "> Failed to set camera gain.";

                                    if(!inputCam->setCameraFPS(ft.ACQ_FPS))
                                        throw "> Failed to set camera fps.";
                                }

                                break;

                            case VIDEO :

                                cout << "Video in input : " << ft.VIDEO_PATH <<endl;
                                BOOST_LOG_SEV(slg, notification) << "Video in input : " << ft.VIDEO_PATH;

                                inputCam = new CameraVideo( 	ft.VIDEO_PATH,
                                                                &queueRAM,
                                                                &m_queueRAM,
                                                                &c_queueRAM_Full,
                                                                &c_queueRAM_New);

                                break;

                            case FRAMES :

                                {

                                    cout << "> Single frames in input." << endl;

                                    // Get frame format.
                                    path p(ft.FRAMES_PATH);

                                    namespace fs = boost::filesystem;

                                    string filename = "";

                                    if(fs::exists(p)){

                                        std::cout << "> Frames directory " << p.string() << " exists." << '\n';

                                        for(directory_iterator file(p);file!= directory_iterator(); ++file){

                                            path curr(file->path());

                                            if(is_regular_file(curr)){

                                                filename = file->path().c_str() ;
                                                break;

                                            }
                                        }

                                    }else{

                                        throw "> Frames directory not found.";

                                    }

                                    if(filename == "")
                                        throw "> No file found in the frame directory.";

                                    Mat resMat;

                                    Fits2D newFits;
                                    int format = 0;

                                    if(!newFits.readIntKeyword(filename, "BITPIX", format))
                                        throw "> Failed to read fits keyword : BITPIX";

                                    inputCam = new CameraFrames(	ft.FRAMES_PATH,
                                                                    ft.FRAMES_START,
                                                                    ft.FRAMES_STOP,
                                                                    &queueRAM,
                                                                    &m_queueRAM,
                                                                    &c_queueRAM_Full,
                                                                    &c_queueRAM_New,
                                                                    fitsHeader,
                                                                    format);

                                }

                                break;

                            default :

                                break;

                        }



                        if( inputCam != NULL ){

                            BOOST_LOG_SEV(slg, notification) << "Program starting.";

                            // start acquisition thread
                            inputCam->startThread();


                            /// ------------------------------------------------
                            ///               Create stack thread
                            /// ------------------------------------------------

                            if(ft.STACK_ENABLED){

                                ast = new AstThread(    ft.DATA_PATH,
                                                        ft.STATION_NAME,
                                                        ft.STACK_MTHD,
                                                        configPath,
                                                        &queueRAM,
                                                        ft.STACK_INTERVAL,
                                                        ft.STACK_TIME,
                                                        ft.ACQ_BIT_DEPTH,
                                                        ft.LONGITUDE,
                                                        &m_queueRAM,
                                                        &c_queueRAM_Full,
                                                        &c_queueRAM_New,
                                                        fitsHeader,
                                                        ft.ACQ_FPS,
                                                        ft.STACK_REDUCTION);

                                ast->startCapture();

                            }

                            /// ------------------------------------------------
                            ///            Create detection thread
                            /// ------------------------------------------------

                            if(ft.DET_ENABLED){

                                det  = new DetThread(   mask,
                                                        1,
                                                        ft.ACQ_BIT_DEPTH,
                                                        &queueRAM,
                                                        &m_queueRAM,
                                                        &c_queueRAM_Full,
                                                        &c_queueRAM_New,
                                                        &c_queueEvToRecNew,
                                                        &m_queueEvToRec,
                                                        &queueEvToRec,
                                                        30 * ft.DET_TIME_AFTER,
                                                        ft.DET_GE_MAX,
                                                        ft.DET_TIME_MAX,
                                                        ft.DATA_PATH,
                                                        ft.STATION_NAME,
                                                        ft.DEBUG_ENABLED,
                                                        ft.DEBUG_PATH,
                                                        ft.DET_DOWNSAMPLE_ENABLED,
                                                        fitsHeader);

                                det->startDetectionThread();

                                /// ------------------------------------------------
                                ///            Create record thread
                                /// ------------------------------------------------

                                rec = new RecThread(    ft.DATA_PATH,
                                                        &queueEvToRec,
                                                        &m_queueEvToRec,
                                                        &c_queueEvToRecNew,
                                                        ft.ACQ_BIT_DEPTH,
                                                        ft.DET_SAVE_AVI,
                                                        ft.DET_SAVE_FITS3D,
                                                        ft.DET_SAVE_FITS2D,
                                                        ft.DET_SAVE_SUM,
                                                        ft.DET_SAVE_POS,
                                                        ft.DET_SAVE_BMP,
                                                        ft.DET_SAVE_TRAIL,
                                                        ft.DET_SAVE_GEMAP,
                                                        fitsHeader,
                                                        ft.MAIL_RECIPIENT,
                                                        ft.STATION_NAME,
                                                        ft.MAIL_ENABLE);

                                rec->start();

                            }


                            // main thread loop
                            BOOST_LOG_SEV(slg, notification) << "FreeTure is working...";
                            cout << "FreeTure is working..."<<endl;

                            if(ft.CAMERA_TYPE == BASLER || ft.CAMERA_TYPE == DMK){

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

                                    if(ft.CFG_FILECOPY_ENABLED){

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

                                    if(executionTime != 0){

                                        if(cptTime > executionTime){

                                            BOOST_LOG_SEV(slg, notification) << "Break main loop";
                                            loopBroken = true;
                                            break;
                                        }
                                        cptTime ++;

                                    }

                                }

                            }else{

                                inputCam->join();

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


                            if(ast!=NULL){
                                cout << "Send signal to stop image capture thread for astrometry." << endl;
                                BOOST_LOG_SEV(slg, notification) << "Send signal to stop image capture thread for astrometry.";
                                ast->stopCapture();
                                delete ast;

                            }

                            //stop acquisition thread
                            BOOST_LOG_SEV(slg, notification) << "Send signal to stop acquisition thread.";
                            inputCam->stopThread();

                            if (inputCam!=NULL)
                                delete inputCam;

                        }

                    }

                    break;

                case 4 :

                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%% MODE 4 : RUN ACQ TEST %%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    {

                        cout << "================================================" << endl;
                        cout << "======= FREETURE - Acquisition test mode =======" << endl;
                        cout << "================================================" << endl << endl;

                        // Display or not the grabbed frame.
                        if(vm.count("display"))     display     = vm["display"].as<bool>();

                        // Path where to save files.
                        if(vm.count("savepath"))    savePath    = vm["savepath"].as<string>();

                        // Acquisition format.
                        if(vm.count("bitdepth"))    acqFormat   = vm["bitdepth"].as<int>();

                        CamBitDepth camFormat;

                        switch(acqFormat){

                            case 8 :

                                {
                                    camFormat = MONO_8;
                                }

                                break;

                            case 12 :

                                {
                                    camFormat = MONO_12;
                                }

                                break;

                            default :

                                    throw "> The acquisition format specified in program options is not allowed.";

                                break;

                        }

                        // Cam id.
                        if(vm.count("id"))          camID     = vm["id"].as<int>();

                        // Save bmp.
                        if(vm.count("bmp"))         saveBmp     = vm["bmp"].as<bool>();

                        // Save fits.
                        if(vm.count("fits"))        saveFits2D  = vm["fits"].as<bool>();

                        // Type of camera in input.
                        if(vm.count("camtype"))     camtype     = vm["camtype"].as<string>();

                        EnumParser<CamType> cam_type;

                        std::transform(camtype.begin(), camtype.end(),camtype.begin(), ::toupper);

                        // Gain value.
                        if(vm.count("gain"))

                            gain = vm["gain"].as<int>();

                        else{

                            throw "Please define the gain value.";

                        }

                        // Exposure value.
                        if(vm.count("exposure"))

                           exp = vm["exposure"].as<int>();

                        else{

                            throw "Please define the exposure time value.";

                        }

                        Camera  *inputCam = NULL;
                        string  cam;        // Camera name
                        string  date = "";  // Acquisition date
                        Mat     frame;      // Image data

                        namedWindow( "Frame", WINDOW_AUTOSIZE );

                        switch(cam_type.parseEnum("CAMERA_TYPE", camtype)){

                            case BASLER :

                                {

                                    inputCam = new CameraBasler(exp, gain, camFormat);

                                }

                                break;

                            case DMK :

                                {

                                    inputCam = new CameraDMK(exp, gain, camFormat);

                                }

                                break;

                        }

                        if(inputCam == NULL)
                            throw "> Failed to create Camera object.";

                        cout << "> Trying to get the camera." << endl;

                        // Retrieve the camera device.
                        if(inputCam->getDeviceById(camID,cam))

                            cout << "> Camera found : " << cam << "." << endl;

                        else

                            throw "> No camera found.";

                        cout<< "> Trying to connect to the camera ..."<<endl;

                        // Select the first camera.
                        if(!inputCam->setSelectedDevice(cam)){

                            throw "> Connection failed.";

                        }else{

                            cout << "> Connection success." << endl;

                            if(!inputCam->setCameraPixelFormat(camFormat))
                                throw "> Failed to set camera bit depth.";

                            if(!inputCam->setCameraExposureTime(exp))
                                throw "> Failed to set camera exposure.";

                            if(!inputCam->setCameraGain(gain))
                                throw "> Failed to set camera gain.";

                            if(!inputCam->startGrab())
                                throw "> Failed to initialize the grab.";

                            if(!inputCam->grabSingleFrame(frame, date))
                                throw "> Failed to grab a single frame.";

                            // Display the frame in an opencv window
                            if(display){

                                Mat temp;

                                if(camFormat == MONO_12){

                                    temp = Conversion::convertTo8UC1(frame);

                                }else

                                    frame.copyTo(temp);

                                imshow( "Frame", temp );
                                waitKey(0);

                            }

                            // Save the frame in BMP.
                            if(saveBmp){

                                Mat temp;

                                if(camFormat == MONO_12){

                                    temp = Conversion::convertTo8UC1(frame);

                                }else

                                    frame.copyTo(temp);

                                SaveImg::saveBMP(temp, savePath + "frame");

                            }

                            // Save the frame in Fits 2D.
                            if(saveFits2D){

                                Fits fitsHeader;
                                fitsHeader.setGaindb((int)gain);
                                fitsHeader.setExposure(exp);

                                Fits2D newFits(savePath + "frame",fitsHeader);

                                switch(camFormat){

                                    case MONO_8 :

                                        {

                                            if(newFits.writeFits(frame, UC8, 0, true,"" ))
                                                cout << "> Fits saved in " << savePath << endl;
                                            else
                                                cout << "Failed to save Fits." << endl;

                                        }

                                        break;

                                    case MONO_12 :

                                        {

                                            if(newFits.writeFits(frame, US16, 0, true,"" ))
                                                cout << "> Fits saved in " << savePath << endl;
                                            else
                                                cout << "Failed to save Fits." << endl;

                                        }

                                        break;

                                }
                            }
                        }
                    }

                    break;

                case 5 :

                    {


                        namespace fs = boost::filesystem;

                        path p("/home/fripon/Orsay_20141122_191438UT-0.fit");

                        if(is_regular_file(p)){

                            Fits fitsHeader;
                            fitsHeader.loadKeywordsFromConfigFile("/home/fripon/fripon/freeture/share/configuration.cfg");

                            Mat resMat, res;

                            Fits2D newFits("/home/fripon/Orsay_20141122_191438UT-0.fit",fitsHeader);
                            newFits.readFits32F(resMat, "/home/fripon/Orsay_20141122_191438UT-0.fit");
                            resMat.copyTo(res);

                            cout << "before"<<endl;
                            Mat newMat ;
                            float bz, bs;
                            //ImgReduction::dynamicReductionBasedOnHistogram(99.5, resMat).copyTo(newMat);
                            ImgReduction::dynamicReductionByFactorDivision(resMat,MONO_12,1800,bz,bs).copyTo(newMat);

                            Fits2D newFits2("/home/fripon/testFits_",fitsHeader);


                           //Rechercher la valeur max 7371000 par rapport au n * 4095

                            newFits2.setBzero(bz/*32768*112.4725*/);
                            newFits2.setBscale(bs/*112.4725*/);
                            newFits2.writeFits(newMat, bit_depth_enum::S16, 0, true,"" );

                            double minVal2, maxVal2;
                            minMaxLoc(newMat, &minVal2, &maxVal2);

                            cout << "> Max : "<< maxVal2<<endl;
                            cout << "> Min : "<< minVal2<<endl<<endl;






                        }

                    }

                    break;

                case 6:

                    {


                        FreeTure ft(configPath);
                        ft.loadParameters();

                        for(int i = 0; i < ft.MAIL_RECIPIENT.size(); i++){

                            cout << ft.MAIL_RECIPIENT.at(i) << endl;

                        }






                        vector<string> to;
                        to.push_back("yoan.audureau@gmail.com");
                        to.push_back("fripon@ceres.geol.u-psud.fr");

                        vector<string> pathAttachments;
                        //pathAttachments.push_back("/home/fripon/frame.bmp");

                        //pathAttachments.push_back("D:/logoFripon.png");

                        string acquisitionDate = TimeDate::localDateTime(second_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");

                        //SMTPClient mailc("smtp.u-psud.fr", 25, "u-psud.fr");

                        SMTPClient mailc("10.8.0.1", 25, "u-psud.fr");
                        mailc.send("yoan.audureau@u-psud.fr", to, "station ORSAY "+acquisitionDate+" UT", "Test d'acquisition à " + acquisitionDate, pathAttachments, true);


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
                        cout << "5 : Full FreeTure checkup"                         << endl;

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

	return 0 ;

}




/*
            ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            ///%%%%%%%%%%%%%%%%%% MODE 5 : FULL FreeTure checkup %%%%%%%%%%%%%%%%%%%%%
            ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

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
    /*

                Fits fitsHeader;
                fitsHeader.loadKeywordsFromConfigFile("/home/fripon/friponProject/friponCapture/configuration.cfg");

                Camera *inputCam = NULL;

                inputCam = new CameraBasler(1000,
                                            400,
                                            false,
                                            true,
                                            12,
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

    */

             /*   vector<string> to;
                to.push_back("yoan.audureau@gmail.com");
                to.push_back("yoan.audureau@yahoo.fr");

                vector<string> pathAttachments;
                pathAttachments.push_back("/home/fripon/capture.bmp");

                //pathAttachments.push_back("D:/logoFripon.png");

                string acquisitionDate = TimeDate::localDateTime(second_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");

                SMTPClient mailc("smtp.u-psud.fr", 25, "u-psud.fr");
                mailc.send("yoan.audureau@u-psud.fr", to, "station ORSAY "+acquisitionDate+" UT", "Test d'acquisition d'une frame sur la station d'Orsay à " + acquisitionDate, pathAttachments, true);

    */




/*
            }else if(mode == 6){


                namespace fs = boost::filesystem;

                path p("/home/fripon/Orsay_20141122_191438UT-0.fit");

                if(is_regular_file(p)){

                    Fits fitsHeader;
                    fitsHeader.loadKeywordsFromConfigFile("/home/fripon/fripon/freeture/share/configuration.cfg");

                    Mat resMat, res;

                    Fits2D newFits("/home/fripon/Orsay_20141122_191438UT-0.fit",fitsHeader);
                    newFits.readFits32F(resMat, "/home/fripon/Orsay_20141122_191438UT-0.fit");
                    resMat.copyTo(res);

                    cout << "before"<<endl;
                    Mat newMat ;
                    //ImgReduction::dynamicReductionBasedOnHistogram(99.5, resMat).copyTo(newMat);
                    ImgReduction::dynamicReductionByFactorDivision(resMat).copyTo(newMat);

                    Fits2D newFits2("/home/fripon/testFits_",fitsHeader);


                   //Rechercher la valeur max 7371000 par rapport au n * 4095

                    newFits2.setBzero(32768*112.4725);
                    newFits2.setBscale(112.4725);
                    newFits2.writeFits(newMat, bit_depth_enum::S16, 0, true );

                    double minVal2, maxVal2;
                    minMaxLoc(newMat, &minVal2, &maxVal2);

                    cout << "> Max : "<< maxVal2<<endl;
                    cout << "> Min : "<< minVal2<<endl<<endl;


                }*/

