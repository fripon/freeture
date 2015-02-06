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
*	Last modified:		01/12/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    main.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    01/12/2014
*/

#include "includes.h"
#include <algorithm>
#include "ImgReduction.h"
#include "CameraVideo.h"
#include "CameraFrames.h"
#include "Camera.h"
#include "Configuration.h"
#include "Frame.h"
#include "DetThread.h"
#include "AstThread.h"
#include "RecEvent.h"
#include "SaveImg.h"
#include "Conversion.h"
#include "Fits2D.h"
#include "ManageFiles.h"
#include "TimeDate.h"
#include "Histogram.h"
#include "EImgBitDepth.h"
#include "SMTPClient.h"
#include "FreeTure.h"
#include "ECamType.h"
#include "EParser.h"
#define BOOST_NO_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#include <boost/circular_buffer.hpp>

#include <boost/date_time.hpp>

using namespace boost::filesystem;

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

template <typename Container> void stringtokk(Container &container, string const &in, const char * const delimiters = "_"){


    const string::size_type len = in.length();

    string::size_type i = 0;


    while (i < len){


        // Eat leading whitespace

        i = in.find_first_not_of(delimiters, i);


        if (i == string::npos)

            return;   // Nothing left but white space


        // Find the end of the token

        string::size_type j = in.find_first_of(delimiters, i);


        // Push token

        if (j == string::npos){


            container.push_back(in.substr(i));

            return;


        }else


            container.push_back(in.substr(i, j-i));


        // Set up for next loop

        i = j + 1;

    }

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
        string  version         = string(PACKAGE_VERSION);
        string  camtype         = "basler";
        bool    display         = false;
        int     camID           = 0;
        int     fps             = 30;

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

            if(!fs::exists(cPath) && mode != 4 && mode != 1 ){

                throw runtime_error("Configuration file not found. Check path : " + configPath);

            }

            switch(mode){

                case 1 :

                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%% MODE 1 : LIST CONNECTED CAMERAS %%%%%%%%%%%%%%%%%%
                    ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    {
                        cout << "================================================" << endl;
                        cout << "========= FREETURE - Available cameras =========" << endl;
                        cout << "================================================" << endl << endl;

                        if(vm.count("camtype")) camtype = vm["camtype"].as<string>();

                        EParser<CamType> cam_type;

                        std::transform(camtype.begin(), camtype.end(),camtype.begin(), ::toupper);

                        //cout << "Searching " << camtype << " cameras..." << endl << endl;
                        cout << "Searching cameras..." << endl << endl;

                        Camera *cam = new Camera(cam_type.parseEnum("CAMERA_TYPE", camtype));
                        cam->getListCameras();

                        if(cam != NULL)
                            delete cam;

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

                        path pLog(ft.LOG_PATH);

                        if(!boost::filesystem::exists(pLog)){

                            if(!create_directory(pLog))
                                throw "> Failed to create a directory for logs files.";
                        }

                        // log configuration
                        init_log(ft.LOG_PATH, ft.LOG_SEVERITY);
                        src::severity_logger< LogSeverityLevel > slg;
                        slg.add_attribute("ClassName", boost::log::attributes::constant<std::string>("main.cpp"));
                        BOOST_LOG_SCOPED_THREAD_TAG("LogName", "mainThread");
                        BOOST_LOG_SEV(slg, notification) << " FREETURE - Meteor detection mode ";

                        // Circular buffer for last frames.
                        boost::circular_buffer<Frame> framesBuffer(ft.ACQ_BUFFER_SIZE * ft.ACQ_FPS);
                        boost::mutex m_framesBuffer;
                        boost::condition_variable c_newElemFramesBuffer;

                        bool newFrameForDet = false;
                        boost::mutex m_newFrameForDet;
                        boost::condition_variable c_newFrameForDet;

                        // Circular buffer for stacked frames.
                        boost::circular_buffer<StackedFrames> stackedFramesBuffer(1);
                        boost::mutex m_stackedFramesBuffer;
                        boost::condition_variable c_newElemStackedFramesBuffer;

                        // Input device type
                        Camera          *inputCam       = NULL;
                        CameraVideo     *inputCamVideo  = NULL;
                        CameraFrames    *inputCamFrame  = NULL;
                        DetThread       *det            = NULL;
                        AstThread       *ast            = NULL;

                        Mat mask;

                        /// ------------------------------------------------
                        ///                   Load Mask
                        /// ------------------------------------------------

                        if(ft.ACQ_MASK_ENABLED){

                            BOOST_LOG_SEV(slg, normal) << " Masking enabled ";

                            mask = imread(ft.ACQ_MASK_PATH, CV_LOAD_IMAGE_GRAYSCALE);

                            if(!mask.data){
                                BOOST_LOG_SEV(slg, critical) << " Can't load the mask from this location : " << ft.ACQ_MASK_PATH;
                                throw "Can't load the mask.";
                            }

                        }

                        /// ------------------------------------------------
                        ///            Create acquisition thread
                        /// ------------------------------------------------

                        switch(ft.CAMERA_TYPE){

                            case BASLER :
                            case DMK :

                                BOOST_LOG_SEV(slg, notification) << " GigE camera in input : " << ft.CAMERA_TYPE;

                                inputCam = new Camera(  ft.CAMERA_TYPE,
                                                        ft.ACQ_EXPOSURE,
                                                        ft.ACQ_GAIN,
                                                        ft.ACQ_BIT_DEPTH,
                                                        ft.ACQ_FPS,
                                                        ft.STACK_TIME * ft.ACQ_FPS,
                                                        ft.STACK_INTERVAL * ft.ACQ_FPS,
                                                        ft.STACK_ENABLED,
                                                        &framesBuffer,
                                                        &m_framesBuffer,
                                                        &c_newElemFramesBuffer,
                                                        &stackedFramesBuffer,
                                                        &m_stackedFramesBuffer,
                                                        &c_newElemStackedFramesBuffer,
                                                        &newFrameForDet,
                                                        &m_newFrameForDet,
                                                        &c_newFrameForDet);

                                inputCam->getListCameras();

                                if(!inputCam->setSelectedDevice(ft.CAMERA_NAME)){
                                    BOOST_LOG_SEV(slg, critical) << " Connection failed to the camera : " << ft.CAMERA_NAME;
                                    throw runtime_error("Connection failed to the camera : " + ft.CAMERA_NAME);

                                }else{

                                    BOOST_LOG_SEV(slg, notification) << " Connection success to the " << ft.CAMERA_NAME << " camera.";
                                    cout << "> Connection success to the " << ft.CAMERA_NAME << " camera." << endl;

                                    switch(ft.ACQ_BIT_DEPTH){

                                       case MONO_8 :

                                            if(!inputCam->setCameraPixelFormat(MONO_8)){
                                                BOOST_LOG_SEV(slg, critical) << " Failed to set camera with MONO_8 ";
                                                throw "ERROR :Failed to set camera with MONO_8";
                                            }

                                            break;

                                       case MONO_12 :

                                            if(!inputCam->setCameraPixelFormat(MONO_12)){
                                                BOOST_LOG_SEV(slg, critical) << " Failed to set camera with MONO_12 ";
                                                throw "ERROR :Failed to set camera with MONO_12";
                                            }

                                            break;

                                    }

                                    if(!inputCam->setCameraExposureTime(ft.ACQ_EXPOSURE)){
                                        BOOST_LOG_SEV(slg, critical) << " Failed to set camera exposure.";
                                        throw "> Failed to set camera exposure.";
                                    }

                                    if(!inputCam->setCameraGain(ft.ACQ_GAIN)){
                                        BOOST_LOG_SEV(slg, critical) << " Failed to set camera gain. ";
                                        throw "> Failed to set camera gain.";
                                    }

                                    if(!inputCam->setCameraFPS(ft.ACQ_FPS)){
                                        BOOST_LOG_SEV(slg, critical) << " Failed to set camera fps.";
                                        throw "> Failed to set camera fps.";
                                    }

                                }

                                break;

                            case VIDEO :

                                cout << "Video in input : " << ft.VIDEO_PATH <<endl;
                                BOOST_LOG_SEV(slg, notification) << " Video in input : " << ft.VIDEO_PATH;

                                inputCamVideo = new CameraVideo( 	ft.VIDEO_PATH,
                                                                    &framesBuffer,
                                                                    &m_framesBuffer,
                                                                    &c_newElemFramesBuffer,
                                                                    &newFrameForDet,
                                                                    &m_newFrameForDet,
                                                                    &c_newFrameForDet);

                                break;

                            case FRAMES :

                                {

                                    cout << "> Single frames in input." << endl;
                                    BOOST_LOG_SEV(slg, notification) << " Frames in input";

                                    // Get frame format.
                                    path p(ft.FRAMES_PATH);

                                    namespace fs = boost::filesystem;

                                    string filename = "";

                                    if(fs::exists(p)){

                                        BOOST_LOG_SEV(slg, normal) << " Frames directory " << p.string() << " exists.";

                                        for(directory_iterator file(p);file!= directory_iterator(); ++file){

                                            path curr(file->path());

                                            if(is_regular_file(curr)){

                                                filename = file->path().c_str() ;
                                                break;

                                            }
                                        }

                                    }else{

                                        BOOST_LOG_SEV(slg, critical) << " Frames directory not found.";
                                        throw "> Frames directory not found.";

                                    }

                                    if(filename == ""){
                                        BOOST_LOG_SEV(slg, fail) << " No file found in the frame directory.";
                                        throw "> No file found in the frame directory.";
                                    }

                                    Mat resMat;

                                    Fits2D newFits;
                                    int format, framewidth, frameheight;

                                    if(!newFits.readIntKeyword(filename, "BITPIX", format)){
                                        BOOST_LOG_SEV(slg, critical) << " Failed to read fits keyword : BITPIX";
                                        throw "> Failed to read fits keyword : BITPIX";
                                    }

                                    if(!newFits.readIntKeyword(filename, "NAXIS1", framewidth)){
                                        BOOST_LOG_SEV(slg, critical) << " Failed to read fits keyword : NAXIS1";
                                        throw "> Failed to read fits keyword : NAXIS1";
                                    }

                                    if(!newFits.readIntKeyword(filename, "NAXIS2", frameheight)){
                                        BOOST_LOG_SEV(slg, critical) << " Failed to read fits keyword : NAXIS2";
                                        throw "> Failed to read fits keyword : NAXIS2";
                                    }

                                    // Check mask size
                                    if(mask.data && (framewidth != mask.cols || frameheight != mask.rows)){
                                        BOOST_LOG_SEV(slg, critical) << "Mask's size (" << Conversion::intToString(mask.cols) << ";" << Conversion::intToString(mask.rows) << ") doesn't fit to frame's size (" << Conversion::intToString(framewidth) << ";" << Conversion::intToString(frameheight) <<")";
                                        throw runtime_error("Mask's size (" + Conversion::intToString(mask.cols) + ";" + Conversion::intToString(mask.rows) + ") doesn't fit to frame's size (" + Conversion::intToString(framewidth) + ";" + Conversion::intToString(frameheight) +")");
                                    }

                                    inputCamFrame = new CameraFrames(	ft.FRAMES_PATH,
                                                                        ft.FRAMES_SEPARATOR_POSITION,
                                                                        ft.FRAMES_SEPARATOR,
                                                                        fitsHeader,
                                                                        format,
                                                                        &framesBuffer,
                                                                        &m_framesBuffer,
                                                                        &c_newElemFramesBuffer,
                                                                        &newFrameForDet,
                                                                        &m_newFrameForDet,
                                                                        &c_newFrameForDet);

                                }

                                break;

                            default :

                                break;

                        }

                        if( inputCam != NULL || inputCamFrame != NULL || inputCamVideo != NULL){

                            if(inputCam != NULL) inputCam->startThread();
                            if(inputCamVideo != NULL) inputCamVideo->startThread();
                            if(inputCamFrame != NULL) inputCamFrame->startThread();

                            /// ------------------------------------------------
                            ///               Create stack thread
                            /// ------------------------------------------------

                            if(ft.STACK_ENABLED){


                                ast = new AstThread(    ft.DATA_PATH,
                                                        ft.STATION_NAME,
                                                        ft.STACK_MTHD,
                                                        configPath,
                                                        ft.STACK_TIME,
                                                        ft.ACQ_BIT_DEPTH,
                                                        ft.LONGITUDE,
                                                        fitsHeader,
                                                        ft.ACQ_FPS,
                                                        ft.STACK_REDUCTION,
                                                        &stackedFramesBuffer,
                                                        &m_stackedFramesBuffer,
                                                        &c_newElemStackedFramesBuffer
                                                    );

                                ast->startThread();

                            }

                            /// ------------------------------------------------
                            ///            Create detection thread
                            /// ------------------------------------------------

                            if(ft.DET_ENABLED){

                                det  = new DetThread(   mask,
                                                        1,
                                                        ft.ACQ_BIT_DEPTH,
                                                        ft.DET_METHOD,
                                                        ft.ACQ_FPS * ft.DET_TIME_AFTER,
                                                        ft.ACQ_BUFFER_SIZE * ft.ACQ_FPS,
                                                        ft.DET_GE_MAX,
                                                        ft.DET_TIME_MAX,
                                                        ft.DATA_PATH,
                                                        ft.STATION_NAME,
                                                        ft.DEBUG_ENABLED,
                                                        ft.DEBUG_PATH,
                                                        ft.DET_DOWNSAMPLE_ENABLED,
                                                        fitsHeader,
                                                        &framesBuffer,
                                                        &m_framesBuffer,
                                                        &c_newElemFramesBuffer,
                                                        &newFrameForDet,
                                                        &m_newFrameForDet,
                                                        &c_newFrameForDet,
                                                        ft.DET_SAVE_AVI,
                                                        ft.DET_SAVE_FITS3D,
                                                        ft.DET_SAVE_FITS2D,
                                                        ft.DET_SAVE_SUM,
                                                        ft.DET_SAVE_POS,
                                                        ft.DET_SAVE_BMP,
                                                        ft.DET_SAVE_GEMAP,
                                                        ft.ACQ_FPS * ft.DET_TIME_BEFORE,
                                                        ft.MAIL_DETECTION_ENABLED,
                                                        ft.MAIL_SMTP_SERVER,
                                                        ft.MAIL_SMTP_HOSTNAME,
                                                        ft.MAIL_RECIPIENT);

                                det->startDetectionThread();

                            }


                            // main thread loop
                            BOOST_LOG_SEV(slg, notification) << "FreeTure is working ...";
                            cout << "FreeTure is working ..."<<endl;

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
                                    }

                                    #ifdef _WIN64_
                                        Sleep(1000);
                                    #elif defined _LINUX_
                                        sleep(1);
                                    #endif

                                    if(executionTime != 0){

                                        if(cptTime > executionTime){

                                            BOOST_LOG_SEV(slg, notification) << "Break main loop";

                                            break;
                                        }
                                        cptTime ++;

                                    }

                                }

                            }else{

                                if(inputCam != NULL) inputCam->join();
                                if(inputCamVideo != NULL) inputCamVideo->join();
                                if(inputCamFrame != NULL) inputCamFrame->join();

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

                              /*  if(rec != NULL){
                                    BOOST_LOG_SEV(slg, notification) << "Send signal to stop rec thread.";
                                    rec->stop();
                                    delete rec;
                                }*/
                            }


                            if(ast!=NULL){
                                cout << "Send signal to stop image capture thread for astrometry." << endl;
                                BOOST_LOG_SEV(slg, notification) << "Send signal to stop image capture thread for astrometry.";
                                ast->stopThread();
                                delete ast;

                            }

                            //stop acquisition thread
                            BOOST_LOG_SEV(slg, notification) << "Send signal to stop acquisition thread.";


                            if(inputCam != NULL){
                                inputCam->stopThread();
                                delete inputCam;
                            }

                            if(inputCamVideo != NULL){

                                delete inputCamVideo;

                            }

                            if(inputCamFrame != NULL){

                                delete inputCamFrame;

                            }

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

                        // FPS.
                        if(vm.count("fps"))         fps         = vm["fps"].as<int>();

                        // Save bmp.
                        if(vm.count("bmp"))         saveBmp     = vm["bmp"].as<bool>();

                        // Save fits.
                        if(vm.count("fits"))        saveFits2D  = vm["fits"].as<bool>();

                        // Type of camera in input.
                        if(vm.count("camtype"))     camtype     = vm["camtype"].as<string>();

                        EParser<CamType> cam_type;

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


                        inputCam = new Camera(cam_type.parseEnum("CAMERA_TYPE", camtype), exp, gain, camFormat);


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
/*

                            if(!inputCam->setCameraFPS(fps));
                                throw "> Failed to set camera fps.";
*/

                            if(!inputCam->setCameraPixelFormat(camFormat))
                                throw "> Failed to set camera bit depth.";

                            if(!inputCam->setCameraExposureTime(exp))
                                throw "> Failed to set camera exposure.";

                            if(!inputCam->setCameraGain(gain))
                                throw "> Failed to set camera gain.";

                            if(!inputCam->startGrab(fps))
                                throw "> Failed to initialize the grab.";

                            if(!inputCam->grabSingleFrame(frame, date))
                                throw "> Failed to grab a single frame.";

                            // Display the frame in an opencv window
                            if(display){

                                namedWindow( "Frame", WINDOW_AUTOSIZE );

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
                                vector<string> dd;

                                switch(camFormat){

                                    case MONO_8 :

                                        {

                                            if(newFits.writeFits(frame, UC8, dd, false,"capture" ))
                                                cout << "> Fits saved in " << savePath << endl;
                                            else
                                                cout << "Failed to save Fits." << endl;

                                        }

                                        break;

                                    case MONO_12 :

                                        {

                                            if(newFits.writeFits(frame, S16, dd, false,"capture" ))
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

                        path p("/home/fripon/Montsec_20150205_212509_UT.fit");

                        if(is_regular_file(p)){

                            Fits fitsHeader;
                            fitsHeader.loadKeywordsFromConfigFile("/home/fripon/fripon/freeture/share/configuration.cfg");

                            Mat resMat, res;

                            Fits2D newFits("/home/fripon/Montsec_20150205_212509_UT.fit",fitsHeader);
                            newFits.readFits32F(resMat, "/home/fripon/Montsec_20150205_212509_UT.fit");
                            resMat.copyTo(res);

                            double minVal, maxVal;
                            minMaxLoc(res, &minVal, &maxVal);

                            cout << "> Max : "<< maxVal<<endl;
                            cout << "> Min : "<< minVal<<endl<<endl;

                            cout << "before"<<endl;
                            Mat newMat ;
                            float bz, bs;
                            //ImgReduction::dynamicReductionBasedOnHistogram(99.5, resMat).copyTo(newMat);
                            ImgReduction::dynamicReductionByFactorDivision(resMat,MONO_12,1800,bz,bs).copyTo(newMat);

                            Fits2D newFits2("/home/fripon/testFits_",fitsHeader);

vector<string> dd;
                           //Rechercher la valeur max 7371000 par rapport au n * 4095

                            newFits2.setBzero(bz/*32768*112.4725*/);
                            newFits2.setBscale(bs/*112.4725*/);
                            newFits2.writeFits(newMat, S16, dd, false,"test" );

                            double minVal2, maxVal2;
                            minMaxLoc(newMat, &minVal2, &maxVal2);

                            cout << "> Max : "<< maxVal2<<endl;
                            cout << "> Min : "<< minVal2<<endl<<endl;


                        }

                    }

                    break;

                case 6 :

                    {
                        int sizebuffer = 10;

                        Mat f = Mat(960,1280,CV_16UC1,Scalar(120));

                        boost::circular_buffer<Frame>::iterator it;

                        boost::circular_buffer<Frame> cb(sizebuffer);
                        cb.set_capacity(10);

                        cout << "max size: " << cb.max_size() <<endl;

                        for(int i = 1; i<sizebuffer; i++){

                            Frame fr = Frame(f, 400+i, 25, "2014:10:25::10:25:22");
                            cb.push_back(fr);

                        }

                        for(it = cb.begin(); it != cb.end(); ++it){

                            cout << (*it).getGain() << " | ";

                        }

                        cout <<endl << "nb elements : " << cb.size() << endl << endl;

                        Frame fr1(f, 450, 25, "2014:10:25::10:25:22");
                        cb.push_back(fr1);

                        Frame fr2(f, 500, 25, "2014:10:25::10:25:22");
                        cb.push_back(fr2);


                        for(it = cb.begin(); it != cb.end(); ++it){

                            cout << (*it).getGain() << " | ";

                        }

                        cout << "select the last : " << endl;
                        //it= cb.end() - 1;
                        cout << cb.back().getGain()<<endl;

                        cout << "select the prev before last : "<< endl;

                        boost::circular_buffer<Frame>::reverse_iterator rit;

                        //for(int;rit!=)
                        cout << cb.at(cb.size()-2).getGain()<<endl;


                        /*Frame *a = cb.linearize();
                        Frame *dest;

                        memcpy(&dest, &a, sizeof(a));*/

                        //copy(buf.begin(), buf.end(), ostream_iterator<T>(cout, " "));

                        vector<Frame>::iterator it2;
                        vector<Frame> cb2;

                        //std::copy ( cb.begin(), cb.end(), cb2.begin() );

                        cb2.assign(cb.begin()+2,cb.end());

                         for(it2 = cb2.begin(); it2 != cb2.end(); ++it2){

                            cout << (*it2).getGain() << " | ";

                        }

                    }

                    break;

                case 7 :

                    {

                        string f2 = "stationOrsay_01_frame.fit";
                        string f1 = "stationOrsay_frame_20.fit";

                        list<string> ch;

                        stringtokk(ch,f2.c_str(), "_");

                        list<string>::const_iterator lit(ch.begin()), lend(ch.end());
                        int i = 0;
                        int number = 0;

                        for(; lit != lend; ++lit){

                            cout << i <<" : " << (*lit) << endl;

                            if(i == 1){

                                number = atoi((*lit).c_str()); break;
                            }

                            if(i == ch.size() - 1 && i != ch.size() - 1){

                                list<string> ch_;
                                stringtokk(ch_, f1.c_str(), ".");
                                number = atoi(ch_.back().c_str());
                                break;

                            }

                            i++;

                        }

                        cout << "number : " << number << endl;
                    }


                    break;

                case 8 :

                    {

                        vector<string> to;
                        to.push_back("yoan.audureau@gmail.com");
                        to.push_back("yoan.audureau@yahoo.fr");

                        vector<string> pathAttachments;

                        SMTPClient mailc("smtp.u-psud.fr", 25, "u-psud.fr");
                        mailc.send("yoan.audureau@u-psud.fr", to, "test", "test" , pathAttachments, false);

                    }

                    break;

                case 9 :

                    {

                        cout << "#### TEST SIDERAL TIME ####" << endl << endl;

                        int year        = 0;
                        int month       = 0;
                        int day         = 0;
                        int hour        = 0;
                        int minute      = 0;
                        float seconds  = 0.6f;

                        boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
                        string acqDateInMicrosec = "2014-03-25T11:29:23";//to_iso_extended_string(time);

                        cout << "DATE : " << acqDateInMicrosec << endl;

                        string separator = "T";

                        list<string> ch;
                        stringtokk(ch, acqDateInMicrosec, separator.c_str());

                        string YYYYMMDD = ch.front();
                        string HHMMSS   = ch.back();

                        cout << "First part : " << YYYYMMDD << endl;
                        cout << "Sec   part : " << HHMMSS   << endl;

                        separator = "-";

                        list<string> ch1;
                        stringtokk(ch1, YYYYMMDD, separator.c_str());
                        std:list<string>::const_iterator lit(ch1.begin()), lend(ch1.end());

                        int i = 0;

                        for(; lit != lend; ++lit){

                            if(i == 0)
                                year = atoi((*lit).c_str());

                            if(i == 1)
                                month = atoi((*lit).c_str());

                            if(i == 2)
                                day = atoi((*lit).c_str());

                            i++;

                        }

                        i = 0;

                        separator = ":";

                        list<string> ch2;
                        stringtokk(ch2, HHMMSS, separator.c_str());
                        list<string>::const_iterator lit2(ch2.begin()), lend2(ch2.end());

                        for(; lit2 != lend2; ++lit2){

                            if(i == 0)
                                hour = atoi((*lit2).c_str());

                            if(i == 1)
                                minute = atoi((*lit2).c_str());

                            if(i == 2)
                                seconds = atof((*lit2).c_str());

                            i++;

                        }

                        cout << "hour : " << hour << endl;
                        cout << "minute : " << minute << endl;
                        cout << "seconds : " << seconds << endl;

                        //int     debObsInSeconds = firstDateInt.at(3)*3600 + firstDateInt.at(4)*60 + firstDateInt.at(5);
                        //int     endObsInSeconds = lastDateInt.at(3)*3600 + lastDateInt.at(4)*60 + lastDateInt.at(5);
                        //int     elapTime        = endObsInSeconds - debObsInSeconds;
                        vector <int> YYYYMMDDhhmm;
                        YYYYMMDDhhmm.push_back(year);
                        YYYYMMDDhhmm.push_back(month);
                        YYYYMMDDhhmm.push_back(day);
                        YYYYMMDDhhmm.push_back(hour);
                        YYYYMMDDhhmm.push_back(minute);

                        double  julianDate      = TimeDate::gregorianToJulian_2(YYYYMMDDhhmm);
                        cout << "JULIAN DATE: " << julianDate << endl;
                        double  julianCentury   = TimeDate::julianCentury(julianDate);
                        cout << "JULIAN CENTURY : " << julianCentury << endl;
                        double  sideralT        = TimeDate::localSideralTime_2(julianCentury, YYYYMMDDhhmm.at(3), YYYYMMDDhhmm.at(4), seconds, 2.1794397);
                        cout << ">>> sideralTime : " << sideralT << endl;

                    }

                    break;

                case 10 :

                    {


                        Mat A = Mat(10, 10, CV_8UC3, Scalar(0,0,0));

                        Vec3b bgrPixel = Vec3b(0,0,255);
                        A.at<Vec3b>(0, 0) = bgrPixel;
                        A.at<Vec3b>(9, 9) = Vec3b(255,0,0);
                        A.at<Vec3b>(0, 9) = Vec3b(255,0,0);
                        A.at<Vec3b>(9, 0) = Vec3b(255,0,0);

                       /* for(int i = 0; i < A.rows; i++)
                        {
                            for(int j = 0; j < A.cols; j++)
                            {
                                Vec3b bgrPixel = Vec3b(0,0,255);
                                cout << A.at<Vec3b>(i, j)<< endl;
                                A.at<Vec3b>(i, j) = bgrPixel;

                                // do something with BGR values...
                            }
                        }*/

                        SaveImg::saveBMP(A, "/home/fripon/testColor");


                        FreeTure ft(configPath);
                        ft.loadParameters();

                        init_log("/home/fripon/log/", ft.LOG_SEVERITY);
                        src::severity_logger< LogSeverityLevel > slg;
                        slg.add_attribute("ClassName", boost::log::attributes::constant<std::string>("main.cpp"));

                        boost::gregorian::date current_date(boost::gregorian::day_clock::local_day());
                        cout << to_iso_string(current_date)<<endl;;
                        //http://www.boost.org/doc/libs/1_47_0/doc/html/date_time/gregorian.html#date_construct_from_clock
                        BOOST_LOG_SCOPED_THREAD_TAG("LogName", "logTest_" + to_iso_string(current_date));
                        BOOST_LOG_SEV(slg, normal) << "Test logging in mode 10";
                        BOOST_LOG_SEV(slg, notification) << "Test logging in mode 10";

                        BOOST_LOG_SCOPED_THREAD_TAG("LogName", "logTest_20150206");

                        BOOST_LOG_SEV(slg, fail) << "Test logging in mode 10";
                        BOOST_LOG_SEV(slg, warning) << "Test logging in mode 10";
                        BOOST_LOG_SEV(slg, critical) << "Test logging in mode 10";


                        Fits2D *f = new Fits2D();



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

	return 0 ;

}




/*
            ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            ///%%%%%%%%%%%%%%%%%% MODE 5 : FULL FreeTure checkup %%%%%%%%%%%%%%%%%%%%%
            ///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

            }else if(mode == 5){


                Mat img(960, 1280, CV_8UC3, Scalar(0,0,0));

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

