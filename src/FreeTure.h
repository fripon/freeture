/*
								FreeTure.h

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
*	Last modified:		04/12/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    FreeTure.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    19/06/2014
* \brief   FreeTure'parameters.
*/

#pragma once

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <boost/tokenizer.hpp>
#include "Configuration.h"
#include "ECamType.h"
#include "ECamBitDepth.h"
#include "EDetMeth.h"
#include "EStackMeth.h"
#include "EParser.h"

#include <boost/filesystem.hpp>

class FreeTure{

    private:

        string cfgFile;

	public:

        FreeTure(string cfg):cfgFile(cfg){};

		bool loadParameters();

		void printParameters();

        EParser<CamType> cam_type;
        EParser<DetMeth> det_mth;
        EParser<StackMeth> stack_mth;
        EParser<LogSeverityLevel> log_sev;

	    CamType	            CAMERA_TYPE;
        std::string         CAMERA_NAME;
        int		            CAMERA_ID;
        std::string	        VIDEO_PATH;
        std::string         FRAMES_PATH;
        int		            ACQ_FPS;
        CamBitDepth		    ACQ_BIT_DEPTH;
        int                 ACQ_EXPOSURE;
        int                 ACQ_GAIN;
        bool                ACQ_AUTO_EXPOSURE_ENABLED;
        bool                ACQ_MASK_ENABLED;
        std::string         ACQ_MASK_PATH;
        int                 ACQ_BUFFER_SIZE;
        bool	            DET_ENABLED;
        DetMeth	            DET_METHOD;
        bool                DET_SAVE_FITS3D;
        bool                DET_SAVE_FITS2D;
        bool                DET_SAVE_BMP;
        bool                DET_SAVE_SUM;
        bool                DET_SAVE_AVI;
        bool                DET_SAVE_GEMAP;
        bool                DET_SAVE_TRAIL;
        bool                DET_SAVE_POS;
        bool                DET_DOWNSAMPLE_ENABLED;
        double              DET_TIME_BEFORE;
        double              DET_TIME_AFTER;
        int                 DET_GE_MAX;
        int                 DET_TIME_MAX;
        bool	            STACK_ENABLED;
        double	            STACK_TIME;
        int		            STACK_INTERVAL;
        StackMeth           STACK_MTHD;
        bool                STACK_REDUCTION;
        vector<string>      MAIL_RECIPIENT;
        bool                MAIL_DETECTION_ENABLED;
        string              MAIL_SMTP_HOSTNAME;
        string              MAIL_SMTP_SERVER;
        bool                DEBUG_ENABLED;
        std::string         DEBUG_PATH;
        std::string         STATION_NAME;
        bool                CFG_FILECOPY_ENABLED;
        std::string         DATA_PATH;
        std::string         LOG_PATH;
        string              FRAMES_SEPARATOR;
        int                 FRAMES_SEPARATOR_POSITION;
        LogSeverityLevel    LOG_SEVERITY;
        double              LONGITUDE;
        std::string         logDirName;

};


