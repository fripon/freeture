/*
								DetByLists.h

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
 * @file    DetByLists.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    17/06/2014
 */

#pragma once

#include "includes.h"
#include "LocalEvent.h"
#include "PixelEvent.h"
#include "RecEvent.h"
#include "Conversion.h"
#include "ManageFiles.h"
#include "SaveImg.h"
#include "Frame.h"

#include "Fits2D.h"
#include "ELogSeverityLevel.h"
#include "ECamBitDepth.h"
#include <boost/filesystem.hpp>
#include <iterator>
#include <algorithm>

using namespace boost::filesystem;

using namespace std;
using namespace cv;

using namespace boost::posix_time;

namespace logging	= boost::log;
namespace sinks		= boost::log::sinks;
namespace attrs		= boost::log::attributes;
namespace src		= boost::log::sources;
namespace expr		= boost::log::expressions;
namespace keywords	= boost::log::keywords;

class DetByLists{

    public:

        DetByLists();

        static bool detectionMethodByListManagement(
                                                        Frame currentFrame,
                                                        Frame previousFrame,
                                                        int const *roiSize,
                                                        vector<GlobalEvent> &listGlobalEvents,
                                                        vector<Scalar> &listColors,
                                                        Mat mask,
                                                        int timeMax,
                                                        int nbGE,
                                                        int timeAfter,
                                                        int pixelFormat,
                                                        Mat localMask,
                                                        bool debug,
                                                        vector<Point> regionsPos,
                                                        bool downsample,
                                                        Mat &prevthresh,
                                                        int &nbDet,
                                                        vector<GlobalEvent>::iterator &itGEToSave);

        static void buildListSubdivisionOriginPoints(
                                                        vector<Point> &listSubdivPosition,
                                                        int nbSubdivOnAxis,
                                                        int imgH,
                                                        int imgW                            );
};
