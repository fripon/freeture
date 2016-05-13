/*
                            CfgParam.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*   This file is part of:   freeture
*
*   Copyright:      (C) 2014-2016 Yoan Audureau, Chiara Marmo
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
*   Last modified:      13/05/2016
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    CfgParam.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    13/06/2014
* \brief   Get parameters from configuration file.
*/

#include "CfgParam.h"

boost::log::sources::severity_logger< LogSeverityLevel >  CfgParam::logger;

CfgParam::Init CfgParam::initializer;

CfgParam::CfgParam(string cfgFilePath) {

    // Initialize parameters.

    showErrors = false;

    pair<int,bool> var1(-1, false);
    pair<pair<int,bool>,string> var2(var1, "");
    param.DEVICE_ID = var2;
    param.data.status = false;
    param.camInput.status = false;
    param.det.status = false;
    param.fitskeys.status = false;
    param.framesInput.status = false;
    param.log.status = false;
    param.vidInput.status = false;
    param.st.status = false;
    param.station.status = false;
    param.mail.status = false;

    param.data.DATA_PATH = "./";
    param.data.FITS_COMPRESSION = false;
    param.data.FITS_COMPRESSION_METHOD = "[compress]";

    param.log.LOG_ARCHIVE_DAY = 5;
    param.log.LOG_PATH = "./";
    param.log.LOG_SEVERITY = notification;
    param.log.LOG_SIZE_LIMIT = 50;

    vector<string> finput, vinput;
    param.framesInput.INPUT_FRAMES_DIRECTORY_PATH = finput;
    param.vidInput.INPUT_VIDEO_PATH = vinput;
    param.framesInput.INPUT_TIME_INTERVAL = 0;
    param.vidInput.INPUT_TIME_INTERVAL = 0;

    param.camInput.ACQ_DAY_EXPOSURE = 0;
    param.camInput.ACQ_DAY_GAIN = 0;
    param.camInput.ACQ_FORMAT = MONO8;
    param.camInput.ACQ_FPS = 30;
    param.camInput.ACQ_HEIGHT = 480;
    param.camInput.ACQ_NIGHT_EXPOSURE = 0;
    param.camInput.ACQ_NIGHT_GAIN = 0;
    param.camInput.ACQ_RES_CUSTOM_SIZE = false;
    param.camInput.ACQ_WIDTH = 640;
    param.camInput.ephem.EPHEMERIS_ENABLED = false;
    param.camInput.ephem.SUNRISE_DURATION = 3600;
    vector<int>sunrisetime, sunsettime;
    sunrisetime.push_back(7);
    sunrisetime.push_back(0);
    sunsettime.push_back(22);
    sunsettime.push_back(0);
    param.camInput.ephem.SUNRISE_TIME = sunrisetime;
    param.camInput.ephem.SUNSET_DURATION = 3600;
    param.camInput.ephem.SUNSET_TIME = sunsettime;
    param.camInput.EXPOSURE_CONTROL_FREQUENCY = 300;
    param.camInput.EXPOSURE_CONTROL_SAVE_IMAGE = false;
    param.camInput.EXPOSURE_CONTROL_SAVE_INFOS = false;
    param.camInput.regcap.ACQ_REGULAR_ENABLED = false;
    param.camInput.schcap.ACQ_SCHEDULE_ENABLED = false;
    param.camInput.SHIFT_BITS = false;

    param.det.ACQ_BUFFER_SIZE = 15;
    param.det.ACQ_MASK_ENABLED = false;
    param.det.DET_DEBUG = false;
    param.det.DET_DEBUG_UPDATE_MASK = false;
    param.det.DET_DOWNSAMPLE_ENABLED = true;
    param.det.DET_ENABLED = false;

    param.st.STACK_ENABLED = false;

    param.mail.MAIL_DETECTION_ENABLED = false;

    param.station.STATION_NAME = "STATION";
    param.station.SITEELEV = 0.0;
    param.station.SITELAT = 0.0;
    param.station.SITELONG = 0.0;

    // Load parameters.

    boost::filesystem::path pcfg(cfgFilePath);
    if(boost::filesystem::exists(pcfg)) {
        if(cfg.Load(cfgFilePath)) {
            loadDeviceID();
            loadDataParam();
            loadLogParam();

            if(param.DEVICE_ID.first.second) {

                // Get input type according to device number.
                Device *device = new Device();
                device->setVerbose(false);
                device->listDevices(false);
                inputType = device->getDeviceType(device->getDeviceSdk(param.DEVICE_ID.first.first));
                delete device;

                switch(inputType) {

                    case VIDEO :
                            loadVidParam();
                        break;

                    case SINGLE_FITS_FRAME :
                            loadFramesParam();
                        break;

                    // camera
                    case CAMERA :
                            loadCamParam();
                        break;

                }


            }

            loadDetParam();
            loadStackParam();
            loadStationParam();
            loadFitskeysParam();
            loadMailParam();
        }else{
            emsg.push_back("Fail to load configuration file.");
            cout << "Fail to load configuration file." << endl;
        }
    }else{
        emsg.push_back("Configuration file path not exists : " + cfgFilePath);
        cout << "Configuration file path not exists : " << cfgFilePath << endl;
    }
}

void CfgParam::loadDeviceID() {

    pair<int,bool> var1;
    pair<pair<int,bool>,string> var2;
    param.DEVICE_ID = var2;

    Device *device = new Device();
    device->setVerbose(false);
    device->listDevices(false);

    int cId;
    string cString;
    bool failIntId = false, failStringId = false;
    string failmsg = "- CAMERA_ID : ";

    if(!cfg.Get("CAMERA_ID", cId)) {
        failIntId = true;
        failmsg += "Fail to get value. Probably not defined.\n";
    }

    if(!cfg.Get("CAMERA_ID", cString)) {
        failStringId = true;
        failmsg += "Fail to get value. Probably not defined.\n";
    }else{
        try{
            EParser<CamSdkType> cam_string;
            CamSdkType cType = cam_string.parseEnum("CAMERA_ID", cString);

            if(cType == VIDEOFILE) {
                cId = device->mNbDev - 2;
            }else if(cType == FRAMESDIR){
                cId = device->mNbDev - 1;
            }else{
                failmsg += "Not correct input.\n";
                failStringId = true;
            }

        }catch (std::exception &ex) {
            failmsg += string(ex.what());
            failStringId = true;
        }
    }

    if(failIntId && failStringId) {
        param.DEVICE_ID.second = failmsg;
        delete device;
        return;
    }

    if(device->mNbDev < 0 || cId > (device->mNbDev - 1)){
        param.DEVICE_ID.second = "- CAMERA_ID's value not exist.";
        delete device;
        return;
    }

    param.DEVICE_ID.first.first = cId;
    param.DEVICE_ID.first.second = true;
    param.DEVICE_ID.second = "";

    delete device;

}

void CfgParam::loadDataParam() {

    bool e = false;

    if(!cfg.Get("DATA_PATH", param.data.DATA_PATH)) {
        param.data.errormsg.push_back("- DATA_PATH : Fail to get value.");
        e = true;
    }else{

        namespace fs = boost::filesystem;
        path p(param.data.DATA_PATH);

        if(!fs::exists(p)){
            if(!fs::create_directory(p)){
                e = true;
                param.data.errormsg.push_back("- DATA_PATH : Can't create Data Path directory.");
            }
        }
    }

    if(!cfg.Get("FITS_COMPRESSION", param.data.FITS_COMPRESSION)) {
        param.data.errormsg.push_back("- FITS_COMPRESSION : Fail to get value.");
        e = true;
    }else{

        param.data.FITS_COMPRESSION_METHOD = "";

        if(param.data.FITS_COMPRESSION) {
            if(!cfg.Get("FITS_COMPRESSION_METHOD", param.data.FITS_COMPRESSION_METHOD)) {
                param.data.errormsg.push_back("- FITS_COMPRESSION_METHOD : Fail to get value.");
                e = true;
            }
        }
    }

    if(!e) param.data.status = true;
}

