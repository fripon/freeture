/*
								FreeTure.h

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
*	Last modified:		04/12/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
 * @file    FreeTure.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    04/12/2014
 *
 */

#pragma once

#include "includes.h"
#include "Configuration.h"
#include "ECamType.h"
#include "ECamBitDepth.h"
#include "EDetMeth.h"
#include "EAstStackMeth.h"
#include "EnumParser.h"

#include <boost/filesystem.hpp>



class FreeTure{

    private:

        string cfgFile;

	public:

        FreeTure(string cfg):cfgFile(cfg){};

		bool loadParameters();

		void printParameters();

        EnumParser<CamType> cam_type;

	    ///======= Input parameters =======

	    CamType	        CAMERA_TYPE;
        std::string     CAMERA_NAME;
        int		        CAMERA_ID;
        std::string	    VIDEO_PATH;
        std::string     FRAMES_PATH;
        int             FRAMES_START;
        int             FRAMES_STOP;

        ///==== Acquisition parameters ====

        int		        ACQ_FPS;
        CamBitDepth		ACQ_BIT_DEPTH;
        int             ACQ_EXPOSURE;
        int             ACQ_GAIN;
        bool            ACQ_AUTO_EXPOSURE_ENABLED;
        bool            ACQ_MASK_ENABLED;
        std::string     ACQ_MASK_PATH;
        bool            ACQ_SAVE_FRAMES_ENABLED;
        std::string     ACQ_SAVE_FRAMES_TYPE;
        std::string     ACQ_SAVE_FRAMES_PATH;
        int		        ACQ_SAVE_FRAMES_BUFFER_SIZE;

        ///====== Detection parameters ====

        bool	        DET_ENABLED;
        DetMeth	        DET_METHOD;
        bool            DET_SAVE_FITS3D;
        bool            DET_SAVE_FITS2D;
        bool            DET_SAVE_BMP;
        bool            DET_SAVE_SUM;
        bool            DET_SAVE_AVI;
        bool            DET_SAVE_GEMAP;
        bool            DET_SAVE_TRAIL;
        bool            DET_SAVE_POS;
        bool            DET_DOWNSAMPLE_ENABLED;
        int             DET_TIME_BEFORE;
        int             DET_TIME_AFTER;
        int             DET_GE_MAX;
        int             DET_TIME_MAX;

        ///======== Stack parameters ======

        bool	        STACK_ENABLED;
        double	        STACK_TIME;
        int		        STACK_INTERVAL;
        AstStackMeth    STACK_MTHD;
        bool            STACK_REDUCTION;

        ///======= Others parameters ======

        bool            DEBUG_ENABLED;
        std::string     DEBUG_PATH;

        std::string     STATION_NAME;
        bool            CFG_FILECOPY_ENABLED;
        std::string     DATA_PATH;

        std::string     LOG_PATH;

        double          LONGITUDE;

        std::string     logDirName;

};


