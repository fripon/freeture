/*
						DetectionTemporal.cpp

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
* \file    DetectionTemporal.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Detection method by temporal movement.
*/

#include "DetectionTemporal.h"

boost::log::sources::severity_logger< LogSeverityLevel >  DetectionTemporal::logger;
DetectionTemporal::_Init DetectionTemporal::_initializer;

DetectionTemporal::DetectionTemporal(){

	initStatus = false;

	DEBUG = false;

	listColors.push_back(Scalar(0,0,139));      // DarkRed
	listColors.push_back(Scalar(0,0,255));      // Red
	listColors.push_back(Scalar(60,20,220));    // Crimson
	listColors.push_back(Scalar(0,100,100));    // IndianRed
	listColors.push_back(Scalar(92,92,205));    // Salmon
	listColors.push_back(Scalar(0,140,255));    // DarkOrange
	listColors.push_back(Scalar(30,105,210));   // Chocolate
	listColors.push_back(Scalar(0,255,255));    // Yellow
	listColors.push_back(Scalar(140,230,240));  // Khaki
	listColors.push_back(Scalar(224,255,255));  // LightYellow
	listColors.push_back(Scalar(211,0,148));    // DarkViolet
	listColors.push_back(Scalar(147,20,255));   // DeepPink
	listColors.push_back(Scalar(255,0,255));    // Magenta
	listColors.push_back(Scalar(0,100,0));      // DarkGreen
	listColors.push_back(Scalar(0,128,128));    // Olive
	listColors.push_back(Scalar(0,255,0));      // Lime
	listColors.push_back(Scalar(212,255,127));  // Aquamarine
	listColors.push_back(Scalar(208,224,64));   // Turquoise
	listColors.push_back(Scalar(205,0,0));      // Blue
	listColors.push_back(Scalar(255,191,0));    // DeepSkyBlue
	listColors.push_back(Scalar(255,255,0));    // Cyan

	detStatus = false;

}

bool DetectionTemporal::initMethod(string cfg_path){

	try{
		Configuration cfg;
		cfg.Load(cfg_path);

		int ACQ_FPS; cfg.Get("ACQ_FPS", ACQ_FPS);

		string acq_bit_depth; cfg.Get("ACQ_BIT_DEPTH", acq_bit_depth);
		EParser<CamBitDepth> cam_bit_depth;
		ACQ_BIT_DEPTH = cam_bit_depth.parseEnum("ACQ_BIT_DEPTH", acq_bit_depth);

		cfg.Get("DET_DOWNSAMPLE_ENABLED", DET_DOWNSAMPLE_ENABLED);

		cfg.Get("DET_SAVE_GEMAP", DET_SAVE_GEMAP);

		cfg.Get("DET_SAVE_POS", DET_SAVE_POS);

		bool ACQ_MASK_ENABLED;

		cfg.Get("ACQ_MASK_ENABLED", ACQ_MASK_ENABLED);

		if(ACQ_MASK_ENABLED){

			string ACQ_MASK_PATH;
			cfg.Get("ACQ_MASK_PATH", ACQ_MASK_PATH);

			frameMask = imread(ACQ_MASK_PATH, CV_LOAD_IMAGE_GRAYSCALE);

			if(!frameMask.data){
				cout << " Can't load the mask from this location : " << ACQ_MASK_PATH;

				throw "Can't load the mask.";
			}

		}

		/// Create local mask to eliminate single white pixels.

        Mat maskTemp(3,3,CV_8UC1,Scalar(255));
        maskTemp.at<uchar>(1, 1) = 0;
        maskTemp.copyTo(localMask);



        cfg.Get("DEBUG", DEBUG);
        cfg.Get("DEBUG_PATH", DEBUG_PATH);


        if(DEBUG) initDebug();

	}catch(exception& e){

		cout << e.what() << endl;
		return false;

	}catch(const char * msg){

		cout << msg << endl;
		return false;

	}

	return true;

}

DetectionTemporal::~DetectionTemporal(){

}