void CfgParam::loadLogParam() {

    bool e = false;

    if(!cfg.Get("LOG_PATH", param.log.LOG_PATH)) {
        param.log.errormsg.push_back("- LOG_PATH : Fail to get value.");
        e = true;
    }else{

        namespace fs = boost::filesystem;
        path p(param.log.LOG_PATH);

        if(!fs::exists(p)){
            if(!fs::create_directory(p)){
                e = true;
                param.log.errormsg.push_back("- LOG_PATH : Can't create Log Path directory.");
            }
        }
    }

    if(!cfg.Get("LOG_ARCHIVE_DAY", param.log.LOG_ARCHIVE_DAY)) {
        param.log.errormsg.push_back("- LOG_ARCHIVE_DAY : Fail to get value.");
        e = true;
    }

    if(!cfg.Get("LOG_SIZE_LIMIT", param.log.LOG_SIZE_LIMIT)) {
        param.log.errormsg.push_back("- LOG_SIZE_LIMIT : Fail to get value.");
        e = true;
    }

    string log_severity;
    EParser<LogSeverityLevel> log_sev;
    if(!cfg.Get("LOG_SEVERITY", log_severity)) {
        param.log.errormsg.push_back("- LOG_SEVERITY : Fail to get value.");
        e = true;
    }

    try {
        param.log.LOG_SEVERITY = log_sev.parseEnum("LOG_SEVERITY", log_severity);
    }catch (std::exception &ex) {
        param.log.errormsg.push_back("- LOG_SEVERITY : " + string(ex.what()));
        e = true;
    }

    if(!e) param.log.status = true;
}

void CfgParam::loadFramesParam() {

    bool e = false;

    if(!cfg.Get("INPUT_TIME_INTERVAL", param.framesInput.INPUT_TIME_INTERVAL)) {
        param.framesInput.errormsg.push_back("- INPUT_TIME_INTERVAL : Fail to get value.");
        //cout << "- INPUT_FRAMES_DIRECTORY_PATH : Fail to get value." << endl;
        e = true;
    }

    string inputPaths;
    if(!cfg.Get("INPUT_FRAMES_DIRECTORY_PATH", inputPaths)) {
        param.framesInput.errormsg.push_back("- INPUT_FRAMES_DIRECTORY_PATH : Fail to get value.");
        //cout << "- INPUT_FRAMES_DIRECTORY_PATH : Fail to get value." << endl;
        e = true;
    }

    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(",");
    tokenizer tokens(inputPaths, sep);

    for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
        boost::filesystem::path p_input_frames_dir(*tok_iter);
        if(!boost::filesystem::exists(p_input_frames_dir)) {
            param.framesInput.errormsg.push_back("- INPUT_FRAMES_DIRECTORY_PATH : " + *tok_iter + " not exists.");
            e = true;
        }else{
            param.framesInput.INPUT_FRAMES_DIRECTORY_PATH.push_back(*tok_iter);
        }
    }

    if(!e) param.framesInput.status = true;
}

void CfgParam::loadVidParam() {

    bool e = false;

    if(!cfg.Get("INPUT_TIME_INTERVAL", param.vidInput.INPUT_TIME_INTERVAL)) {
        param.vidInput.errormsg.push_back("- INPUT_TIME_INTERVAL : Fail to get value.");
        //cout << "- INPUT_FRAMES_DIRECTORY_PATH : Fail to get value." << endl;
        e = true;
    }

    string input_video_path;
    if(!cfg.Get("INPUT_VIDEO_PATH", input_video_path)) {
        param.vidInput.errormsg.push_back("- INPUT_VIDEO_PATH : Fail to get value.");
        e = true;
    }

    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(",");
    tokenizer tokens(input_video_path, sep);

    for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
        boost::filesystem::path p_input_video_path(*tok_iter);
        if(!is_regular_file(p_input_video_path)) {
            param.vidInput.errormsg.push_back("- INPUT_VIDEO_PATH : " + *tok_iter + " not exists.");
            e = true;
        }else{
            param.vidInput.INPUT_VIDEO_PATH.push_back(*tok_iter);
        }
    }

    if(!e) param.vidInput.status = true;

}

