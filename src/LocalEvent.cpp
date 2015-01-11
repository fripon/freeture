/*
								LocalEvent.cpp

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
 * @file    LocalEvent.cpp
 * @author  Yoan Audureau
 * @version 1.0
 * @date    03/06/2014
 * @section DESCRIPTION
 *
 * The detection class contains all meteor detection methods
 */

#include "LocalEvent.h"

LocalEvent::LocalEvent( Scalar              color,
                        Point               roiPos,
                        vector <PixelEvent> listPix,
                        int                 H,
                        int                 W,
                        const int                 *roiSize){
    nbArea = 0;
    evColor = color;
    listRoiCenter.push_back(roiPos);
    evMap = Mat::zeros(H,W,CV_8UC1);
    Mat roi(roiSize[1],roiSize[0],CV_8UC1,Scalar(255));
    roi.copyTo(evMap(Rect(roiPos.x-roiSize[0]/2,roiPos.y-roiSize[1]/2,roiSize[0],roiSize[1])));
    listRoiPix.push_back(listPix);
    belongGlobalEv = false;
    notTakeAccount = false;
    //centerOfMass = roiPos;
    // Indique à quel frame du frame buffer le localEvent courant appartient
    framePosition = 0;

}

LocalEvent::~LocalEvent(){

}


void LocalEvent::computeCenterOfMass(bool roi){

    double t = (double)getTickCount();

    float CoM_x = 0.0, CoM_y = 0.0;

    int massSum = 0, X = 0, Y = 0;

    if(roi){

        vector <Point>::iterator it;

        for (it=listRoiCenter.begin(); it!=listRoiCenter.end(); ++it){

            X += (*it).x;
            Y +=(*it).y;
        }

        X = X / listRoiCenter.size();
        Y = Y / listRoiCenter.size();

    }else{

        // Parcours de la liste des pixels de chaque ROI qui forme ce localEvent
        vector< vector<PixelEvent> >::iterator it;

        for (it=listRoiPix.begin(); it!=listRoiPix.end(); ++it){

            vector <PixelEvent> &pe = *it;

            vector <PixelEvent>::iterator it1;

            for (it1=pe.begin(); it1!=pe.end(); ++it1){

                PixelEvent &pe1 = *it1;

                X+=pe1.intensity * pe1.position.x;
                Y+=pe1.intensity * pe1.position.y;

                massSum += pe1.intensity;

            }
        }

        X = (float)(X/massSum);
        Y = (float)(Y/massSum);

    }

    centerOfMass = Point(X,Y);

    //cout << "center of mass: (" << X<<";"<<Y<<")"<<endl;

    t = (((double)getTickCount() - t)/getTickFrequency())*1000;

   // cout << "Time CoM : " <<std::setprecision(3)<< std::fixed<< t << " ms"<< endl;

}