void DetectionTemporal::resetDetection(){

	listGlobalEvents.clear();

}

void DetectionTemporal::initDebug(){

    vector<string> debugSubDir;
    //debugSubDir.push_back("curr");
    //debugSubDir.push_back("diff");
    //debugSubDir.push_back("thsh");
    debugSubDir.push_back("evMp");
    debugSubDir.push_back("motionMap");
    debugSubDir.push_back("rmsinglewhitepix");
    debugSubDir.push_back("show_active_region");
    debugSubDir.push_back("GEMAP");

    for(int i = 0; i< debugSubDir.size(); i++){

        const boost::filesystem::path path(DEBUG_PATH + debugSubDir.at(i));

        if(boost::filesystem::exists(path))
            boost::filesystem::remove_all(path);

        if(!boost::filesystem::exists(path))
            boost::filesystem::create_directories(path);

    }


}

void DetectionTemporal::saveDetectionInfos(string p){


	SaveImg::saveBMP((*GEToSave).getMapEvent(), p + "GEMap");

	/// INFOS DET
    bool infos = true;
    if(infos){

		ofstream infFile;
		string infFilePath = p + "infos.txt";
		infFile.open(infFilePath.c_str());

		infFile << " * AGE              : " << (*GEToSave).getAge() << "\n";
		infFile << " * AGE LAST ELEM    : " << (*GEToSave).getAgeLastElem()<< "\n";
		infFile << " * LINEAR STATE     : " << (*GEToSave).getLinearStatus()<< "\n";
		infFile << " * BAD POS          : " << (*GEToSave).getBadPos()<< "\n";
		infFile << " * GOOD POS         : " << (*GEToSave).getGoodPos()<< "\n";
		infFile << " * Num first frame  : " << (*GEToSave).getNumFirstFrame()<< "\n";
		infFile << " * Num last frame   : " << (*GEToSave).getNumLastFrame()<< "\n";
		//infFile << " * Static Test      : " << (*geToSave).getStaticTestRes();
		infFile << "\n mainPoint : \n";

		for(int i = 0; i < (*GEToSave).mainPts.size(); i++){

			infFile << "(" << (*GEToSave).mainPts.at(i).x << ";"<<  (*GEToSave).mainPts.at(i).y << ")\n";

		}

		infFile.close();
	}


	 /// POSITIONS FILE
	bool recPos = true;
    if(recPos){

        ofstream posFile;
        string posFilePath = p + "positions.txt";
        posFile.open(posFilePath.c_str());

        vector<LocalEvent>::iterator itLE;

        for(itLE = (*GEToSave).LEList.begin(); itLE!=(*GEToSave).LEList.end(); ++itLE){

            Point pos;
            pos = (*itLE).getMassCenter();

			if(DET_DOWNSAMPLE_ENABLED){

                pos*=2;

            }

            string line = Conversion::intToString((*itLE).getNumFrame()) + "               (" + Conversion::intToString(pos.x)  + ";" + Conversion::intToString(pos.y) + ")\n";
            posFile << line;

        }

        // infos

        posFile.close();

    }




}

void DetectionTemporal::generatePixelGrill(int w, int h, int s, Mat mask, int f){

    // generatePixelGrill(1280, 960, 5, frameMask, 5);

    Mat m; mask.copyTo(m);

    if(DET_DOWNSAMPLE_ENABLED){

        w/=2;
        h/=2;

        pyrDown(mask, m, Size(w, h));

    }

    Mat g = Mat(h,w,CV_8UC1, Scalar(0));
    cvtColor(g, g, CV_GRAY2BGR);

    for(int a = 0; a < h/f; a++)

        for(int b = 0; b < w/f; b++){

            rectangle(g, Point((b*f),(a*f)), Point((b*f) + s,(a*f) + s), Scalar(0,255,0));
            //g.at<Vec3b>((int)(a*f) + 3 , (int)(b*f) + 3) = Vec3b(0,0,255);

        }

    g.copyTo(g, m);

    if(DEBUG) SaveImg::saveBMP(g, DEBUG_PATH + "/grill/pixelGrill");

}

