/*
                                CfgParam.h

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
*   Last modified:      20/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    CfgParam.h
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
#include <boost/filesystem.hpp>
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <stdlib.h>
#include "ECamPixFmt.h"
#include "ETimeMode.h"
#include "EImgFormat.h"
#include "EDetMeth.h"
#include "ELogSeverityLevel.h"
#include "EStackMeth.h"
#include "ESmtpSecurity.h"
#include <vector>
#include "CfgLoader.h"
#include "Device.h"
#include "EInputDeviceType.h"
#include "SParam.h"
#include "ECamSdkType.h"

using namespace boost::filesystem;
using namespace std;
using namespace cv;

class CfgParam{

    private :

        static boost::log::sources::severity_logger< LogSeverityLevel > logger;

        static class Init {

            public :

                Init() {

                    logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("CfgParam"));

                }

        }initializer;

        CfgLoader cfg;
        parameters param;

        InputDeviceType inputType;

        void loadDeviceID();
        void loadDataParam();
        void loadLogParam();
        void loadFramesParam();
        void loadVidParam();
        void loadCamParam();
        void loadDetParam();
        void loadStackParam();
        void loadStationParam();
        void loadFitskeysParam();
        void loadMailParam();

        vector<string> emsg;

    public :

        bool showErrors;

        /**
         * Constructor.
         *
         */
        CfgParam(string cfgFilePath);

        int             getDeviceID();
        dataParam       getDataParam();
        logParam        getLogParam();
        framesParam     getFramesParam();
        videoParam      getVidParam();
        cameraParam     getCamParam();
        detectionParam  getDetParam();
        stackParam      getStackParam();
        stationParam    getStationParam();
        fitskeysParam   getFitskeysParam();
        mailParam       getMailParam();
        parameters      getAllParam();

        bool deviceIdIsCorrect();
        bool dataParamIsCorrect();
        bool logParamIsCorrect();
        bool framesParamIsCorrect();
        bool vidParamIsCorrect();
        bool camParamIsCorrect();
        bool detParamIsCorrect();
        bool stackParamIsCorrect();
        bool stationParamIsCorrect();
        bool fitskeysParamIsCorrect();
        bool mailParamIsCorrect();
        bool allParamAreCorrect();
        bool inputIsCorrect();

};