void CfgParam::loadCamParam() {

    bool e = false;

    if(!param.DEVICE_ID.first.second) {

        loadDeviceID();
        if(!param.DEVICE_ID.first.second) {
            return;
        }
    }

    Device *device = new Device();
    device->setVerbose(false);
    device->listDevices(false);

    if(!device->createCamera(param.DEVICE_ID.first.first, true)) {
        delete device;
        return;
    }

    if(!cfg.Get("ACQ_FPS", param.camInput.ACQ_FPS)) {
        param.camInput.errormsg.push_back("- ACQ_FPS : Fail to get value.");
        e = true;
    }

    //-------------------------------------------------------------------

    string pixfmt;
    if(!cfg.Get("ACQ_FORMAT", pixfmt)) {
        param.camInput.errormsg.push_back("- ACQ_FORMAT : Fail to get value.");
        e = true;
    }else {
        try {
            EParser<CamPixFmt> camPixFmt;
            param.camInput.ACQ_FORMAT = camPixFmt.parseEnum("ACQ_FORMAT", pixfmt);
        }catch (std::exception &ex) {
            param.camInput.errormsg.push_back("- ACQ_FORMAT : " + string(ex.what()));
            e = true;
        }
    }

    //-------------------------------------------------------------------

    if(!cfg.Get("ACQ_RES_CUSTOM_SIZE", param.camInput.ACQ_RES_CUSTOM_SIZE)) {
        param.camInput.errormsg.push_back("- ACQ_RES_CUSTOM_SIZE : Fail to get value.");
        e = true;
    }else{

        if(param.camInput.ACQ_RES_CUSTOM_SIZE) {

            string acq_res_custome_size;
            if(!cfg.Get("ACQ_RES_SIZE", acq_res_custome_size)) {
                param.camInput.errormsg.push_back("- ACQ_RES_SIZE : Fail to get value.");
                e = true;
            }else {

                if(acq_res_custome_size.find("x") != std::string::npos) {

                    string width = acq_res_custome_size.substr(0,acq_res_custome_size.find("x"));
                    string height = acq_res_custome_size.substr(acq_res_custome_size.find("x")+1,string::npos);
                    int mSizeWidth = atoi(width.c_str());
                    int mSizeHeight = atoi(height.c_str());

                    if(mSizeHeight <= 0) {
                        param.camInput.errormsg.push_back("- ACQ_RES_SIZE : Height value is not correct.");
                        e = true;
                    }else{
                        param.camInput.ACQ_HEIGHT = mSizeHeight;
                    }

                    if(mSizeWidth <= 0) {
                        param.camInput.errormsg.push_back("- ACQ_RES_SIZE : Width value is not correct.");
                        e = true;
                    }else{
                        param.camInput.ACQ_WIDTH = mSizeWidth;
                    }

                }else {
                    param.camInput.errormsg.push_back("- ACQ_RES_SIZE : Format is not correct. It must be : WxH.");
                    e = true;
                }
            }
        }else {

            param.camInput.ACQ_HEIGHT = 480;
            param.camInput.ACQ_WIDTH = 640;

        }
    }

    //-------------------------------------------------------------------

    if(!cfg.Get("SHIFT_BITS", param.camInput.SHIFT_BITS)) {
        param.camInput.errormsg.push_back("- SHIFT_BITS : Fail to get value.");
        e = true;
    }

    int ming= -1, maxg = -1;
    double mine = -1, maxe = -1;
    device->getCameraGainBounds(ming, maxg);
    device->getCameraExposureBounds(mine, maxe);

    //-------------------------------------------------------------------

    if(!cfg.Get("ACQ_NIGHT_EXPOSURE", param.camInput.ACQ_NIGHT_EXPOSURE)) {
        param.camInput.errormsg.push_back("- ACQ_NIGHT_EXPOSURE : Fail to get value.");
        e = true;
    }else{

        if(mine != -1 && maxe != -1) {
            if(param.camInput.ACQ_NIGHT_EXPOSURE < mine || param.camInput.ACQ_NIGHT_EXPOSURE > maxe) {
                param.camInput.errormsg.push_back("- ACQ_NIGHT_EXPOSURE : Value <" +
                    Conversion::intToString(param.camInput.ACQ_NIGHT_EXPOSURE) +
                    "> is not correct. \nAvailable range is from " +
                    Conversion::intToString(mine) + " to " +
                    Conversion::intToString(maxe));
                e = true;
            }
        }
    }

    //-------------------------------------------------------------------

    if(!cfg.Get("ACQ_NIGHT_GAIN", param.camInput.ACQ_NIGHT_GAIN)) {
        param.camInput.errormsg.push_back("- ACQ_NIGHT_GAIN : Fail to get value.");
        e = true;
    }else{

        if(ming != -1 && maxg != -1) {
            if(param.camInput.ACQ_NIGHT_GAIN < ming || param.camInput.ACQ_NIGHT_GAIN > maxg) {
                param.camInput.errormsg.push_back("- ACQ_NIGHT_GAIN : Value <" +
                    Conversion::intToString(param.camInput.ACQ_NIGHT_GAIN) +
                    "> is not correct. \nAvailable range is from " +
                    Conversion::intToString(ming) + " to " +
                    Conversion::intToString(maxg));
                e = true;
            }
        }
    }

    //-------------------------------------------------------------------

    if(!cfg.Get("ACQ_DAY_EXPOSURE", param.camInput.ACQ_DAY_EXPOSURE)) {
        param.camInput.errormsg.push_back("- ACQ_DAY_EXPOSURE : Fail to get value.");
        e = true;
    }else{

        if(mine != -1 && maxe != -1) {
            if(param.camInput.ACQ_DAY_EXPOSURE < mine || param.camInput.ACQ_DAY_EXPOSURE > maxe) {
                param.camInput.errormsg.push_back("- ACQ_DAY_EXPOSURE : Value <" +
                    Conversion::intToString(param.camInput.ACQ_DAY_EXPOSURE) +
                    "> is not correct. \nAvailable range is from " +
                    Conversion::intToString(mine) + " to " +
                    Conversion::intToString(maxe));
                e = true;
            }
        }
    }

    //-------------------------------------------------------------------

    if(!cfg.Get("ACQ_DAY_GAIN", param.camInput.ACQ_DAY_GAIN)) {
        param.camInput.errormsg.push_back("- ACQ_DAY_GAIN : Fail to get value.");
        e = true;
    }else{

        if(ming != -1 && maxg != -1) {
            if(param.camInput.ACQ_DAY_GAIN < ming || param.camInput.ACQ_DAY_GAIN > maxg) {
                param.camInput.errormsg.push_back("- ACQ_DAY_GAIN : Value <" +
                    Conversion::intToString(param.camInput.ACQ_DAY_GAIN) +
                    "> is not correct. \nAvailable range is from " +
                    Conversion::intToString(ming) + " to " +
                    Conversion::intToString(maxg));
                e = true;
            }
        }
    }

    //-------------------------------------------------------------------

    if(!cfg.Get("EXPOSURE_CONTROL_FREQUENCY", param.camInput.EXPOSURE_CONTROL_FREQUENCY)) {
        param.camInput.errormsg.push_back("- EXPOSURE_CONTROL_FREQUENCY : Fail to get value.");
        e = true;
    }

    //-------------------------------------------------------------------

    if(!cfg.Get("EXPOSURE_CONTROL_SAVE_IMAGE", param.camInput.EXPOSURE_CONTROL_SAVE_IMAGE)) {
        param.camInput.errormsg.push_back("- EXPOSURE_CONTROL_SAVE_IMAGE : Fail to get value.");
        e = true;
    }

    //-------------------------------------------------------------------

    if(!cfg.Get("EXPOSURE_CONTROL_SAVE_INFOS", param.camInput.EXPOSURE_CONTROL_SAVE_INFOS)) {
        param.camInput.errormsg.push_back("- EXPOSURE_CONTROL_SAVE_INFOS : Fail to get value.");
        e = true;
    }

    //-------------------------------------------------------------------

    if(!cfg.Get("EPHEMERIS_ENABLED", param.camInput.ephem.EPHEMERIS_ENABLED)) {
        param.camInput.errormsg.push_back("- EPHEMERIS_ENABLED : Fail to get value.");
        e = true;
    }

    //-------------------------------------------------------------------
    if(param.camInput.ephem.EPHEMERIS_ENABLED) {
        if(!cfg.Get("SUN_HORIZON_1", param.camInput.ephem.SUN_HORIZON_1)) {
            param.camInput.errormsg.push_back("- SUN_HORIZON_1 : Fail to get value.");
            e = true;
        }

        //-------------------------------------------------------------------

        if(!cfg.Get("SUN_HORIZON_2", param.camInput.ephem.SUN_HORIZON_2)) {
            param.camInput.errormsg.push_back("- SUN_HORIZON_2 : Fail to get value.");
            e = true;
        }
    }

    //-------------------------------------------------------------------

    if(!param.camInput.ephem.EPHEMERIS_ENABLED) {

        string sunrise_time;
        if(!cfg.Get("SUNRISE_TIME", sunrise_time)) {
            param.camInput.errormsg.push_back("- SUNRISE_TIME : Fail to get value.");
            e = true;
        }else{

            if(sunrise_time.find(":") != std::string::npos) {

                typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                boost::char_separator<char> sep(":");
                tokenizer tokens(sunrise_time, sep);

                param.camInput.ephem.SUNRISE_TIME.clear();
                for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
                    param.camInput.ephem.SUNRISE_TIME.push_back(atoi((*tok_iter).c_str()));
                }

                if(param.camInput.ephem.SUNRISE_TIME.size() == 2) {
                    if(param.camInput.ephem.SUNRISE_TIME.at(0) < 0 || param.camInput.ephem.SUNRISE_TIME.at(0) >= 24) {
                        param.camInput.errormsg.push_back("- SUNRISE_TIME : Hours value must be between 0 - 23");
                        e = true;
                    }

                    if(param.camInput.ephem.SUNRISE_TIME.at(1) < 0 || param.camInput.ephem.SUNRISE_TIME.at(0) >= 60) {
                        param.camInput.errormsg.push_back("- SUNRISE_TIME : Minutes value must be between 0 - 59");
                        e = true;
                    }
                }

            }else {
                param.camInput.errormsg.push_back("- SUNRISE_TIME : Format is not correct. It must be : HH:MM");
                e = true;
            }
        }

        //-------------------------------------------------------------------

        string sunset_time;
        if(!cfg.Get("SUNSET_TIME", sunset_time)) {
            param.camInput.errormsg.push_back("- SUNSET_TIME : Fail to get value.");
            e = true;
        }else{

            if(sunset_time.find(":") != std::string::npos) {

                typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                boost::char_separator<char> sep(":");
                tokenizer tokens(sunset_time, sep);

                param.camInput.ephem.SUNSET_TIME.clear();
                for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter)
                    param.camInput.ephem.SUNSET_TIME.push_back(atoi((*tok_iter).c_str()));

                if(param.camInput.ephem.SUNSET_TIME.size() == 2) {
                    if(param.camInput.ephem.SUNSET_TIME.at(0) < 0 || param.camInput.ephem.SUNSET_TIME.at(0) >= 24) {
                        param.camInput.errormsg.push_back("- SUNSET_TIME : Hours value must be between 0 - 23");
                        e = true;
                    }

                    if(param.camInput.ephem.SUNSET_TIME.at(1) < 0 || param.camInput.ephem.SUNSET_TIME.at(0) >= 60) {
                        param.camInput.errormsg.push_back("- SUNSET_TIME : Minutes value must be between 0 - 59");
                        e = true;
                    }
                }

            }else {
                param.camInput.errormsg.push_back("- SUNSET_TIME : Format is not correct. It must be : HH:MM");
                e = true;
            }
        }

        //-------------------------------------------------------------------

        if(!cfg.Get("SUNSET_DURATION", param.camInput.ephem.SUNSET_DURATION)) {
            param.camInput.errormsg.push_back("- SUNSET_DURATION : Fail to get value.");
            e = true;
        }

        //-------------------------------------------------------------------

        if(!cfg.Get("SUNRISE_DURATION", param.camInput.ephem.SUNRISE_DURATION)) {
            param.camInput.errormsg.push_back("- SUNRISE_DURATION : Fail to get value.");
            e = true;
        }
    }

    //-------------------------------------------------------------------

    if(!cfg.Get("ACQ_REGULAR_ENABLED", param.camInput.regcap.ACQ_REGULAR_ENABLED)) {
        param.camInput.errormsg.push_back("- ACQ_REGULAR_ENABLED : Fail to get value.");
        e = true;
    }else {
        if(param.camInput.regcap.ACQ_REGULAR_ENABLED) {
            //-------------------------------------------------------------------

            string reg_mode;
            if(!cfg.Get("ACQ_REGULAR_MODE", reg_mode)) {
                e = true;
                param.camInput.errormsg.push_back("- ACQ_REGULAR_MODE : Fail to load value.");
            }else {
                try {
                    EParser<TimeMode> regMode;
                    param.camInput.regcap.ACQ_REGULAR_MODE = regMode.parseEnum("ACQ_REGULAR_MODE", reg_mode);
                }catch (std::exception &ex) {
                    e = true;
                    param.camInput.errormsg.push_back("- ACQ_REGULAR_MODE : " + string(ex.what()));
                }
            }

            //-------------------------------------------------------------------

            {

                string img_output;
                if(!cfg.Get("ACQ_REGULAR_OUTPUT", img_output)) {
                    e = true;
                    param.camInput.errormsg.push_back("- ACQ_REGULAR_OUTPUT : Fail to load value.");
                }else {
                    try {
                        EParser<ImgFormat> imgOutput;
                        param.camInput.regcap.ACQ_REGULAR_OUTPUT = imgOutput.parseEnum("ACQ_REGULAR_OUTPUT", img_output);
                    }catch (std::exception &ex) {
                        e = true;
                        param.camInput.errormsg.push_back("- ACQ_REGULAR_OUTPUT : " + string(ex.what()));
                    }
                }
            }

            //-------------------------------------------------------------------

            string regAcqParam;
            if(!cfg.Get("ACQ_REGULAR_CFG", regAcqParam)) {
                e = true;
                param.camInput.errormsg.push_back("- ACQ_REGULAR_CFG : Fail to load value.");
            }else {
                std::transform(regAcqParam.begin(), regAcqParam.end(),regAcqParam.begin(), ::toupper);

                typedef boost::tokenizer<boost::char_separator<char> > tokenizer1;
                boost::char_separator<char> sep1("HMSEGFN");
                tokenizer1 tokens1(regAcqParam, sep1);

                vector<string> res1;
                for(tokenizer1::iterator tokIter = tokens1.begin();tokIter != tokens1.end(); ++tokIter)
                    res1.push_back(*tokIter);

                if(res1.size() == 7) {

                    // Get regular acquisition time interval.
                    if(atoi(res1.at(0).c_str())< 0 || atoi(res1.at(0).c_str()) >= 24) {
                        e = true;
                        param.camInput.errormsg.push_back("- ACQ_REGULAR_CFG : Hours can't have the value <" + res1.at(0) + ">.\nAvailable range is from 0 to 23.");
                    }

                    if(atoi(res1.at(1).c_str())< 0 || atoi(res1.at(1).c_str()) >= 60) {
                        e = true;
                        param.camInput.errormsg.push_back("- ACQ_REGULAR_CFG : Minutes can't have the value <" + res1.at(1) + ">.\nAvailable range is from 0 to 23.");
                    }

                    if(atoi(res1.at(2).c_str())< 0 || atoi(res1.at(2).c_str()) >= 60) {
                        e = true;
                        param.camInput.errormsg.push_back("- ACQ_REGULAR_CFG : Seconds can't have the value <" + res1.at(2) + ">.\nAvailable range is from 0 to 23.");
                    }

                    param.camInput.regcap.ACQ_REGULAR_CFG.interval = atoi(res1.at(0).c_str()) * 3600 + atoi(res1.at(1).c_str()) * 60 + atoi(res1.at(2).c_str());

                    // Get regular acquisition exposure time.
                    param.camInput.regcap.ACQ_REGULAR_CFG.exp = atoi(res1.at(3).c_str());

                    if(mine != -1 && maxe != -1) {
                        if(param.camInput.regcap.ACQ_REGULAR_CFG.exp < mine || param.camInput.regcap.ACQ_REGULAR_CFG.exp > maxe) {
                            param.camInput.errormsg.push_back("- ACQ_REGULAR_CFG : Exposure value <" +
                                Conversion::intToString(param.camInput.regcap.ACQ_REGULAR_CFG.exp) +
                                "> is not correct. \nAvailable range is from " +
                                Conversion::intToString(mine) + " to " +
                                Conversion::intToString(maxe));
                            e = true;
                        }
                    }

                    // Get regular acquisition gain.
                    param.camInput.regcap.ACQ_REGULAR_CFG.gain = atoi(res1.at(4).c_str());

                    if(ming != -1 && maxg != -1) {
                        if(param.camInput.regcap.ACQ_REGULAR_CFG.gain < ming || param.camInput.regcap.ACQ_REGULAR_CFG.gain > maxg) {
                            param.camInput.errormsg.push_back("- ACQ_REGULAR_CFG : Gain value <" +
                                Conversion::intToString(param.camInput.regcap.ACQ_REGULAR_CFG.gain) +
                                "> is not correct. \nAvailable range is from " +
                                Conversion::intToString(ming) + " to " +
                                Conversion::intToString(maxg));
                            e = true;
                        }
                    }

                    // Get regular acquisition repetition.
                    param.camInput.regcap.ACQ_REGULAR_CFG.rep = atoi(res1.at(6).c_str());

                    // Get regular acquisition format.
                    param.camInput.regcap.ACQ_REGULAR_CFG.fmt = static_cast<CamPixFmt>(atoi(res1.at(5).c_str()));
                    EParser<CamPixFmt> fmt;
                    if(fmt.getStringEnum(param.camInput.regcap.ACQ_REGULAR_CFG.fmt) == ""){
                        e = true;
                        param.camInput.errormsg.push_back("- ACQ_REGULAR_CFG : Fail to extract pixel format on " + regAcqParam + ". Check if index <" + res1.at(5) + "> exits.");
                    }
                }
            }
        }
    }

    //-------------------------------------------------------------------

    if(!cfg.Get("ACQ_SCHEDULE_ENABLED", param.camInput.schcap.ACQ_SCHEDULE_ENABLED)) {
        param.camInput.errormsg.push_back("- ACQ_SCHEDULE_ENABLED : Fail to get value.");
        e = true;
    }else{

        if(param.camInput.schcap.ACQ_SCHEDULE_ENABLED) {

            if(!param.camInput.regcap.ACQ_REGULAR_ENABLED) {
                //-------------------------------------------------------------------

                {
                    string img_output;
                    if(!cfg.Get("ACQ_SCHEDULE_OUTPUT", img_output)) {
                        e = true;
                        param.camInput.errormsg.push_back("- ACQ_SCHEDULE_OUTPUT : Fail to load value.");
                    }else {
                        try {
                            EParser<ImgFormat> imgOutput;
                            param.camInput.schcap.ACQ_SCHEDULE_OUTPUT = imgOutput.parseEnum("ACQ_SCHEDULE_OUTPUT", img_output);
                        }catch (std::exception &ex) {
                            e = true;
                            param.camInput.errormsg.push_back("- ACQ_SCHEDULE_OUTPUT : " + string(ex.what()));
                        }
                    }
                }

                //-------------------------------------------------------------------

                {
                    string sACQ_SCHEDULE;
                    if(!cfg.Get("ACQ_SCHEDULE", sACQ_SCHEDULE)) {
                        e = true;
                        param.camInput.errormsg.push_back("- ACQ_SCHEDULE : Fail to load value.");
                    }else {

                        vector<string> sch1;

                        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                        boost::char_separator<char> sep(",");
                        tokenizer tokens(sACQ_SCHEDULE, sep);

                        for(tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter) {
                            string s = *tok_iter;
                            std::transform(s.begin(), s.end(),s.begin(), ::toupper);
                            sch1.push_back(s);
                        }

                        for(int i = 0; i < sch1.size(); i++) {

                            typedef boost::tokenizer<boost::char_separator<char> > tokenizer_;
                            boost::char_separator<char> sep_("HMSEGFN");
                            tokenizer tokens_(sch1.at(i), sep_);

                            vector<string> sp;

                            for(tokenizer::iterator tok_iter_ = tokens_.begin();tok_iter_ != tokens_.end(); ++tok_iter_)
                                sp.push_back(*tok_iter_);

                            if(sp.size() == 7) {

                                scheduleParam spa;
                                bool status = true;
                                spa.hours = atoi(sp.at(0).c_str());
                                if(spa.hours< 0 || spa.hours >= 24) {
                                    e = true;
                                    status = false;
                                    param.camInput.errormsg.push_back("- ACQ_SCHEDULE : In " + sch1.at(i) + ". Hours can't have the value <" + Conversion::intToString(spa.hours) + ">.\nAvailable range is from 0 to 23.");
                                }
                                spa.min = atoi(sp.at(1).c_str());
                                if(spa.min< 0 || spa.min >= 60) {
                                    e = true;
                                    status = false;
                                    param.camInput.errormsg.push_back("- ACQ_SCHEDULE : In " + sch1.at(i) + ". Minutes can't have the value <" + Conversion::intToString(spa.min) + ">.\nAvailable range is from 0 to 59.");
                                }
                                spa.sec = atoi(sp.at(2).c_str());
                                if(spa.sec< 0 || spa.sec >= 60) {
                                    e = true;
                                    status = false;
                                    param.camInput.errormsg.push_back("- ACQ_SCHEDULE : In " + sch1.at(i) + ". Seconds can't have the value <" + Conversion::intToString(spa.sec) + ">.\nAvailable range is from 0 to 59.");
                                }
                                spa.exp = atoi(sp.at(3).c_str());

                                if(mine != -1 && maxe != -1) {
                                    if(spa.exp < mine || spa.exp > maxe) {
                                        param.camInput.errormsg.push_back("- ACQ_SCHEDULE : In " + sch1.at(i) + ". Exposure value <" +
                                            Conversion::intToString(spa.exp) +
                                            "> is not correct. \nAvailable range is from " +
                                            Conversion::intToString(mine) + " to " +
                                            Conversion::intToString(maxe));
                                        e = true;
                                        status = false;
                                    }
                                }

                                spa.gain = atoi(sp.at(4).c_str());

                                if(ming != -1 && maxg != -1) {
                                    if(spa.gain < ming || spa.gain > maxg) {
                                        param.camInput.errormsg.push_back("- ACQ_SCHEDULE : In " + sch1.at(i) + ". Gain value <" +
                                            Conversion::intToString(spa.gain) +
                                            "> is not correct. \nAvailable range is from " +
                                            Conversion::intToString(ming) + " to " +
                                            Conversion::intToString(maxg));
                                        e = true;
                                        status = false;
                                    }
                                }

                                spa.rep = atoi(sp.at(6).c_str());
                                if(spa.rep< 0 || spa.rep >= 60) {
                                    e = true;
                                    status = false;
                                    param.camInput.errormsg.push_back("- ACQ_SCHEDULE : One repetition must be defined at least.");
                                }
                                spa.fmt = static_cast<CamPixFmt>(atoi(sp.at(5).c_str()));
                                EParser<CamPixFmt> fmt;
                                if(fmt.getStringEnum(spa.fmt) == ""){
                                    e = true;
                                    status = false;
                                    param.camInput.errormsg.push_back("- ACQ_SCHEDULE : Fail to extract pixel format for :  " + sch1.at(i) + ". Index <" + sp.at(5) + "> not exist.");
                                }

                                if(status)
                                    param.camInput.schcap.ACQ_SCHEDULE.push_back(spa);

                            }
                        }

                        // Order scheduled acquisition

                        if(param.camInput.schcap.ACQ_SCHEDULE.size() != 0){

                            // Sort time in list.
                            vector<scheduleParam> tempSchedule;

                            do{

                                int minH; int minM; int minS; bool init = false;

                                vector<scheduleParam>::iterator it;
                                vector<scheduleParam>::iterator it_select;

                                for(it = param.camInput.schcap.ACQ_SCHEDULE.begin(); it != param.camInput.schcap.ACQ_SCHEDULE.end(); ++it){

                                    if(!init){

                                        minH = (*it).hours;
                                        minM = (*it).min;
                                        minS = (*it).sec;
                                        it_select = it;
                                        init = true;

                                    }else{

                                        if((*it).hours < minH){

                                            minH = (*it).hours;
                                            minM = (*it).min;
                                            minS = (*it).sec;
                                            it_select = it;

                                        }else if((*it).hours == minH){

                                            if((*it).min < minM){

                                                minH = (*it).hours;
                                                minM = (*it).min;
                                                minS = (*it).sec;
                                                it_select = it;

                                            }else if((*it).min == minM){

                                                if((*it).sec < minS){

                                                    minH = (*it).hours;
                                                    minM = (*it).min;
                                                    minS = (*it).sec;
                                                    it_select = it;

                                                }

                                            }

                                        }

                                    }

                                }

                                if(init){

                                    tempSchedule.push_back((*it_select));
                                    //cout << "-> " << (*it_select).hours << "H " << (*it_select).min << "M " << (*it_select).sec << "S " << endl;
                                    param.camInput.schcap.ACQ_SCHEDULE.erase(it_select);

                                }

                            }while(param.camInput.schcap.ACQ_SCHEDULE.size() != 0);

                            param.camInput.schcap.ACQ_SCHEDULE = tempSchedule;

                        }
                    }
                }

            }else{
                e = true;
                param.camInput.errormsg.push_back("- ACQ_SCHEDULE : Disable ACQ_REGULAR_ENABLED to use ACQ_SCHEDULE.");
            }
        }
    }

    if(device != NULL) {
        delete device;
    }

    if(!e) param.camInput.status = true;
}

