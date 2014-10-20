/*
								GlobalEvent.cpp

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
 * @file    GlobalEvent.cpp
 * @author  Yoan Audureau
 * @version 1.0
 * @date    03/06/2014
*/

#include "GlobalEvent.h"

GlobalEvent::GlobalEvent(LocalEvent f, Mat mapp, vector<string> date){

    age             = 0;
    ageLastElem     = 0;
    f               = f;
    listLocalEvent.push_back(f);
    mapp.copyTo(mapEvent);
    B               = Point(0,0);
    vDir            = Point(0,0);
    dateEvent       = date;
    frameBufferLocation = "";
    nbLEchecked = 0;

    LELinked = false;
    flash = false;

}

GlobalEvent::~GlobalEvent(){
    //dtor
}

void GlobalEvent::incrementLocalEventsPosition(){

    vector<LocalEvent>::iterator it;

    for (it=listLocalEvent.begin(); it!=listLocalEvent.end(); ++it){

       // cout << "Pos before : "<< (*it).getFramePosition() <<endl;
        (*it).setFramePosition((*it).getFramePosition()+1);
       // cout << "Pos after : "<< (*it).getFramePosition() <<endl;

    }
}

void GlobalEvent::extractEventRecInfos(RecEvent &rec){

    vector<Point>   eventXYPosition;
    vector<int>     eventBufferPosition;

    vector<LocalEvent>::iterator it;

    for (it=listLocalEvent.begin(); it!=listLocalEvent.end(); ++it){

        eventXYPosition.push_back((*it).centerOfMass);
        eventBufferPosition.push_back((*it).getFramePosition());

    }

    rec.setBuffer(geBuffer);                        // Copy the event's buffer
    rec.setListMetPos(eventXYPosition);             // XY positions of the event in the frame
    rec.setPositionInBuffer(eventBufferPosition);   // Frame numbers of event positions
    rec.setMapEvent(mapEvent);                      // GEMAP
    rec.setDateEvent(dateEvent);                    // Date of the first event's frame

}

/*


    recEvent *rec = new recEvent(listMetPos, positionInBuffer);
    recEvent &refRec = *rec;

    extractEventRecInfos(*rec) //*rec est ensuite pris comme une référence


    delete rec;


*/
