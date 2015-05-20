/*
								Device.cpp

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

/**
* \file    Device.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    02/09/2014
* \brief
*/

#include "Device.h"

boost::log::sources::severity_logger< LogSeverityLevel >  Device::logger;
Device::_Init Device::_initializer;

Device::Device(CamType type){

    STATION_NAME                    = "STATION";
    DATA_PATH                       = "./";
    ACQ_SCHEDULE_ENABLED            = false;
    ACQ_NIGHT_EXPOSURE              = 0;
    ACQ_NIGHT_GAIN                  = 0;
    ACQ_DAY_EXPOSURE                = 0;
    ACQ_DAY_GAIN                    = 0;
    ACQ_FPS                         = 1;
    CAMERA_ID                       = 0;
    ACQ_MASK_ENABLED                = false;
    ACQ_MASK_PATH                   = "./";
    ACQ_DAY_ENABLED                 = false;
    EPHEMERIS_ENABLED               = true;
    EXPOSURE_CONTROL_SAVE_IMAGE     = false;
    EXPOSURE_CONTROL_SAVE_INFOS     = false;
    EXPOSURE_CONTROL_FREQUENCY      = 300;
    SUNSET_DURATION                 = 3600;
    SUNRISE_DURATION                = 3600;
    minExposureTime                 = 0;
    maxExposureTime                 = 0;
    minGain                         = 0;
    maxGain                         = 0;
    DISPLAY_INPUT                   = false;
    VIDEO_FRAMES_INPUT              = false;

    switch(type){

        case BASLER_GIGE :

            {
                #ifdef USE_PYLON
                    BOOST_LOG_SEV(logger, normal) << "INPUT : BASLER_GIGE -> Use Pylon";
                    cam = new CameraGigeSdkPylon();
                #else
                    #ifdef LINUX
                        BOOST_LOG_SEV(logger, normal) << "INPUT : BASLER_GIGE -> Use Aravis";
                        cam = new CameraGigeSdkAravis();
                    #endif
                #endif
            }

            break;

        case DMK_GIGE:

            {

                #ifdef WINDOWS
                    BOOST_LOG_SEV(logger, normal) << "INPUT : DMK_GIGE -> Use Imaging Source";
                    cam = new CameraGigeSdkIc();
                #else
                    #ifdef LINUX
                        BOOST_LOG_SEV(logger, normal) << "INPUT : DMK_GIGE -> Use Aravis";
                        cam = new CameraGigeSdkAravis(true);
                    #endif
                #endif

            }

            break;

        default :

            cam = NULL;

    }
}

Device::~Device(){

    if(cam != NULL) delete cam;

}

