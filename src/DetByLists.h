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
#include "GlobalEvent.h"
#include "PixelEvent.h"
#include "RecEvent.h"
#include "Conversion.h"
#include "ManageFiles.h"
#include "SaveImg.h"
#include "Frame.h"
#include "Fifo.h"
#include "Fits2D.h"
#include "EnumLog.h"
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

using namespace logenum;

class DetByLists{

    public:

        DetByLists();

        static bool detectionMethodByListManagement(
                                                    Frame f,
                                                    vector<string> date,
                                                    Mat currentFrame,
                                                    Mat previousFrame,
                                                    Mat mean,
                                                    int const *roiSize,
                                                    vector <GlobalEvent> &listGlobalEvents,
                                                    Mat mask,
                                                    boost::mutex &mutex_listEventToRecord,
                                                    boost::mutex &mutexQueue,
                                                    vector<RecEvent> &listEventToRecord,
                                                    Fifo<Frame>  &framesQueue,
                                                    string recPath,
                                                    string stationName,
                                                    int &nbDet,
                                                    int geMaxElement,
                                                    int geMaxDuration,
                                                    int geAfterTime,
                                                    int pixelFormat,
                                                    vector<Point> &lastDet,
                                                    Mat maskNeighborhood,
                                                    VideoWriter &videoDebug,
                                                    bool debug,
                                                    vector<Point> listSubdivPosition,
                                                    bool maskMoon,
                                                    Point moonPos,
                                                    bool downsample  );

        static void buildListSubdivisionOriginPoints(
                                                        vector<Point> &listSubdivPosition,
                                                        int nbSubdivOnAxis,
                                                        int imgH,
                                                        int imgW                            );
};
