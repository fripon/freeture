/*
								LocalEvent.h

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

//!  An event which occurs on a single frame, so call a localEvent
/*!
  Pixels which have a value superior than a detection threshold can form a localEvent in a single frame.
  if several Regions of interest centered on this kind of pixels cross on each other, it can define a localEvent's shape.
*/
class LocalEvent{

    private:

        //! Color
        /*!
          A color attached to this localEvent. It's used to group several Regions of interest to this localEvent
        */
        Scalar evColor;

         //! Map
        /*!
          A map which corresponds to the ROI that composed the localEvent
        */
        Mat evMap;

        //! Position in buffer
        /*!
          Indicate to which frame in the buffer, this localEvent has been detected
        */
        int framePosition;


    public:

        vector <Scalar> listColor;
        vector <Point> listRoiCenter;           // Liste des ROI qui compose le localEvent
        vector <vector<PixelEvent> > listRoiPix;   // Liste des pixels dans chaque ROI qui ont une valeurs supérieures au seuil de détection
        Point centerOfMass;                     // Centre de masse des pixels de chaque ROI
        int nbArea;
        int area;

        bool belongGlobalEv;

        bool notTakeAccount;

        LocalEvent(Scalar color, Point roiPos, vector <PixelEvent> listPix, int H, int W, const int *roiSize);

        ~LocalEvent();

        void computeCenterOfMass(bool roi);


        Scalar getColor(){

            return evColor;

        };

        Mat getMap(){

            return evMap;

        };

        int getFramePosition(){

            return framePosition;

        }

        void setFramePosition(int pos){

            framePosition = pos;

        }

        void setMap(Mat mapM){

            mapM.copyTo(evMap);

        };

};