bool Device::prepareDevice(CamType type, string cfgFile){

	try{

        Configuration cfg;
		cfg.Load(cfgFile);

		// Get detection option status.
        cfg.Get("DETECTION_ENABLED", DETECTION_ENABLED);

		switch(type){

			case FRAMES :

				{

                    // Get frames location.
					string INPUT_FRAMES_DIRECTORY_PATH; cfg.Get("INPUT_FRAMES_DIRECTORY_PATH", INPUT_FRAMES_DIRECTORY_PATH);
					BOOST_LOG_SEV(logger, normal) << "Read INPUT_FRAMES_DIRECTORY_PATH from configuration file : " << INPUT_FRAMES_DIRECTORY_PATH;

					// Get separator position in frame's name.
					int FRAMES_SEPARATOR_POSITION; cfg.Get("FRAMES_SEPARATOR_POSITION", FRAMES_SEPARATOR_POSITION);
					BOOST_LOG_SEV(logger, normal) << "Read FRAMES_SEPARATOR_POSITION from configuration file : " << FRAMES_SEPARATOR_POSITION;

					VIDEO_FRAMES_INPUT = true;

					vector<string> framesLocationList;

                    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                    boost::char_separator<char> sep(",");
                    tokenizer tokens(INPUT_FRAMES_DIRECTORY_PATH, sep);

                    int n = 1;

                    for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
                        framesLocationList.push_back(*tok_iter);
                        cout << "LOCATION 1 : " << Conversion::intToString(n) << " : " << *tok_iter << endl;
                        n++;
                    }

                    // Create camera using pre-recorded fits2D in input.
					cam = new CameraFrames(framesLocationList, FRAMES_SEPARATOR_POSITION);
					if(!cam->grabInitialization())
                        throw "Fail to prepare acquisition on the first frames directory.";

				}

				break;

			case VIDEO:

				{
				    // Get frames locations.
					string	INPUT_VIDEO_PATH; cfg.Get("INPUT_VIDEO_PATH", INPUT_VIDEO_PATH);

					// Get display input option.
					cfg.Get("INPUT_VIDEO_DISPLAY", DISPLAY_INPUT);

					VIDEO_FRAMES_INPUT = true;

					vector<string> videoList;

                    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                    boost::char_separator<char> sep(",");
                    tokenizer tokens(INPUT_VIDEO_PATH, sep);

                    int n = 1;

                    for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
                        videoList.push_back(*tok_iter);
                        cout << "VIDEO " << Conversion::intToString(n) << " : " << *tok_iter << endl;
                        n++;
                    }

                    // Create camera using pre-recorded video in input.
					cam = new CameraVideo(videoList);
					cam->grabInitialization();
				}

				break;

			default :

                // Get camera ID to use.
                cfg.Get("CAMERA_ID", CAMERA_ID);
                BOOST_LOG_SEV(logger, notification) << "CAMERA_ID : " << CAMERA_ID;

                // Get data location.
                cfg.Get("DATA_PATH", DATA_PATH);
                BOOST_LOG_SEV(logger, notification) << "DATA_PATH : " << DATA_PATH;

                // Get station name.
                cfg.Get("STATION_NAME", STATION_NAME);
                BOOST_LOG_SEV(logger, notification) << "STATION_NAME : " << STATION_NAME;

                // Get acquisition format.
                string acq_bit_depth; cfg.Get("ACQ_BIT_DEPTH", acq_bit_depth);
                EParser<CamBitDepth> cam_bit_depth;
                ACQ_BIT_DEPTH = cam_bit_depth.parseEnum("ACQ_BIT_DEPTH", acq_bit_depth);
                BOOST_LOG_SEV(logger, notification) << "ACQ_BIT_DEPTH : " << acq_bit_depth;

                // Get night exposure time.
                cfg.Get("ACQ_NIGHT_EXPOSURE", ACQ_NIGHT_EXPOSURE);
                BOOST_LOG_SEV(logger, notification) << "ACQ_NIGHT_EXPOSURE : " << ACQ_NIGHT_EXPOSURE;

                // Get night gain.
                cfg.Get("ACQ_NIGHT_GAIN", ACQ_NIGHT_GAIN);
                BOOST_LOG_SEV(logger, notification) << "ACQ_NIGHT_GAIN : " << ACQ_NIGHT_GAIN;

                // Get day option.
                cfg.Get("ACQ_DAY_ENABLED", ACQ_DAY_ENABLED);
                BOOST_LOG_SEV(logger, notification) << "ACQ_DAY_ENABLED : " << ACQ_DAY_ENABLED;

                // Get day exposure time.
                cfg.Get("ACQ_DAY_EXPOSURE", ACQ_DAY_EXPOSURE);
                BOOST_LOG_SEV(logger, notification) << "ACQ_DAY_EXPOSURE : " << ACQ_DAY_EXPOSURE;

                // Get day gain.
                cfg.Get("ACQ_DAY_GAIN", ACQ_DAY_GAIN);
                BOOST_LOG_SEV(logger, notification) << "ACQ_DAY_GAIN : " << ACQ_DAY_GAIN;

                // Get ephemeris option.
                cfg.Get("EPHEMERIS_ENABLED", EPHEMERIS_ENABLED);
                BOOST_LOG_SEV(logger, notification) << "EPHEMERIS_ENABLED : " << EPHEMERIS_ENABLED;

                // Get schedule option status.
                cfg.Get("ACQ_SCHEDULE_ENABLED", ACQ_SCHEDULE_ENABLED);
                BOOST_LOG_SEV(logger, notification) << "ACQ_SCHEDULE_ENABLED : " << ACQ_SCHEDULE_ENABLED;

                 // Get regular acquisition option status.
                cfg.Get("ACQ_REGULAR_ENABLED", ACQ_REGULAR_ENABLED);
                BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_ENABLED : " << ACQ_REGULAR_ENABLED;

                if(ACQ_SCHEDULE_ENABLED && ACQ_REGULAR_ENABLED){

                    throw "\nCheck configuration file : \n \"You can enable ACQ_SCHEDULE_ENABLED or ACQ_REGULAR_ENABLED (not both)\"\n";

                }

                // Get regular acquisition time interval.
                cfg.Get("ACQ_REGULAR_INTERVAL", ACQ_REGULAR_INTERVAL);
                BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_INTERVAL : " << ACQ_REGULAR_INTERVAL;

                // Get regular acquisition exposure time.
                cfg.Get("ACQ_REGULAR_EXPOSURE", ACQ_REGULAR_EXPOSURE);
                BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_EXPOSURE : " << ACQ_REGULAR_EXPOSURE;

                // Get regular acquisition gain.
                cfg.Get("ACQ_REGULAR_GAIN", ACQ_REGULAR_GAIN);
                BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_GAIN : " << ACQ_REGULAR_GAIN;


                // Get regular acquisition repetition.
                cfg.Get("ACQ_REGULAR_REPETITION", ACQ_REGULAR_REPETITION);
                BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_REPETITION : " << ACQ_REGULAR_REPETITION;

                // Get regular acquisition format.
                string acq_regular_format; cfg.Get("ACQ_REGULAR_FORMAT", acq_regular_format);
                EParser<CamBitDepth> cam_regular_bit_depth;
                ACQ_REGULAR_FORMAT = cam_regular_bit_depth.parseEnum("ACQ_REGULAR_FORMAT", acq_regular_format);
                BOOST_LOG_SEV(logger, notification) << "ACQ_REGULAR_FORMAT : " << acq_regular_format;

                // Get sunrise time.
                string sunrise_time;
                cfg.Get("SUNRISE_TIME", sunrise_time);
                BOOST_LOG_SEV(logger, notification) << "SUNRISE_TIME : " << sunrise_time;

                {

                    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                    boost::char_separator<char> sep(":");
                    tokenizer tokens(sunrise_time, sep);

                    for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){

                        SUNRISE_TIME.push_back(atoi((*tok_iter).c_str()));

                    }
                }

                // Get sunrise duration.
                cfg.Get("SUNRISE_DURATION", SUNRISE_DURATION);
                BOOST_LOG_SEV(logger, notification) << "SUNRISE_DURATION : " << SUNRISE_DURATION;

                {
                    // Compute start time of exposure control for sunrise.

                    float Hd_start = (SUNRISE_TIME.at(0) * 3600 + SUNRISE_TIME.at(1) * 60 - SUNRISE_DURATION)/3600.f;

                    if(Hd_start < 0) Hd_start = Hd_start + 24;

                    cout << "Hd_start : " << Hd_start<< endl;

                    SUNRISE_TIME.clear();

                    SUNRISE_TIME = TimeDate::HdecimalToHMS(Hd_start);

                    cout <<  "SUNRISE_TIME_START : " << SUNRISE_TIME.at(0) << "H" <<  SUNRISE_TIME.at(1) << endl;


                }

                // Get sunset time.
                string sunset_time;
                cfg.Get("SUNSET_TIME", sunset_time);
                BOOST_LOG_SEV(logger, notification) << "SUNSET_TIME : " << sunset_time;

                {

                    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                    boost::char_separator<char> sep(":");
                    tokenizer tokens(sunset_time, sep);

                    for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){

                        SUNSET_TIME.push_back(atoi((*tok_iter).c_str()));

                    }
                }

                 // Get sunset duration.
                cfg.Get("SUNSET_DURATION", SUNSET_DURATION);
                BOOST_LOG_SEV(logger, notification) << "SUNSET_DURATION : " << SUNSET_DURATION;

                {
                    // Compute start time of exposure control for sunset.

                    float Hd_start = (SUNSET_TIME.at(0) * 3600 + SUNSET_TIME.at(1) * 60 - SUNSET_DURATION)/3600.f;

                    if(Hd_start < 0) Hd_start = Hd_start + 24;

                    cout << "Hd_start : " << Hd_start<< endl;

                    SUNSET_TIME.clear();

                    SUNSET_TIME = TimeDate::HdecimalToHMS(Hd_start);

                    cout <<  "SUNSET_TIME_START : " << SUNSET_TIME.at(0) << "H" <<  SUNSET_TIME.at(1) << endl;

                }

                int timeStartSunrise = SUNRISE_TIME.at(0) * 3600 + SUNRISE_TIME.at(1) * 60;
                int timeStopSunrise = timeStartSunrise + SUNRISE_DURATION * 2;
                int timeStartSunset = SUNSET_TIME.at(0) * 3600 + SUNSET_TIME.at(1) * 60;
                int timeStopSunset = timeStartSunset + SUNSET_DURATION * 2;

                cout << "timeStartSunrise : " << timeStartSunrise << endl;
                cout << "timeStopSunrise : " << timeStopSunrise << endl;
                cout << "timeStartSunset : " << timeStartSunset << endl;
                cout << "timeStopSunset : " << timeStopSunset << endl;

                // Get acquisition FPS.
                cfg.Get("ACQ_FPS", ACQ_FPS);
                BOOST_LOG_SEV(logger, notification) << "ACQ_FPS : " << ACQ_FPS;
                if(ACQ_FPS <= 0) ACQ_FPS = 1;

                // Get exposure control save infos option.
                cfg.Get("EXPOSURE_CONTROL_SAVE_INFOS", EXPOSURE_CONTROL_SAVE_INFOS);
                BOOST_LOG_SEV(logger, notification) << "EXPOSURE_CONTROL_SAVE_INFOS : " << EXPOSURE_CONTROL_SAVE_INFOS;

                // Get exposure control save image option.
                cfg.Get("EXPOSURE_CONTROL_SAVE_IMAGE", EXPOSURE_CONTROL_SAVE_IMAGE);
                BOOST_LOG_SEV(logger, notification) << "EXPOSURE_CONTROL_SAVE_IMAGE : " << EXPOSURE_CONTROL_SAVE_IMAGE;

                cfg.Get("EXPOSURE_CONTROL_FREQUENCY", EXPOSURE_CONTROL_FREQUENCY);
                BOOST_LOG_SEV(logger, notification) << "EXPOSURE_CONTROL_FREQUENCY : " << EXPOSURE_CONTROL_FREQUENCY;
                EXPOSURE_CONTROL_FREQUENCY = EXPOSURE_CONTROL_FREQUENCY * ACQ_FPS;

                if(ACQ_SCHEDULE_ENABLED){

                    string	sACQ_SCHEDULE;
                    cfg.Get("ACQ_SCHEDULE", sACQ_SCHEDULE);

                    vector<string> sch1;

                    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                    boost::char_separator<char> sep(",");
                    tokenizer tokens(sACQ_SCHEDULE, sep);

                    int n = 1;
                    BOOST_LOG_SEV(logger, notification) << "SCHEDULE : ";
                    for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
                        string s = *tok_iter;
                        std::transform(s.begin(), s.end(),s.begin(), ::toupper);
                        sch1.push_back(s);
                        cout << "-> " << Conversion::intToString(n) << " - " << s << endl;
                        BOOST_LOG_SEV(logger, notification) << "-> " << Conversion::intToString(n) << " - " << s;
                        n++;
                    }

                    //23h25m00s10000000e400g12f1n
                    for(int i = 0; i < sch1.size(); i++){

                        typedef boost::tokenizer<boost::char_separator<char> > tokenizer_;
                        boost::char_separator<char> sep_("HMSEGFN");
                        tokenizer tokens_(sch1.at(i), sep_);

                        vector<string> sp;

                        for(tokenizer::iterator tok_iter_ = tokens_.begin();tok_iter_ != tokens_.end(); ++tok_iter_){

                        sp.push_back(*tok_iter_);

                        }

                        if(sp.size() == 7){

                            AcqSchedule r = AcqSchedule(atoi(sp.at(0).c_str()), atoi(sp.at(1).c_str()), atoi(sp.at(2).c_str()), atoi(sp.at(3).c_str()), atoi(sp.at(4).c_str()), atoi(sp.at(5).c_str()), atoi(sp.at(6).c_str()));

                            int scheduledTimeInSec = atoi(sp.at(0).c_str()) * 3600 + atoi(sp.at(1).c_str()) * 60 + atoi(sp.at(2).c_str());

                            // Only keep night time.
                            if((scheduledTimeInSec > timeStopSunset) || (scheduledTimeInSec < timeStartSunrise)){

                                ACQ_SCHEDULE.push_back(r);

                            }
                        }
                    }
                }

                // Get use mask option.
                cfg.Get("ACQ_MASK_ENABLED", ACQ_MASK_ENABLED);
                BOOST_LOG_SEV(logger, notification) << "ACQ_MASK_ENABLED : " << ACQ_MASK_ENABLED;

                if(ACQ_MASK_ENABLED){

                    cfg.Get("ACQ_MASK_PATH", ACQ_MASK_PATH);
                    BOOST_LOG_SEV(logger, notification) << "ACQ_MASK_PATH : " << ACQ_MASK_PATH;

                    ACQ_MASK = imread(ACQ_MASK_PATH, CV_LOAD_IMAGE_GRAYSCALE);

                    if(!ACQ_MASK.data){

                        BOOST_LOG_SEV(logger, fail) << " Can't load the mask from this location : " << ACQ_MASK_PATH;
                        throw "Failed to load the mask";

                    }

                }

                BOOST_LOG_SEV(logger, notification) << "Loading fits keywords...";
                fitsHeader.loadKeywordsFromConfigFile(cfgFile);

                runContinuousAcquisition();

		}

	}catch(exception& e){

		cout << e.what() << endl;
		return false;

	}catch(const char * msg){

		cout << msg << endl;
		return false;

	}

	return true;
}

