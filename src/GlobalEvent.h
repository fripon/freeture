/*
								GlobalEvent.h

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
 * @file    GlobalEvent.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    13/06/2014
 */


#pragma once

#include "includes.h"
#include "Frame.h"
#include "LocalEvent.h"
#include "RecEvent.h"
#include "SaveImg.h"

using namespace cv;
using namespace std;

using namespace boost::posix_time;

namespace logging	= boost::log;
namespace sinks		= boost::log::sinks;
namespace attrs		= boost::log::attributes;
namespace src		= boost::log::sources;
namespace expr		= boost::log::expressions;
namespace keywords	= boost::log::keywords;

class GlobalEvent{

    private:


        //! Event's age which depends on the frame buffer size
        int age;

        //! Number of frames since the last frame(localEvent) added to this global event
        int ageLastElem;

        //! Acquisition date of the first frame(localEvent) of the global event
        vector<string> dateEvent;

        //! Local events which composed this globalEvent
        vector<LocalEvent> listLocalEvent;

        //! Map of the last local event added
        Mat mapEvent;

        Point A;
        Point B;
        Point vDir;
        Point lastCoM;




    public:
        bool flash;
        int nbLEchecked;
        vector<Frame> framesOnDisk;
        vector<Frame> prevFrames;
        vector<int> framesNumberOnDisk;
        string frameBufferLocation;

        vector<Mat> geBuffer;

        bool LELinked;


        GlobalEvent(LocalEvent f, Mat mapp, vector<string> date);

        ~GlobalEvent();

        /*vector<int> *getDateEvent(){

            vector<int> *date = &dateEvent;

            return date;

        }*/

        vector<LocalEvent> *getListLocalEvent(){

            vector<LocalEvent> *liste = &listLocalEvent;

            return liste;

        }

        Point getA(){ return A; }
        void setA(Point p){ A = p; }

        Point getB(){ return B; }
        void setB(Point p){ B = p; }

        Point getvDir(){ return vDir; }
        void setvDir(Point p){ vDir = p; }

        Point getLastCoM(){ return lastCoM; }
        void setLastCoM(Point p){ lastCoM = p; }

        //! Increment the localEvent's position in the buffer
        /*!
          When a new frame is grabbed, the position of the previous localEvent from previous frames have to be shift because
          the new frame take the number 0 and the former frame in the buffer is removed.
        */
        void incrementLocalEventsPosition();

        //! Extract event's position in the frame and the event's position in the buffer
        /*!
          This function extract position's informations on the global event to fill a recEvent's properties.
        */
        void extractEventRecInfos(RecEvent &rec);

        Mat getMapEvent(){

            return mapEvent;

        }

        void setMapEvent(Mat m){

            m.copyTo(mapEvent);

        }

        int getAge(){

            return age;

        }

        int getAgeLastElem(){

            return ageLastElem;

        }

        vector<string> getDate(){

            return dateEvent;

        }

        void setAge(int a){

            age = a;

        }

        void setAgeLastElem(int age){

            ageLastElem = age;

        }

};
