/*
				RecEvent.h

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
* \file    RecEvent.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Save a detected event.
*/

#pragma once

#include "includes.h"
#include "Frame.h"
#include "SaveImg.h"
#include "Fits.h"
#include "Fits2D.h"
#include "Fits3D.h"
#include "SMTPClient.h"
#include "ECamBitDepth.h"
#include "GlobalEvent.h"
#include "ImgReduction.h"
#include <boost/circular_buffer.hpp>
#include "TimeDate.h"
#include "Conversion.h"

using namespace cv;
using namespace std;

class RecEvent{

    public :

        RecEvent(){};
        ~RecEvent(){};

        static bool buildEventLocation(vector<string> eventDate, string eventPath, string stationName, string &currentEventPath);

        static bool saveGE( boost::circular_buffer<Frame>  *frameBuffer,
                            vector<GlobalEvent> &GEList,
                            vector<GlobalEvent>::iterator itGE,
                            Fits fitsHeader,
                            bool downsample,
                            bool recAvi,
                            bool recFits3D,
                            bool recFits2D,
                            bool recPos,
                            bool recSum,
                            bool recBmp,
                            bool recMapGE,
                            int timeAfter,
                            int timeBefore,
                            int frameBufferMaxSize,
                            bool mailNotification,
                            string SMTPServer,
                            string SMTPHostname,
                            vector<string> mailRecipients,
                            string eventPath,
                            string stationName,
                            string currentEventPath,
                            CamBitDepth pixelFormat);

};