void Device::runContinuousAcquisition(){

    /// List Gige Camera to check the ID.
    BOOST_LOG_SEV(logger, notification) << "Printing Connected Gige Camera...";
    cam->listGigeCameras();

    /// Create camera according to its ID.
    BOOST_LOG_SEV(logger, notification) << "Creating Device according ID " << CAMERA_ID << " ...";
    if(!cam->createDevice(CAMERA_ID))
        throw "Fail to create device.";

    /// Set camera format.
    BOOST_LOG_SEV(logger, notification) << "Setting acquisition format...";
    if(!cam->setPixelFormat(ACQ_BIT_DEPTH))
        throw "Fail to set Format.";


    cam->getExposureBounds(minExposureTime, maxExposureTime);

    cam->getGainBounds(minGain, maxGain);

    boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
    string date = to_iso_extended_string(time);
    cout << "date : " << date << endl;
    vector<int> intDate = TimeDate::getIntVectorFromDateString(date);

    int timeStartSunrise = SUNRISE_TIME.at(0) * 3600 + SUNRISE_TIME.at(1) * 60;
    int timeStopSunrise = timeStartSunrise + SUNRISE_DURATION * 2;
    int timeStartSunset = SUNSET_TIME.at(0) * 3600 + SUNSET_TIME.at(1) * 60;
    int timeStopSunset = timeStartSunset + SUNSET_DURATION * 2;

    cout << "timeStartSunrise : " << timeStartSunrise << endl;
    cout << "timeStopSunrise : " << timeStopSunrise << endl;
    cout << "timeStartSunset : " << timeStartSunset << endl;
    cout << "timeStopSunset : " << timeStopSunset << endl;

    int currentTimeInSec = intDate.at(3) * 3600 + intDate.at(4) * 60 + intDate.at(5);
    cout << "currentTimeInSec : " << currentTimeInSec << endl;

    // Check sunrise and sunset time.
    if((currentTimeInSec > timeStopSunrise && currentTimeInSec < timeStartSunset)){

        cout << "DAY ! "<< endl;
        /// Set camera exposure time.
        BOOST_LOG_SEV(logger, notification) << "Setting day exposure time to " << ACQ_DAY_EXPOSURE << "...";
        if(!cam->setExposureTime(ACQ_DAY_EXPOSURE))
            throw "Fail to set Night Exposure.";

        /// Set camera gain.
        BOOST_LOG_SEV(logger, notification) << "Setting day gain to " << ACQ_DAY_GAIN << "...";
        if(!cam->setGain(ACQ_DAY_GAIN))
            throw "Fail to set Night Gain.";

    }else if((currentTimeInSec > timeStopSunset) || (currentTimeInSec < timeStartSunrise) ){

        cout << "NIGHT ! "<< endl;
        /// Set camera exposure time.
        BOOST_LOG_SEV(logger, notification) << "Setting night exposure time to " << ACQ_NIGHT_EXPOSURE << "...";
        if(!cam->setExposureTime(ACQ_NIGHT_EXPOSURE))
            throw "Fail to set Day Exposure.";

        /// Set camera gain.
        BOOST_LOG_SEV(logger, notification) << "Setting night gain to " << ACQ_NIGHT_GAIN << "...";
        if(!cam->setGain(ACQ_NIGHT_GAIN))
            throw "Fail to set Day Gain.";

    }else{

        if(!cam->setExposureTime(minExposureTime))
            throw "Fail to set Day Exposure.";

        /// Set camera gain.
        BOOST_LOG_SEV(logger, notification) << "Setting night gain to " << minGain << "...";
        if(!cam->setGain(minGain))
            throw "Fail to set Day Gain.";

    }

    /// Set camera fps.
    BOOST_LOG_SEV(logger, notification) << "Setting fps...";
    if(!cam->setFPS(ACQ_FPS))
        throw "Fail to set Fps.";

    /// Prepare grabbing.
    BOOST_LOG_SEV(logger, notification) << "Preparing camera to continuous acquisition...";
    if(!cam->grabInitialization())
        throw "Fail to start grab.";

    /// Start acquisition.
    BOOST_LOG_SEV(logger, notification) << "Starting acquisition...";
    cam->acqStart();

}