void CfgParam::loadDetParam() {

    bool e = false;

    if(!cfg.Get("ACQ_BUFFER_SIZE", param.det.ACQ_BUFFER_SIZE)) {
        e = true;
        param.det.errormsg.push_back("- ACQ_BUFFER_SIZE : Fail to load value.");
    }

    if(!cfg.Get("ACQ_MASK_ENABLED", param.det.ACQ_MASK_ENABLED)) {
        e = true;
        param.det.errormsg.push_back("- ACQ_MASK_ENABLED : Fail to load value.");
    }else{

        if(param.det.ACQ_MASK_ENABLED) {

            if(!cfg.Get("ACQ_MASK_PATH", param.det.ACQ_MASK_PATH)) {
                e = true;
                param.det.errormsg.push_back("- ACQ_MASK_PATH : Fail to load value.");
            }else {
                Mat tempmask = imread(param.det.ACQ_MASK_PATH, CV_LOAD_IMAGE_GRAYSCALE);

                if(!tempmask.data) {
                    e = true;
                    param.det.errormsg.push_back("- MASK : Fail to load the mask image. No data.");
                    // Add test to compare mask size to a capture from camera or video or frame file
                }else {

                    tempmask.copyTo(param.det.MASK);

                    if(param.DEVICE_ID.first.second) {

                        Device *device = new Device();
                        device->setVerbose(false);
                        device->listDevices(false);
                        inputType = device->getDeviceType(device->getDeviceSdk(param.DEVICE_ID.first.first));

                        switch(inputType) {

                            case VIDEO :
                                {
                                    if(vidParamIsCorrect()) {

                                        for(int i = 0; i < param.vidInput.INPUT_VIDEO_PATH.size(); i++) {
                                            VideoCapture cap = VideoCapture(param.vidInput.INPUT_VIDEO_PATH.at(i));
                                            if(cap.isOpened()) {
                                                if(cap.get(CV_CAP_PROP_FRAME_HEIGHT) != tempmask.rows) {
                                                    e = true;
                                                    param.det.errormsg.push_back("- ACQ_MASK_PATH : Mask's height (" +
                                                        Conversion::intToString(tempmask.rows) +
                                                        ") is not correct with " + param.vidInput.INPUT_VIDEO_PATH.at(i) + " (" +
                                                        Conversion::intToString(cap.get(CV_CAP_PROP_FRAME_HEIGHT)) + ")");
                                                }

                                                if(cap.get(CV_CAP_PROP_FRAME_WIDTH) != tempmask.cols) {
                                                    e = true;
                                                    param.det.errormsg.push_back("- ACQ_MASK_PATH : Mask's width (" +
                                                        Conversion::intToString(tempmask.cols) +
                                                        ") is not correct with " + param.vidInput.INPUT_VIDEO_PATH.at(i) + " (" +
                                                        Conversion::intToString(cap.get(CV_CAP_PROP_FRAME_WIDTH)) + ")");
                                                }
                                            }else{
                                                e = true;
                                                param.det.errormsg.push_back("- ACQ_MASK_PATH : Check mask's size. Fail to open " + param.vidInput.INPUT_VIDEO_PATH.at(i));
                                            }
                                        }

                                    }else{
                                        e = true;
                                        param.det.errormsg.push_back("- ACQ_MASK_PATH : Check mask's size. Video parameters loading failed.");
                                    }
                                }
                                break;

                            case SINGLE_FITS_FRAME :

                                {
                                    if(framesParamIsCorrect()) {
                                        for(int i = 0; i < param.framesInput.INPUT_FRAMES_DIRECTORY_PATH.size(); i++) {
                                            // Search a fits file.
                                            bool fitsfilefound = false;
                                            string filefound = "";
                                            path p(param.framesInput.INPUT_FRAMES_DIRECTORY_PATH.at(i));
                                            for(directory_iterator file(p);file!= directory_iterator(); ++file){
                                                path curr(file->path());
                                                if(is_regular_file(curr)) {
                                                    if(file->path().string().find(".fit") != std::string::npos) {
                                                        fitsfilefound = true;
                                                        filefound = file->path().string();
                                                        break;
                                                    }
                                                }
                                            }

                                            if(fitsfilefound) {
                                                Fits2D f(filefound);
                                                int h = 0,w = 0;

                                                if(!f.readIntKeyword("NAXIS1", w)){
                                                    e = true;
                                                    param.det.errormsg.push_back("- ACQ_MASK_PATH : Check mask's size. Fail to read NAXIS1. " + param.framesInput.INPUT_FRAMES_DIRECTORY_PATH.at(i));
                                                }

                                                if(!f.readIntKeyword("NAXIS2", h)){
                                                    e = true;
                                                    param.det.errormsg.push_back("- ACQ_MASK_PATH : Check mask's size. Fail to read NAXIS2. " + param.framesInput.INPUT_FRAMES_DIRECTORY_PATH.at(i));
                                                }

                                                if(h!=0 && w!=0) {

                                                    if(h != tempmask.rows) {
                                                        e = true;
                                                        param.det.errormsg.push_back("- ACQ_MASK_PATH : Mask's height (" +
                                                            Conversion::intToString(tempmask.rows) +
                                                            ") is not correct with " + param.framesInput.INPUT_FRAMES_DIRECTORY_PATH.at(i) + " (" +
                                                            Conversion::intToString(h) + ")");
                                                    }

                                                    if(w != tempmask.cols) {
                                                        e = true;
                                                        param.det.errormsg.push_back("- ACQ_MASK_PATH : Mask's width (" +
                                                            Conversion::intToString(tempmask.cols) +
                                                            ") is not correct with " + param.framesInput.INPUT_FRAMES_DIRECTORY_PATH.at(i) + " (" +
                                                            Conversion::intToString(w) + ")");
                                                    }
                                                }

                                            }else{
                                                e = true;
                                                param.det.errormsg.push_back("- ACQ_MASK_PATH : Check mask's size. No fits file found in " + param.framesInput.INPUT_FRAMES_DIRECTORY_PATH.at(i));
                                            }
                                        }
                                    }
                                }

                                break;

                            case CAMERA :
                                {
                                    /*if(camParamIsCorrect()) {

                                    }*/
                                }
                                break;

                            default :
                                e = true;
                                param.det.errormsg.push_back("- ACQ_MASK_PATH : Fail to create device to check mask's size.");

                        }

                        delete device;

                    }else {
                        e = true;
                        param.det.errormsg.push_back("- ACQ_MASK_PATH : Fail to create device to check mask's size. CAMERA_ID not loaded.");
                    }
                }
            }
        }
    }

    if(!cfg.Get("DET_ENABLED", param.det.DET_ENABLED)) {
        e = true;
        param.det.errormsg.push_back("- DET_ENABLED : Fail to load value.");
    }

    string det_mode;
    if(!cfg.Get("DET_MODE", det_mode)) {
        e = true;
        param.det.errormsg.push_back("- DET_MODE : Fail to load value.");
    }else {
        try {
            EParser<TimeMode> detMode;
            param.det.DET_MODE = detMode.parseEnum("DET_MODE", det_mode);
        }catch (std::exception &ex) {
            e = true;
            param.det.errormsg.push_back("- DET_MODE : " + string(ex.what()));
        }
    }

    if(!cfg.Get("DET_DEBUG", param.det.DET_DEBUG)) {
        e = true;
        param.det.errormsg.push_back("- DET_DEBUG : Fail to load value.");
    }else{

        if(param.det.DET_DEBUG){

            if(!cfg.Get("DET_DEBUG_PATH", param.det.DET_DEBUG_PATH)) {
                e = true;
                param.det.errormsg.push_back("- DET_DEBUG_PATH : Fail to load value.");
            }else{

                namespace fs = boost::filesystem;
                path p(param.det.DET_DEBUG_PATH);

                if(!fs::exists(p)){
                    if(!fs::create_directory(p)){
                        e = true;
                        param.det.errormsg.push_back("- DET_DEBUG_PATH : Can't create Debug Path.");
                    }
                }
            }
        }
    }

    if(!cfg.Get("DET_TIME_AROUND", param.det.DET_TIME_AROUND)) {
        e = true;
        param.det.errormsg.push_back("- DET_TIME_AROUND : Fail to load value.");
    }

    if(!cfg.Get("DET_TIME_MAX", param.det.DET_TIME_MAX)) {
        e = true;
        param.det.errormsg.push_back("- DET_TIME_MAX : Fail to load value.");
    }else{

        // If input device type is frames or video, increase DET_TIME_MAX because
        // time can not be take account as the time interval between can be increased.
        if(inputType == VIDEO || inputType == SINGLE_FITS_FRAME) {
            param.det.DET_TIME_MAX = 10000;
        }else{
            if(param.det.DET_TIME_MAX <= 0 || param.det.DET_TIME_MAX > 30) {
                e = true;
                param.det.errormsg.push_back("- DET_TIME_MAX : Available range is from 1 to 30 seconds.");
            }
        }
    }

    string det_mthd;
    if(!cfg.Get("DET_METHOD", det_mthd)) {
        e = true;
        param.det.errormsg.push_back("- DET_METHOD : Fail to load value.");
    }else {
        try {
            EParser<DetMeth> detMthd;
            param.det.DET_METHOD = detMthd.parseEnum("DET_METHOD", det_mthd);
        }catch (std::exception &ex) {
            e = true;
            param.st.errormsg.push_back("- DET_METHOD : " + string(ex.what()));
        }
    }

    if(!cfg.Get("DET_SAVE_FITS3D", param.det.DET_SAVE_FITS3D)) {
        e = true;
        param.det.errormsg.push_back("- DET_SAVE_FITS3D : Fail to load value.");
    }

    if(!cfg.Get("DET_SAVE_FITS2D", param.det.DET_SAVE_FITS2D)) {
        e = true;
        param.det.errormsg.push_back("- DET_SAVE_FITS2D : Fail to load value.");
    }

    if(!cfg.Get("DET_SAVE_SUM", param.det.DET_SAVE_SUM)) {
        e = true;
        param.det.errormsg.push_back("- DET_SAVE_SUM : Fail to load value.");
    }

    if(!cfg.Get("DET_SUM_REDUCTION", param.det.DET_SUM_REDUCTION)) {
        e = true;
        param.det.errormsg.push_back("- DET_SUM_REDUCTION : Fail to load value.");
    }

    string det_sum_mthd;
    if(!cfg.Get("DET_SUM_MTHD", det_sum_mthd)) {
        e = true;
        param.det.errormsg.push_back("- DET_SUM_MTHD : Fail to load value.");
    }else {
        try {
            EParser<StackMeth> detSumMthd;
            param.det.DET_SUM_MTHD = detSumMthd.parseEnum("DET_SUM_MTHD", det_sum_mthd);
        }catch (std::exception &ex) {
            e = true;
            param.det.errormsg.push_back("- DET_SUM_MTHD : " + string(ex.what()));
        }
    }

    if(!cfg.Get("DET_SAVE_SUM_WITH_HIST_EQUALIZATION", param.det.DET_SAVE_SUM_WITH_HIST_EQUALIZATION)) {
        e = true;
        param.det.errormsg.push_back("- DET_SAVE_SUM_WITH_HIST_EQUALIZATION : Fail to load value.");
    }

    if(!cfg.Get("DET_SAVE_AVI", param.det.DET_SAVE_AVI)) {
        e = true;
        param.det.errormsg.push_back("- DET_SAVE_AVI : Fail to load value.");
    }

    if(!cfg.Get("DET_UPDATE_MASK", param.det.DET_UPDATE_MASK)) {
        e = true;
        param.det.errormsg.push_back("- DET_UPDATE_MASK : Fail to load value.");
    }

    if(!cfg.Get("DET_UPDATE_MASK_FREQUENCY", param.det.DET_UPDATE_MASK_FREQUENCY)) {
        e = true;
        param.det.errormsg.push_back("- DET_UPDATE_MASK_FREQUENCY : Fail to load value.");
    }

    if(!cfg.Get("DET_DEBUG_UPDATE_MASK", param.det.DET_DEBUG_UPDATE_MASK)) {
        e = true;
        param.det.errormsg.push_back("- DET_DEBUG_UPDATE_MASK : Fail to load value.");
    }else{

        if(param.det.DET_DEBUG_UPDATE_MASK){

            if(!cfg.Get("DET_DEBUG_PATH", param.det.DET_DEBUG_PATH)) {
                e = true;
                param.det.errormsg.push_back("- DET_DEBUG_PATH : Fail to load value.");
            }else{

                namespace fs = boost::filesystem;
                path p(param.det.DET_DEBUG_PATH);

                if(!fs::exists(p)){
                    if(!fs::create_directory(p)){
                        e = true;
                        param.det.errormsg.push_back("- DET_DEBUG_PATH : Can't create Debug Path. Debug Path must exist as DET_DEBUG_UPDATE_MASK is enabled.");
                    }
                }
            }
        }
    }

    // --------------------------------------------------------------------------------------

    if(!cfg.Get("DET_DOWNSAMPLE_ENABLED", param.det.DET_DOWNSAMPLE_ENABLED)) {
        e = true;
        param.det.errormsg.push_back("- DET_DOWNSAMPLE_ENABLED : Fail to load value.");
    }

    if(!cfg.Get("DET_SAVE_GEMAP", param.det.temporal.DET_SAVE_GEMAP)) {
        e = true;
        param.det.errormsg.push_back("- DET_SAVE_GEMAP : Fail to load value.");
    }

    if(!cfg.Get("DET_SAVE_DIRMAP", param.det.temporal.DET_SAVE_DIRMAP)) {
        e = true;
        param.det.errormsg.push_back("- DET_SAVE_DIRMAP : Fail to load value.");
    }

    if(!cfg.Get("DET_SAVE_POS", param.det.temporal.DET_SAVE_POS)) {
        e = true;
        param.det.errormsg.push_back("- DET_SAVE_POS : Fail to load value.");
    }

    if(!cfg.Get("DET_LE_MAX", param.det.temporal.DET_LE_MAX)) {
        e = true;
        param.det.errormsg.push_back("- DET_LE_MAX : Fail to load value.");
    }else{

        if(param.det.temporal.DET_LE_MAX < 1 || param.det.temporal.DET_LE_MAX > 10) {

            e = true;
            param.det.errormsg.push_back("- DET_LE_MAX : Available range is from 1 to 10.");

        }

    }

    if(!cfg.Get("DET_GE_MAX", param.det.temporal.DET_GE_MAX)) {
        e = true;
        param.det.errormsg.push_back("- DET_GE_MAX : Fail to load value.");
    }else{

        if(param.det.temporal.DET_GE_MAX < 1 || param.det.temporal.DET_GE_MAX > 10) {

            e = true;
            param.det.errormsg.push_back("- DET_GE_MAX : Available range is from 1 to 10.");

        }

    }

    /*if(!cfg.Get("DET_SAVE_GE_INFOS", param.det.temporal.DET_SAVE_GE_INFOS)) {
        e = true;
        param.det.errormsg.push_back("- DET_SAVE_GE_INFOS : Fail to load value.");
    }*/

    if(!e) param.det.status = true;

}