// Create n * n region in a frame ( n is a pair value)
void DetectionTemporal::subdivideFrame(vector<Point> &sub, int n, int imgH, int imgW){

	/*

	Example : frame with n = 4 -> 16 subdivisions returned

	|07|08|09|10|
	|06|01|02|11|
	|05|04|03|12|
	|16|15|14|13|

	*/

    int subW = imgW/n;
    int subH = imgH/n;

    Point first = cv::Point((n/2 - 1) * subW, (n/2)*subH);
    Point last = Point(imgW - subW, imgH - subH);

    sub.push_back(first);

    int x = first.x, y = first.y;
    int nbdep = 0,
        nbdepLimit = 1,
        dep = 1; // 1 up
                 // 2 right
                 // 3 down
                 // 4 left

    for(int i = 1; i < n * n; i++){

        if(dep == 1){

            y = y - subH;
            sub.push_back(Point(x,y));
            nbdep ++;
            if(nbdep == nbdepLimit){
                nbdep = 0;
                dep ++;
            }

        }else if(dep == 2){

            x = x + subW;
            sub.push_back(Point(x,y));
            nbdep ++;
            if(nbdep == nbdepLimit){
                nbdep = 0;
                nbdepLimit++;
                dep ++;
            }

        }else if(dep == 3){

            y = y + subH;
            sub.push_back(Point(x,y));
            nbdep ++;
            if(nbdep == nbdepLimit){
                nbdep = 0;
                dep ++;
            }

        }else if(dep == 4){

            x = x - subW;
            sub.push_back(Point(x,y));
            nbdep ++;
            if(nbdep == nbdepLimit){
                nbdep = 0;
                nbdepLimit++;
                dep = 1;
			}
        }
    }
}

