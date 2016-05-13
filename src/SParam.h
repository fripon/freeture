/*
                                SParam.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*   This file is part of:   freeture
*
*   Copyright:      (C) 2014-2016 Yoan Audureau
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
*   Last modified:      20/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    SParam.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   FreeTure parameters
*/

#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <map>
#include <stdlib.h>
#include "ECamPixFmt.h"
#include "ETimeMode.h"
#include "EImgFormat.h"
#include "EDetMeth.h"
#include "ELogSeverityLevel.h"
#include "EStackMeth.h"
#include "ESmtpSecurity.h"
#include <vector>
#include "EInputDeviceType.h"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

// ******************************************************
// ****************** MAIL PARAMETERS *******************
// ******************************************************

struct mailParam{
    bool            MAIL_DETECTION_ENABLED;
    string          MAIL_SMTP_SERVER;
    SmtpSecurity    MAIL_CONNECTION_TYPE;
    string          MAIL_SMTP_LOGIN;
    string          MAIL_SMTP_PASSWORD;
    vector<string>  MAIL_RECIPIENTS;
    bool status;
    vector<string> errormsg;
};

// ******************************************************
// ******************* LOG PARAMETERS *******************
// ******************************************************

struct logParam {
    string              LOG_PATH;
    int                 LOG_ARCHIVE_DAY;
    int                 LOG_SIZE_LIMIT;
    LogSeverityLevel    LOG_SEVERITY;
    bool status;
    vector<string> errormsg;
};

// ******************************************************
// **************** OUTPUT DATA PARAMETERS **************
// ******************************************************

struct dataParam {
    string  DATA_PATH;
    bool    FITS_COMPRESSION;
    string  FITS_COMPRESSION_METHOD;
    bool status;
    vector<string> errormsg;
};

// ******************************************************
// **************** INPUT FRAMES PARAMETERS *************
// ******************************************************

struct framesParam {
    int INPUT_TIME_INTERVAL;
    vector<string> INPUT_FRAMES_DIRECTORY_PATH;
    bool status;
    vector<string> errormsg;
};

// ******************************************************
// **************** INPUT VIDEO PARAMETERS **************
// ******************************************************

struct videoParam {
    int INPUT_TIME_INTERVAL;
    vector<string> INPUT_VIDEO_PATH;
    bool status;
    vector<string> errormsg;
};

// ******************************************************
// ********* SCHEDULED ACQUISITION PARAMETERS ***********
// ******************************************************

struct scheduleParam {
    int hours;
    int min;
    int sec;
    int exp;
    int gain;
    int rep;
    CamPixFmt fmt;
};

// ******************************************************
// ************** INPUT CAMERA PARAMETERS ***************
// ******************************************************

struct cameraParam{
    int         ACQ_FPS;
    CamPixFmt   ACQ_FORMAT;
    bool        ACQ_RES_CUSTOM_SIZE;
    bool        SHIFT_BITS;
    int         ACQ_NIGHT_EXPOSURE;
    int         ACQ_NIGHT_GAIN;
    int         ACQ_DAY_EXPOSURE;
    int         ACQ_DAY_GAIN;
    int         ACQ_HEIGHT;
    int         ACQ_WIDTH;
    int         EXPOSURE_CONTROL_FREQUENCY;
    bool        EXPOSURE_CONTROL_SAVE_IMAGE;
    bool        EXPOSURE_CONTROL_SAVE_INFOS;

    struct ephemeris {
        bool    EPHEMERIS_ENABLED;
        double  SUN_HORIZON_1;
        double  SUN_HORIZON_2;
        vector<int>  SUNRISE_TIME;
        vector<int>  SUNSET_TIME;
        int     SUNSET_DURATION;
        int     SUNRISE_DURATION;
    };
    ephemeris ephem;

    struct regularCaptures {
        bool        ACQ_REGULAR_ENABLED;
        TimeMode    ACQ_REGULAR_MODE;
        ImgFormat   ACQ_REGULAR_OUTPUT;
        struct regularParam {
            int interval;
            int exp;
            int gain;
            int rep;
            CamPixFmt fmt;
        };
        regularParam ACQ_REGULAR_CFG;
    };
    regularCaptures regcap;

    struct scheduledCaptures {
        bool        ACQ_SCHEDULE_ENABLED;
        ImgFormat   ACQ_SCHEDULE_OUTPUT;
        vector<scheduleParam> ACQ_SCHEDULE;
    };
    scheduledCaptures schcap;

    bool status;
    vector<string> errormsg;
};

// ******************************************************
// **************** DETECTION PARAMETERS ****************
// ******************************************************

struct detectionParam {
    int         ACQ_BUFFER_SIZE;
    bool        ACQ_MASK_ENABLED;
    string      ACQ_MASK_PATH;
    Mat         MASK;
    bool        DET_ENABLED;
    TimeMode    DET_MODE;
    bool        DET_DEBUG;
    string      DET_DEBUG_PATH;
    int         DET_TIME_AROUND;
    int         DET_TIME_MAX;
    DetMeth     DET_METHOD;
    bool        DET_SAVE_FITS3D;
    bool        DET_SAVE_FITS2D;
    bool        DET_SAVE_SUM;
    bool        DET_SUM_REDUCTION;
    StackMeth   DET_SUM_MTHD;
    bool        DET_SAVE_SUM_WITH_HIST_EQUALIZATION;
    bool        DET_SAVE_AVI;
    bool        DET_UPDATE_MASK;
    int         DET_UPDATE_MASK_FREQUENCY;
    bool        DET_DEBUG_UPDATE_MASK;
    bool        DET_DOWNSAMPLE_ENABLED;

    struct detectionMethod1 {
        bool    DET_SAVE_GEMAP;
        bool    DET_SAVE_DIRMAP;
        bool    DET_SAVE_POS;
        int     DET_LE_MAX;
        int     DET_GE_MAX;
        //bool    DET_SAVE_GE_INFOS;
    };
    detectionMethod1 temporal;

    bool status;
    vector<string> errormsg;

};

// ******************************************************
// ******************* STACK PARAMETERS *****************
// ******************************************************

struct stackParam{
    bool        STACK_ENABLED;
    TimeMode    STACK_MODE;
    int         STACK_TIME;
    int         STACK_INTERVAL;
    StackMeth   STACK_MTHD;
    bool        STACK_REDUCTION;
    bool status;
    vector<string> errormsg;
};

// ******************************************************
// ****************** STATION PARAMETERS ****************
// ******************************************************

 struct stationParam {
    string STATION_NAME;
    string TELESCOP;
    string OBSERVER;
    string INSTRUME;
    string CAMERA;
    double FOCAL;
    double APERTURE;
    double SITELONG;
    double SITELAT;
    double SITEELEV;
    bool status;
    vector<string> errormsg;
};

// ******************************************************
// ***************** FITS KEYS PARAMETERS ***************
// ******************************************************

struct fitskeysParam{
    string FILTER;
    double K1;
    double K2;
    string COMMENT;
    double CD1_1;
    double CD1_2;
    double CD2_1;
    double CD2_2;
    double XPIXEL;
    double YPIXEL;
    bool status;
    vector<string> errormsg;
};

// ******************************************************
// ****************** FREETURE PARAMETERS ***************
// ******************************************************

struct parameters {
    pair<pair<int, bool>,string> DEVICE_ID; // Pair : <value, status>
    dataParam       data;
    logParam        log;
    framesParam     framesInput;
    videoParam      vidInput;
    cameraParam     camInput;
    detectionParam  det;
    stackParam      st;
    stationParam    station;
    fitskeysParam   fitskeys;
    mailParam       mail;
};