void CfgParam::loadStackParam() {

    bool e = false;

    if(!cfg.Get("STACK_ENABLED", param.st.STACK_ENABLED)) {
        e = true;
        param.st.errormsg.push_back("- STACK_ENABLED : Fail to load value.");
    }

    string stack_mode;
    if(!cfg.Get("STACK_MODE", stack_mode)) {
        e = true;
        param.st.errormsg.push_back("- STACK_MODE : Fail to load value.");
    }else {
        try {
            EParser<TimeMode> stackMode;
            param.st.STACK_MODE = stackMode.parseEnum("STACK_MODE", stack_mode);
        }catch (std::exception &ex) {
            e = true;
            param.st.errormsg.push_back("- STACK_MODE : " + string(ex.what()));
        }
    }

    if(!cfg.Get("STACK_TIME", param.st.STACK_TIME)) {
        e = true;
        param.st.errormsg.push_back("- STACK_TIME : Fail to load value.");
    }

    if(!cfg.Get("STACK_INTERVAL", param.st.STACK_INTERVAL)) {
        e = true;
        param.st.errormsg.push_back("- STACK_INTERVAL : Fail to load value.");
    }

    string stack_mthd;
    if(!cfg.Get("STACK_MTHD", stack_mthd)) {
        e = true;
        param.st.errormsg.push_back("- STACK_MTHD : Fail to load value.");
    }else {
        try {
            EParser<StackMeth> stackMthd;
            param.st.STACK_MTHD = stackMthd.parseEnum("STACK_MTHD", stack_mthd);
        }catch (std::exception &ex) {
            e = true;
            param.st.errormsg.push_back("- STACK_MTHD : " + string(ex.what()));
        }
    }

    if(!cfg.Get("STACK_REDUCTION", param.st.STACK_REDUCTION)) {
        e = true;
        param.st.errormsg.push_back("- STACK_REDUCTION : Fail to load value.");
    }

    if(!e) param.st.status = true;

}