bool DetectionTemporal::run(Frame &c, Frame &p){

	if(!initStatus){

		if(DET_DOWNSAMPLE_ENABLED) subdivideFrame(subdivisionPos, 8, c.getImg().rows/2, c.getImg().cols/2);
		else subdivideFrame(subdivisionPos, 8, c.getImg().rows, c.getImg().cols);

		initStatus = true;

	}else{

	    int colorIndex = 0;

		// Height of the frame.
		int imgH = c.getImg().rows;

		// Width of the frame.
		int imgW = c.getImg().cols;

		// Num of the current frame.
		int imgNum = c.getNumFrame();

        // Apply mask.
		Mat currImg, prevImg;
		c.getImg().copyTo(currImg, frameMask);
		p.getImg().copyTo(prevImg, frameMask);

		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%% STEP 1 : FILETRING / THRESHOLDING %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

		// Execution time.
		double tStep1 = (double)getTickCount();

		/// ------------------------------- Downsample frames. --------------------------------------------

		if(DET_DOWNSAMPLE_ENABLED){

            double tdownsample = (double)getTickCount();

			imgH /= 2;
			imgW /= 2;

			pyrDown(currImg, currImg, Size(imgW, imgH));
			pyrDown(prevImg, prevImg, Size(imgW, imgH));

			tdownsample = (((double)getTickCount() - tdownsample)/getTickFrequency())*1000;
            cout << "> Downsample Time : " << tdownsample << endl;

		}

		//if(DEBUG) SaveImg::saveBMP(Conversion::convertTo8UC1(currImg), DEBUG_PATH + "/curr/curr_"+Conversion::intToString(imgNum));

        /// ---------------- Absolute difference between current and previous frame. -----------------------

		double tdiff = (double)getTickCount();

		Mat diff; absdiff(currImg, prevImg, diff);
        Conversion::convertTo8UC1(diff).copyTo(diff);

		tdiff = (((double)getTickCount() - tdiff)/getTickFrequency())*1000;
		cout << "> Difference Time : " << tdiff << endl;

		//if(DEBUG) SaveImg::saveBMP(diff, DEBUG_PATH + "/diff/diff_"+Conversion::intToString(imgNum));

        /// ------------------------ Create binary map of moving pixels. -----------------------------------

		double tmotion = (double)getTickCount();

		// Threshold map.
		Mat mapThreshold = Mat(imgH,imgW, CV_8UC1,Scalar(0));

		// Thresholding diff.
		threshold(diff, mapThreshold, defineThreshold(diff), 255, THRESH_BINARY);

		//if(DEBUG) SaveImg::saveBMP(mapThreshold, DEBUG_PATH + "/thsh/thsh_"+Conversion::intToString(imgNum));

		// Remove single white pixel.

		unsigned char * ptrT;

		Mat black(3,3,CV_8UC1,Scalar(0));

		for(int i = 0; i < mapThreshold.rows; i++){

			ptrT = mapThreshold.ptr<unsigned char>(i);

			for(int j = 0; j < mapThreshold.cols; j++){

				if(ptrT[j] > 0){

					if(j-1 > 0 && j+1 < mapThreshold.cols && i-1 > 0 && i +1 < mapThreshold.rows){

						Mat t = mapThreshold(Rect(j-1, i-1, 3, 3));

						Mat res = t & localMask;

						int n = countNonZero(res);

						if(n == 0){

							if(j-1 > 0 && j+1 < mapThreshold.cols && i-1 > 0 && i +1 < mapThreshold.rows){

								black.copyTo(mapThreshold(Rect(j-1, i-1, 3, 3)));

							}
						}
					}
				}
			}
		}

		//if(DEBUG) SaveImg::saveBMP(mapThreshold, DEBUG_PATH + "/rmsinglewhitepix/thsh_"+Conversion::intToString(imgNum));

		Mat motionMap;

		if(prevThreshMap.data){

			motionMap = mapThreshold & prevThreshMap;

			if(DEBUG) SaveImg::saveBMP(motionMap, DEBUG_PATH + "/motionMap/motionMap_"+Conversion::intToString(imgNum));

			mapThreshold.copyTo(prevThreshMap);

		}else{

		    mapThreshold.copyTo(prevThreshMap);
		    return false;

        }

        tmotion = (((double)getTickCount() - tmotion)/getTickFrequency())*1000;
		cout << "> Motion map Time : " << tmotion << endl;

        /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%% STEP 2 : EVENT EXTRACTION %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

        int subFactor = 5;
        int subW = motionMap.cols / subFactor;
        int subH = motionMap.rows / subFactor;

        Mat show_active_region;
        motionMap.copyTo(show_active_region);
        cvtColor(show_active_region,show_active_region,CV_GRAY2BGR);

        Mat eventMap = Mat(imgH,imgW, CV_8UC3,Scalar(0,0,0));

        double te = (double)getTickCount();

        vector<Point>::iterator itReg;
        vector<Point> activeRegions;
        vector<LocalEvent>::iterator it_LE;
		vector<LocalEvent> listLocalEvents;

		// Loop each subregion to find if there is an object.
        for(int i = 0; i < motionMap.rows / 5 ; i++){

            for(int j = 0; j < motionMap.cols / 5 ; j++){

                 // Extract subregion from binary motion pixel map.
                 Mat s = motionMap(Rect(j*5, i*5, 5, 5));

                 // At least two white pixels are present.
                 if(countNonZero(s) >= 2){

                    // The subregion become active.
                    activeRegions.push_back(Point(j*5, i*5));

                    //cout << "Current active region : (" << j*5 << ";" << i*5 << ")" << endl;

                    bool rMatch = true;

                    // Can the subregion be linked with an existing localEvent ?
                    for(it_LE = listLocalEvents.begin(); it_LE!=listLocalEvents.end(); ++it_LE){

                        bool match = false;

                        // The subregion are checked from top to bottom and left to right.
                        // We check if the current localEvent can have a previous checked neighbour of the current subregion.
                        // Expected position of the north region.
                        Point rN = Point((j*5), (i*5)-5);
                        // Expected position of the west region.
                        Point rW = Point((j*5)-5, (i*5));
                        // Expected position of the north west region.
                        Point rNW = Point((j*5)-5, (i*5)-5);
                        // Expected position of the north east region.
                        Point rNE = Point((j*5)+5, (i*5)-5);

                        // Check if the expected region in the neighbourhood are available.
                        bool checkN = false;
                        bool checkW = false;
                        bool checkNW = false;
                        bool checkNE = false;

                        if(rN.y >= 0) checkN = true;
                        if(rW.x >= 0) checkW = true;
                        if(rNW.y >= 0 && rNW.x >= 0) checkNW = true;
                        if(rNE.y >= 0 && rNE.x < imgW) checkNE = true;

                        // Get regions of the current localEvent.
                        for(int a = 0; a < (*it_LE).leRoiList.size(); a++){

                            // The current subregion can have a north region.
                            if(checkN){

                               // If north region found.
                               if(rN == (*it_LE).leRoiList.at(a)){

                                    // The current subregion is shifted by 2 pixel in high to ckeck if the north region bottom match.
                                    Mat sN; (*it_LE).subdivision.at(a).copyTo(sN);
                                    Mat sNcheck = sN(Rect(0, 3, 5, 2));
                                    Mat sCcheck = s(Rect(0, 0, 5, 2));

                                    Mat res = sNcheck & sCcheck;

                                    if(countNonZero(res)>0){

                                        match = true;   // Add the current subregion to this localEvent
                                        rMatch = false; // No create new localEvent
                                        break;

                                    }

                               }

                            }

                            if(checkW){

                               if(rW == (*it_LE).leRoiList.at(a)){

                                    Mat sW; (*it_LE).subdivision.at(a).copyTo(sW);
                                    Mat sWcheck = sW(Rect(3, 0, 2, 5));
                                    Mat sCcheck = s(Rect(0, 0, 2, 5));

                                    Mat res = sWcheck & sCcheck;

                                    if(countNonZero(res)>0){

                                        match = true;
                                        rMatch = false;
                                        break;

                                    }

                               }

                            }

                            if(checkNW){

                               if(rNW == (*it_LE).leRoiList.at(a)){

                                    Mat sNW; (*it_LE).subdivision.at(a).copyTo(sNW);
                                    Mat sNWcheck = sNW(Rect(3, 3, 2, 2));
                                    Mat sCcheck = s(Rect(0, 0, 2, 2));

                                    Mat res = sNWcheck & sCcheck;

                                    if(countNonZero(res)>0){

                                        match = true;
                                        rMatch = false;
                                        break;

                                    }

                               }

                            }

                            if(checkNE){

                                if(rNE == (*it_LE).leRoiList.at(a)){

                                    Mat sNE; (*it_LE).subdivision.at(a).copyTo(sNE);
                                    Mat sNEcheck = sNE(Rect(0, 3, 2, 2));
                                    Mat sCcheck = s(Rect(3, 0, 2, 2));

                                    Mat res = sNEcheck & sCcheck;

                                    if(countNonZero(res)>0){

                                        match = true;
                                        rMatch = false;
                                        break;

                                    }

                               }

                            }

                        }

                        // If the current subregion has found a neighbour in a localEvent
                        if(match){

                            cout << "Add region to an existing LE" << endl;

                            // Add subregion position.
                            (*it_LE).leRoiList.push_back(Point(j*5, i*5));
                            // Add a copy of the subregion.
                            (*it_LE).subdivision.push_back(s);
                            // Update center of mass.
                            (*it_LE).computeMassCenter();

                            // Update eventMap
                            Mat roi(5, 5, CV_8UC3, (*it_LE).getColor());
                            roi.copyTo(eventMap(Rect(j*5, i*5,5,5)));

                        }

                    }

                    if(rMatch){

                         cout << "Create new LE" << endl;

                         LocalEvent newLE(listColors.at(colorIndex), Point(j*5, i*5), s);
                         // Update center of mass
                         newLE.computeMassCenter();
                         // Add it in the list of localEvent
                         listLocalEvents.push_back(newLE);

                         // Update eventMap with the color of the new localEvent group
                         Mat roi(5, 5, CV_8UC3, listColors.at(colorIndex));
                         roi.copyTo(eventMap(Rect(j*5, i*5,5,5)));

                         colorIndex++;

                    }

                    rectangle(show_active_region, Point(j*5, i*5), Point(j*5 +5, i*5+5), Scalar(0,255,0));

                 }

             }
        }

        te = (((double)getTickCount() - te)/getTickFrequency())*1000;
		cout << "> Extraction time : " << te << endl;

        if(DEBUG) SaveImg::saveBMP(show_active_region, DEBUG_PATH + "/show_active_region/show_active_region_"+Conversion::intToString(imgNum));

        if(DEBUG) SaveImg::saveBMP(eventMap, DEBUG_PATH + "/evMp/evMp_"+Conversion::intToString(imgNum));


		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%% STEP 3 : ATTACH LE TO GE OR CREATE NEW ONE %%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

		// Iterator on list of global event.
		vector<GlobalEvent>::iterator itGE;
		// Iterator on list of local event.
		vector<LocalEvent>::iterator itLE;

		double tStep3 = (double)getTickCount();

		itLE = listLocalEvents.begin();

        cout << "Manage LocalEvent" << endl;

        // Parcours de la liste des local events
		while(itLE != listLocalEvents.end()){

            bool LELinked = false;

			(*itLE).setNumFrame(c.getNumFrame());

            vector<GlobalEvent>::iterator itGE_selected;
            bool ge_selected = false;

            // Can the current localEvent be added to an existing GlobalEvent ?
			for(itGE = listGlobalEvents.begin(); itGE!=listGlobalEvents.end(); ++itGE){

                // Loop each subregion of the current localEvent to extract an extended same region in the GlobalEvent map.
                for(int a = 0; a < (*itLE).leRoiList.size(); a++){

                    // Origin of the dilated same region.
                    Point o = Point((*itLE).leRoiList.at(a).x -2 , (*itLE).leRoiList.at(a).y -2);
                    int sizeX = 9;
                    int sizeY = 9;

                    // Handle bottom right situation.
                    if(o.y + sizeY >= imgH && o.x + sizeX >= imgW){

                        sizeY -= 2;
                        sizeX -= 2;

                    }

                    // Handle top left situation.
                    else if(o.x < 0 && o.y < 0){

                        sizeY -= 2;
                        sizeX -= 2;

                        o.x += 2;
                        o.y += 2;

                    }

                    // Handle top right situation.
                    else if(o.x + sizeX >= imgW  && o.y < 0){

                        sizeY -= 2;
                        sizeX -= 2;

                        o.y += 2;

                    }

                    // Handle bottom left situation.
                    else if(o.x < 0 && o.y + sizeY > imgH){

                        sizeY -= 2;
                        sizeX -= 2;

                        o.x += 2;

                    }

                    // Handle left situation.
                    else if(o.x < 0){

                        o.x +=2;
                        sizeX -= 2;

                    }

                    // Handle top situation.
                    else if(o.y < 0 ){

                        o.y +=2;
                        sizeY -= 2;

                    }

                    // Handle right situation.
                    else if(o.x + sizeX >= imgW ){

                        sizeX -= 2;

                    }

                    // Handle bottom situation.
                    else if(o.y + sizeY >= imgH ){

                        sizeY -= 2;

                    }

                    // Extraction of the dilated region in the global event map.
                    Mat r = (*itGE).getMapEvent()(Rect(o.x, o.y, sizeX, sizeY));

                    if(countNonZero(r) > 0){

                        LELinked = true;

                        // The current LE has found a previous global event.
                        if(ge_selected){

                            // Choose the older global event.
                            if((*itGE).getAge() > (*itGE_selected).getAge()){

                                itGE_selected = itGE;

                            }

                        }else{

                            itGE_selected = itGE;
                            ge_selected = true;

                        }

                        break;

                    }
                }
			}

            // Add current LE to an existing GE
            if(ge_selected){

                // Add LE.
                (*itGE_selected).addLE((*itLE));
                // Flag to indicate that a local event has been added.
                (*itGE_selected).setNewLEStatus(true);
                // reset age of the last local event received by the global event.
                (*itGE_selected).setAgeLastElem(0);

            }

			// The current LE has not been linked. It became a new GE.
			if(!LELinked && listGlobalEvents.size() <= 20){

   				GlobalEvent newGE(c.getAcqDateMicro(), c.getNumFrame(), currImg.rows, currImg.cols, Scalar(0,0,0));
				newGE.addLE((*itLE));

				//Add the new globalEvent to the globalEvent's list
				listGlobalEvents.push_back(newGE);

			}

            // Delete the current localEvent.
			itLE = listLocalEvents.erase(itLE);

		}

		tStep3 = (((double)getTickCount() - tStep3)/getTickFrequency())*1000;
		cout << "> Time 3) : " << tStep3 << endl;

		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%% STEP 4 : UPDATE GLOBAL EVENTS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

		double  t4 = (double)getTickCount();

		itGE = listGlobalEvents.begin();

		bool saveSignal = false;

		while(itGE != listGlobalEvents.end()){

            // Increment age.
			(*itGE).setAge((*itGE).getAge() + 1);

            // The current global event has not received a new local event.
			if(!(*itGE).getNewLEStatus()){

                // Increment age without any new local event.
				(*itGE).setAgeLastElem((*itGE).getAgeLastElem()+1);

			}else{

				(*itGE).setNumLastFrame(c.getNumFrame());
				(*itGE).setNewLEStatus(false);

			}

			// CASE 1 : Finished event.
			if((*itGE).getAgeLastElem() > 5){

                // Linear profil ? Minimum duration respected ?
				if(((*itGE).LEList.size() >= 10 && (*itGE).getLinearStatus())){

					GEToSave = itGE;

                    cout  << endl << endl << endl << endl<< "DETECTION !!!!!!!!!!!!!!!!!!!!!!" << endl << endl << endl;
                    saveSignal = true;

                    break;


				}else{

					// Delete the event.
					itGE = listGlobalEvents.erase(itGE);

				}

			//CASE 2 : Not finished event.
			}else{

                // Too long event ? or not linear ?
				if( (*itGE).getAge() > 400 ||
					!(*itGE).getLinearStatus()){

					// Delete the event.
					itGE = listGlobalEvents.erase(itGE);

                // Let the GE alive.
				}else{

					++itGE;

				}
			}
		}

		t4 = (((double)getTickCount() - t4)/getTickFrequency())*1000;
		cout << "Time 4 : " << t4 << endl;

		/// GEMAP debug

		Mat GEMAP = Mat(480,640, CV_8UC3,Scalar(0,0,0));

		for(itGE = listGlobalEvents.begin(); itGE != listGlobalEvents.end(); ++itGE){

            for(itLE = (*itGE).LEList.begin(); itLE != (*itGE).LEList.end(); ++itLE){


                for(int a = 0; a<(*itLE).leRoiList.size(); a++){


                    Mat temp = Mat(5,5,CV_8UC3,(*itLE).getColor());
                    temp.copyTo(GEMAP(Rect((*itLE).leRoiList.at(a).x, (*itLE).leRoiList.at(a).y, 5, 5)));

                 }

             }

		}

		if(DEBUG) SaveImg::saveBMP(GEMAP, DEBUG_PATH + "/GEMAP/GEMAP_"+Conversion::intToString(imgNum));

		return saveSignal;

	}

	return false;

}

int DetectionTemporal::defineThreshold(Mat i){

    Scalar imgMean;
    Scalar imgStddev;

    meanStdDev(i, imgMean, imgStddev);

    cout << "stdev : " << 5*(int)(cvRound(imgStddev.val[0])+1) << endl;

    return 5*(int)(cvRound(imgStddev.val[0])+1);

}
