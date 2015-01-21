/*
				LocalEvent.h

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
 * @file    LocalEvent.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    12/06/2014
 */

#pragma once

#include "includes.h"
#include "PixelEvent.h"

using namespace cv;
using namespace std;
using namespace boost::posix_time;

namespace logging	= boost::log;
namespace sinks		= boost::log::sinks;
namespace attrs		= boost::log::attributes;
namespace src		= boost::log::sources;
namespace expr		= boost::log::expressions;
namespace keywords	= boost::log::keywords;

class LocalEvent{

    private:

        Scalar  LE_Color;
        Mat     LE_Map;
        Point   LE_MassCenter;

    public:

        vector<Point> LE_Roi;

        LocalEvent(Scalar color, Point roiPos, int frameHeight, int frameWidth, const int *roiSize);

        ~LocalEvent();

        void computeMassCenterWithRoi();

        Scalar  getColor()          {return LE_Color;};
        Mat     getMap()            {return LE_Map;};
        void    setMap(Mat mapM)    {mapM.copyTo(LE_Map);};
        Point   getMassCenter()     {return LE_MassCenter;};

};