void CfgParam::loadStationParam() {

    bool e = false;

    if(!cfg.Get("STATION_NAME", param.station.STATION_NAME)) {
        e = true;
        param.station.errormsg.push_back("- STATION_NAME : Fail to load value.");
    }

    if(!cfg.Get("TELESCOP", param.station.TELESCOP)) {
        e = true;
        param.station.errormsg.push_back("- TELESCOP : Fail to load value.");
    }

    if(!cfg.Get("OBSERVER", param.station.OBSERVER)) {
        e = true;
        param.station.errormsg.push_back("- OBSERVER : Fail to load value.");
    }

    if(!cfg.Get("INSTRUME", param.station.INSTRUME)) {
        e = true;
        param.station.errormsg.push_back("- INSTRUME : Fail to load value.");
    }

    if(!cfg.Get("CAMERA", param.station.CAMERA)) {
        e = true;
        param.station.errormsg.push_back("- CAMERA : Fail to load value.");
    }

    if(!cfg.Get("FOCAL", param.station.FOCAL)) {
        e = true;
        param.station.errormsg.push_back("- FOCAL : Fail to load value.");
    }

    if(!cfg.Get("APERTURE", param.station.APERTURE)) {
        e = true;
        param.station.errormsg.push_back("- APERTURE : Fail to load value.");
    }

    if(!cfg.Get("SITELONG", param.station.SITELONG)) {
        e = true;
        param.station.errormsg.push_back("- SITELONG : Fail to load value.");
    }

    if(!cfg.Get("SITELAT", param.station.SITELAT)) {
        e = true;
        param.station.errormsg.push_back("- SITELAT : Fail to load value.");
    }

    if(!cfg.Get("SITEELEV", param.station.SITEELEV)) {
        e = true;
        param.station.errormsg.push_back("- SITEELEV : Fail to load value.");
    }

    if(!e) param.station.status = true;
}