bool Device::loadDataset(){

    return cam->loadNextDataSet();

}

void Device::listGigeCameras(){
	cam->listGigeCameras();
}

void Device::grabStop(){
	cam->grabCleanse();
}

bool Device::getDeviceStopStatus(){
	return cam->getStopStatus();
}

bool Device::getDatasetStatus(){
	return cam->getDataSetStatus();
}

void Device::acqStop(){
	cam->acqStop();
}

void Device::acqRestart(){
	runContinuousAcquisition();
}

bool Device::grabImage(Frame& newFrame){
	return cam->grabImage(newFrame);
}

bool Device::grabSingleImage(Frame &frame, int camID){
	return cam->grabSingleImage(frame, camID);
}

void Device::getExposureBounds(int &gMin, int &gMax){
	cam->getExposureBounds(gMin, gMax);
}

void Device::getGainBounds(int &eMin, int &eMax){
	cam->getGainBounds(eMin, eMax);
}

bool Device::getPixelFormat(CamBitDepth &format){
	return cam->getPixelFormat(format);
}

int Device::getWidth(){
	return cam->getFrameWidth();
}

int Device::getHeight(){
	return cam->getFrameHeight();
}

int Device::getFPS(){
    return cam->getFPS();
}

string Device::getModelName(){
	return cam->getModelName();
}

bool Device::setExposureTime(int exp){
	return cam->setExposureTime(exp);
}

int Device::getExposureTime(){
	return cam->getExposureTime();
}

bool Device::setGain(int gain){
	return cam->setGain(gain);
}

bool Device::setFPS(int fps){
	return cam->setFPS(fps);
}

bool Device::setPixelFormat(CamBitDepth depth){
	return cam->setPixelFormat(depth);
}
