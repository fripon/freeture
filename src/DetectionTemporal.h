/*
						DetectionTemporalMovement.h

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
*	Last modified:		20/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    DetectionTemporal.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Detection method by temporal movement.
*/

#pragma once

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <boost/tokenizer.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/core.hpp>
#include "ELogSeverityLevel.h"
#include "TimeDate.h"
#include "Fits2D.h"
#include "Fits.h"
#include "Frame.h"
#include "EStackMeth.h"
#include "ECamBitDepth.h"
#include "GlobalEvent.h"
#include "LocalEvent.h"
#include "Detection.h"
#include "EParser.h"
#include "SaveImg.h"
#include <vector>
#include <iterator>
#include <algorithm>

#include <boost/filesystem.hpp>

using namespace boost::filesystem;

using namespace std;
using namespace cv;

class DetectionTemporal : public Detection{

	private :

		static boost::log::sources::severity_logger< LogSeverityLevel > logger;

		static class _Init{

			public:
				_Init()
				{
					logger.add_attribute("ClassName", boost::log::attributes::constant<std::string>("DetectionTemporal"));
				}
		} _initializer;	

        vector<GlobalEvent>	listGlobalEvents;
        int					nbGE;
		vector<Point>		subdivisionPos;
		int roiSize[2];
		vector<Scalar>		listColors; // B, G, R
		
		Mat localMask;

		bool initStatus;

		Mat prevthresh;

		vector<GlobalEvent>::iterator geToSave;

		bool DET_DOWNSAMPLE_ENABLED;

		CamBitDepth ACQ_BIT_DEPTH;

		bool DET_SAVE_GEMAP;

		bool DET_SAVE_POS;
		
	public:

		DetectionTemporal();
	
		~DetectionTemporal();

		void sortSubdivision(unsigned char percentage);

		void subdivideFrame(vector<Point> &sub, int n, int imgH, int imgW);

		int defineThreshold(Mat i, Mat m);

		vector<Scalar> getColorInEventMap(Mat &eventMap, Point roiCenter);

		Point roiPositionToFramePosition(Point roiPix, Point newOrigine, Point framePix);

		void colorInBlack(int j, int i, int areaPosX, int areaPosY, Point areaPosition, Mat &area, Mat &frame);

		void searchROI( Mat &area,                      
						Mat &frame,                   
						Mat &eventMap,
						vector<LocalEvent> &listLE,    
						int imgH,                     
						int imgW,                      
						int areaPosX,                 
						int areaPosY,                   
						Point areaPosition,
						int maxNbLE);    

		bool run(Frame &c, Frame &p);

		int getNumFirstEventFrame();

		string getDateEvent();
		
		int getNumLastEventFrame();

		void resetDetection();

		void saveDetectionInfos(string p);

		bool initMethod(string cfg_path);

		void save(){
			/*




			 /// SAVE mapGE

    if(recMapGE){

        SaveImg::saveBMP((*itGE).getMapEvent(), currentEventPath + "GEMap");
        mailAttachments.push_back(currentEventPath + "GEMap.bmp");

        SaveImg::saveBMP((*itGE).getDirMap(), currentEventPath + "DirMap");

    }

	/// POSITIONS FILE

    if(recPos){

        ofstream posFile;
        string posFilePath = currentEventPath + "positions.txt";
        posFile.open(posFilePath.c_str());

        vector<LocalEvent>::iterator itLE;

        for(itLE = (*itGE).LEList.begin(); itLE!=(*itGE).LEList.end(); ++itLE){

            Point pos;
            pos = (*itLE).getMassCenter();

            if(downsample){

                pos*=2;

            }

            string line = Conversion::intToString((*itLE).getNumFrame() - numFirstFrameToSave) + "               (" + Conversion::intToString(pos.x)  + ";" + Conversion::intToString(pos.y) + ")\n";
            posFile << line;

        }

        // infos

        posFile.close();

    }


	/// INFOS DET
    bool infos = true;
    if(infos){

        ofstream infFile;
        string infFilePath = currentEventPath + "infos.txt";
        infFile.open(infFilePath.c_str());


        infFile << " * AGE              : " << (*itGE).getAge() << "\n";
        infFile << " * AGE LAST ELEM    : " << (*itGE).getAgeLastElem()<< "\n";
        infFile << " * LINEAR STATE     : " << (*itGE).getLinearStatus()<< "\n";
        infFile << " * BAD POS          : " << (*itGE).getBadPos()<< "\n";
        infFile << " * GOOD POS         : " << (*itGE).getGoodPos()<< "\n";
        infFile << " * Num first frame  : " << (*itGE).getNumFirstFrame()<< "\n";
        infFile << " * Num last frame   : " << (*itGE).getNumLastFrame()<< "\n";
        //infFile << " * Static Test      : " << (*itGE).getStaticTestRes();
        infFile << "\n mainPoint : \n";

        for(int i = 0; i < (*itGE).mainPoints.size(); i++){

            infFile << "(" << (*itGE).mainPoints.at(i).x << ";"<<  (*itGE).mainPoints.at(i).y << ")\n";

        }

        infFile << "\n distance : \n";

         for(int i = 0; i < (*itGE).dist.size(); i++){

            infFile << (*itGE).dist.at(i)<< "\n";

        }

        infFile.close();
/*
        vector<LocalEvent>::iterator itLE;
        int cpt = 0;

        for(itLE = (*itGE).LEList.begin(); itLE!=(*itGE).LEList.end(); ++itLE){

            SaveImg::saveBMP((*itGE).getMapEvent(), currentEventPath + "LEMap_" + Conversion::intToString(cpt));
            cpt++;

        }*/
/*
        SaveImg::saveBMP((*itGE).getDirMap2(), currentEventPath + "DirMap2");

        mailAttachments.push_back(currentEventPath + "DirMap2.bmp");

    }


			/* breakAnalyse = true;

                    (*itGEToSave).setAgeLastElem((*itGEToSave).getAgeLastElem() + 1);
                    (*itGEToSave).setAge((*itGEToSave).getAge() + 1);

                    if((*itGEToSave).getAgeLastElem() > timeAfter ||
                       (currentFrame.getFrameRemaining() < 10 && currentFrame.getFrameRemaining()!= 0)){

                        cout << "> Build event location " << endl;
                        cout << "Date size : " << (*itGEToSave).getDate().size() << endl;
                        for(int a = 0; a< (*itGEToSave).getDate().size() ; a++ )
                            cout << (*itGEToSave).getDate().at(a) << endl;


                        string currentEventPath;

                        double timeSave = (double)getTickCount();

                        RecEvent::buildEventLocation((*itGEToSave).getDate(),recordingPath,station, currentEventPath);

                        cout << "Rec event " << endl;


                        boost::mutex::scoped_lock lock(*m_frameBuffer);

                        RecEvent::saveGE(
                                            frameBuffer,
                                            listGlobalEvents,
                                            itGEToSave,
                                            fitsHeader,
                                            downsample,
                                            recAvi,
                                            recFits3D,
                                            recFits2D,
                                            recPos,
                                            recSum,
                                            recBmp,
                                            recMapGE,
                                            timeAfter,
                                            timeBefore,
                                            frameBufferMaxSize,
                                            mailNotification,
                                            SMTPServer,
                                            SMTPHostname,
                                            mailRecipients,
                                            recordingPath,
                                            station,
                                            currentEventPath,
                                            imgFormat);

                        lock.unlock();

                        listGlobalEvents.clear();*/

		};

};