void CfgParam::loadFitskeysParam() {

    bool e = false;

    if(!cfg.Get("K1", param.fitskeys.K1)) {
        e = true;
        param.fitskeys.errormsg.push_back("- K1 : Fail to load value.");
    }

    if(!cfg.Get("K2", param.fitskeys.K2)) {
        e = true;
        param.fitskeys.errormsg.push_back("- K2 : Fail to load value.");
    }

    if(!cfg.Get("FILTER", param.fitskeys.FILTER)) {
        e = true;
        param.fitskeys.errormsg.push_back("- FILTER : Fail to load value.");
    }

    if(!cfg.Get("CD1_1", param.fitskeys.CD1_1)) {
        e = true;
        param.fitskeys.errormsg.push_back("- CD1_1 : Fail to load value.");
    }

    if(!cfg.Get("CD1_2", param.fitskeys.CD1_2)) {
        e = true;
        param.fitskeys.errormsg.push_back("- CD1_2 : Fail to load value.");
    }

    if(!cfg.Get("CD2_1", param.fitskeys.CD2_1)) {
        e = true;
        param.fitskeys.errormsg.push_back("- CD2_1 : Fail to load value.");
    }

    if(!cfg.Get("CD2_2", param.fitskeys.CD2_2)) {
        e = true;
        param.fitskeys.errormsg.push_back("- CD2_2 : Fail to load value.");
    }

    if(!cfg.Get("XPIXEL", param.fitskeys.XPIXEL)) {
        e = true;
        param.fitskeys.errormsg.push_back("- XPIXEL : Fail to load value.");
    }

    if(!cfg.Get("YPIXEL", param.fitskeys.YPIXEL)) {
        e = true;
        param.fitskeys.errormsg.push_back("- YPIXEL : Fail to load value.");
    }

    if(!cfg.Get("COMMENT", param.fitskeys.COMMENT)) {
        e = true;
        param.fitskeys.errormsg.push_back("- COMMENT : Fail to load value.");
    }

    if(!e) param.fitskeys.status = true;
}

void CfgParam::loadMailParam() {

    bool e = false;

    if(!cfg.Get("MAIL_DETECTION_ENABLED", param.mail.MAIL_DETECTION_ENABLED)) {
        e = true;
        param.mail.errormsg.push_back("- MAIL_DETECTION_ENABLED : Fail to load value.");
    }else{

        if(param.mail.MAIL_DETECTION_ENABLED) {

            string mailRecipients;
            if(!cfg.Get("MAIL_RECIPIENT", mailRecipients)) {
                e = true;
                param.mail.errormsg.push_back("- MAIL_RECIPIENT : Fail to load value.");
            }else {

                typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                boost::char_separator<char> sep(",");
                tokenizer tokens(mailRecipients, sep);

                for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
                    param.mail.MAIL_RECIPIENTS.push_back(*tok_iter);
                }
            }

            if(!cfg.Get("MAIL_SMTP_SERVER", param.mail.MAIL_SMTP_SERVER)) {
                e = true;
                param.mail.errormsg.push_back("- MAIL_SMTP_SERVER : Fail to load value.");
            }

            string smtp_connection_type;
            if(!cfg.Get("MAIL_CONNECTION_TYPE", smtp_connection_type)) {
                e = true;
                param.mail.errormsg.push_back("- MAIL_CONNECTION_TYPE : Fail to load value.");
            }else {
                try{
                    EParser<SmtpSecurity> smtp_security;
                    param.mail.MAIL_CONNECTION_TYPE = smtp_security.parseEnum("MAIL_CONNECTION_TYPE", smtp_connection_type);

                    if(param.mail.MAIL_CONNECTION_TYPE != NO_SECURITY) {

                        if(!cfg.Get("MAIL_SMTP_LOGIN", param.mail.MAIL_SMTP_LOGIN)) {
                            e = true;
                            param.mail.errormsg.push_back("- MAIL_SMTP_LOGIN : Fail to load value.");
                        }

                        if(!cfg.Get("MAIL_SMTP_PASSWORD", param.mail.MAIL_SMTP_PASSWORD)) {
                            e = true;
                            param.mail.errormsg.push_back("- MAIL_SMTP_PASSWORD : Fail to load value.");
                        }
                    }else{
                        param.mail.MAIL_SMTP_LOGIN = "";
                        param.mail.MAIL_SMTP_PASSWORD = "";
                    }

                }catch (std::exception &ex) {
                    e = true;
                    param.mail.errormsg.push_back("- MAIL_CONNECTION_TYPE : " + string(ex.what()));
                }
            }
        }
    }

    if(!e) param.mail.status = true;

}

int CfgParam::getDeviceID() {
    return param.DEVICE_ID.first.first;
}

dataParam CfgParam::getDataParam() {
    return param.data;
}

logParam CfgParam::getLogParam() {
    return param.log;
}

framesParam CfgParam::getFramesParam() {
    return param.framesInput;
}

videoParam CfgParam::getVidParam() {
    return param.vidInput;
}

cameraParam CfgParam::getCamParam() {
    return param.camInput;
}

detectionParam CfgParam::getDetParam() {
    return param.det;
}

stackParam CfgParam::getStackParam() {
    return param.st;
}

stationParam CfgParam::getStationParam() {
    return param.station;
}

fitskeysParam CfgParam::getFitskeysParam() {
    return param.fitskeys;
}

mailParam CfgParam::getMailParam() {
    return param.mail;
}

parameters CfgParam::getAllParam() {
    return param;
}

bool CfgParam::deviceIdIsCorrect() {
    if(!param.DEVICE_ID.first.second) {
        if(showErrors) {
            cout << param.DEVICE_ID.second << endl;
        }
        return false;
    }
    return true;
}

bool CfgParam::dataParamIsCorrect() {
    if(!param.data.status) {
        if(showErrors) {
            for(int i = 0; i < param.data.errormsg.size(); i++)
                cout << param.data.errormsg.at(i) << endl;
        }
        return false;
    }
    return true;
}

bool CfgParam::logParamIsCorrect() {
    if(!param.log.status) {
        if(showErrors) {
            for(int i = 0; i < param.log.errormsg.size(); i++)
                cout << param.log.errormsg.at(i) << endl;
        }
        return false;
    }
    return true;
}

bool CfgParam::framesParamIsCorrect() {

    if(!param.framesInput.status) {
        if(showErrors) {
            for(int i = 0; i < param.framesInput.errormsg.size(); i++)
                cout << param.framesInput.errormsg.at(i) << endl;
        }
        return false;
    }
    return true;
}

bool CfgParam::vidParamIsCorrect() {
    if(!param.vidInput.status) {
        if(showErrors) {
            for(int i = 0; i < param.vidInput.errormsg.size(); i++)
                cout << param.vidInput.errormsg.at(i) << endl;
        }
        return false;
    }
    return true;
}

bool CfgParam::camParamIsCorrect() {
    if(!param.camInput.status) {
        if(showErrors) {
            for(int i = 0; i < param.camInput.errormsg.size(); i++)
                cout << param.camInput.errormsg.at(i) << endl;
        }
        return false;
    }
    return true;
}

bool CfgParam::detParamIsCorrect() {
    if(!param.det.status) {
        if(showErrors) {
            for(int i = 0; i < param.det.errormsg.size(); i++)
                cout << param.det.errormsg.at(i) << endl;
        }
        return false;
    }
    return true;
}

bool CfgParam::stackParamIsCorrect() {
    if(!param.st.status) {
        if(showErrors) {
            for(int i = 0; i < param.st.errormsg.size(); i++)
                cout << param.st.errormsg.at(i) << endl;
        }
        return false;
    }
    return true;
}

bool CfgParam::stationParamIsCorrect() {
    if(!param.station.status) {
        if(showErrors) {
            for(int i = 0; i < param.station.errormsg.size(); i++)
                cout << param.station.errormsg.at(i) << endl;
        }
        return false;
    }
    return true;
}

bool CfgParam::fitskeysParamIsCorrect() {
    if(!param.fitskeys.status) {
        if(showErrors) {
            for(int i = 0; i < param.fitskeys.errormsg.size(); i++)
                cout << param.fitskeys.errormsg.at(i) << endl;
        }
        return false;
    }
    return true;
}

bool CfgParam::mailParamIsCorrect() {
    if(!param.mail.status) {
        if(showErrors) {
            for(int i = 0; i < param.mail.errormsg.size(); i++)
                cout << param.mail.errormsg.at(i) << endl;
        }
        return false;
    }
    return true;
}

bool CfgParam::inputIsCorrect() {

    switch(inputType) {

        case VIDEO :
                return vidParamIsCorrect();
            break;

        case SINGLE_FITS_FRAME :
                return framesParamIsCorrect();
            break;

        // camera
        case CAMERA :
                return camParamIsCorrect();
            break;

    }

    return false;
}

bool CfgParam::allParamAreCorrect() {

    bool eFound = false;

    if(!deviceIdIsCorrect()){
        eFound = true;
        cout << ">> Errors on device ID. " << endl;
    }

    if(!dataParamIsCorrect()){
        eFound = true;
        cout << ">> Errors on data parameters. " << endl;
    }

    if(!logParamIsCorrect()){
        eFound = true;
        cout << ">> Errors on log parameters. " << endl;
    }

    if(!inputIsCorrect()){
        eFound = true;
        cout << ">> Errors on input parameters. " << endl;
    }

    if(!detParamIsCorrect()){
        eFound = true;
        cout << ">> Errors on detection parameters. " << endl;
    }

    if(!stackParamIsCorrect()){
        eFound = true;
        cout << ">> Errors on stack parameters. " << endl;
    }

    if(!stationParamIsCorrect()){
        eFound = true;
        cout << ">> Errors on station parameters. " << endl;
    }

    if(!fitskeysParamIsCorrect()){
        eFound = true;
        cout << ">> Errors on fitskeys parameters. " << endl;
    }

    if(!mailParamIsCorrect()){
        eFound = true;
        cout << ">> Errors on mail parameters. " << endl;
    }

    if(eFound)
        return false;

    return true;
}




