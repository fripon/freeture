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

	roiSize[0] = 10;
	roiSize[1] = 10;

	nbGE = 5;

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
		if(ACQ_BIT_DEPTH == MONO_8){

			Mat maskTemp(3,3,CV_8UC1,Scalar(255));
			maskTemp.at<uchar>(1, 1) = 0;
			maskTemp.copyTo(localMask);

		}else if(ACQ_BIT_DEPTH == MONO_12){

			Mat maskTemp(3,3,CV_16UC1,Scalar(4095));
			maskTemp.at<ushort>(1, 1) = 0;
			maskTemp.copyTo(localMask);

		}

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

// Remove subdivisions which are almost totally masked (mask image).
void DetectionTemporal::sortSubdivision(unsigned char percentage){




}

void DetectionTemporal::resetDetection(){

	listGlobalEvents.clear();

}

void DetectionTemporal::saveDetectionInfos(string p){

	SaveImg::saveBMP((*geToSave).getDirMap2(), p + "DirMap2");

	SaveImg::saveBMP((*geToSave).getMapEvent(), p + "GEMap");
        
    SaveImg::saveBMP((*geToSave).getDirMap(), p + "DirMap");

	/// INFOS DET
    bool infos = true;
    if(infos){

		ofstream infFile;
		string infFilePath = p + "infos.txt";
		infFile.open(infFilePath.c_str());
	
		infFile << " * AGE              : " << (*geToSave).getAge() << "\n";
		infFile << " * AGE LAST ELEM    : " << (*geToSave).getAgeLastElem()<< "\n";
		infFile << " * LINEAR STATE     : " << (*geToSave).getLinearStatus()<< "\n";
		infFile << " * BAD POS          : " << (*geToSave).getBadPos()<< "\n";
		infFile << " * GOOD POS         : " << (*geToSave).getGoodPos()<< "\n";
		infFile << " * Num first frame  : " << (*geToSave).getNumFirstFrame()<< "\n";
		infFile << " * Num last frame   : " << (*geToSave).getNumLastFrame()<< "\n";
		//infFile << " * Static Test      : " << (*geToSave).getStaticTestRes();
		infFile << "\n mainPoint : \n";

		for(int i = 0; i < (*geToSave).mainPoints.size(); i++){

			infFile << "(" << (*geToSave).mainPoints.at(i).x << ";"<<  (*geToSave).mainPoints.at(i).y << ")\n";

		}

		infFile << "\n distance : \n";

			for(int i = 0; i < (*geToSave).dist.size(); i++){

			infFile << (*geToSave).dist.at(i)<< "\n";

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

        for(itLE = (*geToSave).LEList.begin(); itLE!=(*geToSave).LEList.end(); ++itLE){

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
	cout << "Run detection in DetectionTemporalMovement" << endl;
	if(!initStatus){

		frameHeight = c.getImg().rows;
		frameWidth = c.getImg().cols;

		if(DET_DOWNSAMPLE_ENABLED) subdivideFrame(subdivisionPos, 8, frameHeight/2, frameWidth/2);
		else subdivideFrame(subdivisionPos, 8, frameHeight, frameWidth);

		initStatus = true;
		
	}else{

		bool saveDiff       = true;
		bool saveThresh     = true;
		bool saveThresh2    = true;
		bool saveEventmap   = true;
		bool saveCurr       = true;
		bool saveRes        = true;

		// Height of the frame.
		int imgH = c.getImg().rows;

		// Width of the frame.
		int imgW = c.getImg().cols;
		Mat currImg, prevImg;

		Mat tempCurr, tempPrev, tempMask;
		cout << "Run detection !" << endl;
		cout <<"size curr : " << c.getImg().rows << "x" << c.getImg().cols << endl;
		cout <<"size prev : " << p.getImg().rows << "x" << p.getImg().cols << endl;
		cout <<"size mask : " <<  frameMask.rows << "x" << frameMask.cols << endl;
		c.getImg().copyTo(tempCurr, frameMask);
		p.getImg().copyTo(tempPrev, frameMask);
		frameMask.copyTo(tempMask);
		
		// List of localEvent objects.
		vector <LocalEvent> listLocalEvents;

		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%% STEP 1 : FILETRING / THRESHOLDING %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

		// Execution time.
		double tStep1 = (double)getTickCount();

		double tdownsample = (double)getTickCount();

		// According to DET_DOWNSAMPLE_ENABLED's parameter in configuration file.
		if(DET_DOWNSAMPLE_ENABLED){

			imgH /= 2;
			imgW /= 2;

			double tdowncurr = (double)getTickCount();
			pyrDown( tempCurr, currImg, Size( tempCurr.cols/2, tempCurr.rows/2 ) );
			tdowncurr = (((double)getTickCount() - tdowncurr)/getTickFrequency())*1000;
			//cout << "> tdowncurr Time : " << tdowncurr << endl;

			double tdownprev = (double)getTickCount();
			pyrDown( tempPrev, prevImg, Size( tempPrev.cols/2, tempPrev.rows/2 ) );
			tdownprev = (((double)getTickCount() - tdownprev)/getTickFrequency())*1000;
			//cout << "> tdownprev Time : " << tdownprev << endl;

			double tdownmask = (double)getTickCount();
			pyrDown( tempMask, tempMask, Size( frameMask.cols/2, frameMask.rows/2 ) );
			tdownmask = (((double)getTickCount() - tdownmask)/getTickFrequency())*1000;
			//cout << "> tdownmask Time : " << tdownmask << endl;

		}else{

			tempCurr.copyTo(currImg);
			tempPrev.copyTo(prevImg);

		}

		if(saveCurr)
			SaveImg::saveBMP(c.getImg(), "C:/Users/Yoan/Documents/GitHub/freeture/debug/curr/curr_"+Conversion::intToString(c.getNumFrame()));

		tdownsample = (((double)getTickCount() - tdownsample)/getTickFrequency())*1000;
		//cout << "> Downsample Time : " << tdownsample << endl;

		//cout << "> Compute difference." << endl;

		double tdiff = (double)getTickCount();

		// Difference between current and previous frame.
		Mat diff;
		absdiff(currImg, prevImg, diff);

		if(ACQ_BIT_DEPTH == MONO_12){

			Conversion::convertTo8UC1(diff).copyTo(diff);
			Conversion::convertTo8UC1(localMask).copyTo(localMask);

		}

		tdiff = (((double)getTickCount() - tdiff)/getTickFrequency())*1000;
		cout << "> Difference Time : " << tdiff << endl;

		if(saveDiff)
			SaveImg::saveBMP(diff, "C:/Users/Yoan/Documents/GitHub/freeture/debug/diff/diff_"+Conversion::intToString(c.getNumFrame()));

		//cout << "> Thresholding" << endl;

		double tthresh = (double)getTickCount();

		// Threshold map.
		Mat mapThreshold = Mat(imgH,imgW, CV_8UC1,Scalar(0));

		// Thresholding diff.
		threshold(diff, mapThreshold, defineThreshold(diff, tempMask), 255, THRESH_BINARY);

		tthresh = (((double)getTickCount() - tthresh)/getTickFrequency())*1000;
		cout << "> Threshold Time : " << tthresh << endl;

		if(saveThresh)
			SaveImg::saveBMP(mapThreshold, "C:/Users/Yoan/Documents/GitHub/freeture/debug/thresh1/thresh_"+Conversion::intToString(c.getNumFrame()));

		double tthresh2 = (double)getTickCount();

		// Remove single white pixel.
		unsigned char * ptrT;

		Mat black(3,3,CV_8UC1,Scalar(0));

		for(int i = 0; i < imgH; i++){

			ptrT = mapThreshold.ptr<unsigned char>(i);

			for(int j = 0; j < imgW; j++){

				if(ptrT[j] > 0){

					if(j-1 > 0 && j+1 < imgW && i-1 > 0 && i +1 < imgH){

						Mat t = mapThreshold(Rect(j-1, i-1, 3, 3));

						Mat res = t & localMask;

						int n = countNonZero(res);

						if(n == 0){

							if(j-1 > 0 && j+1 < imgW && i-1 > 0 && i +1 < imgH){

								black.copyTo(mapThreshold(Rect(j-1, i-1, 3, 3)));

							}
						}
					}
				}
			}
		}

		tthresh2 = (((double)getTickCount() - tthresh2)/getTickFrequency())*1000;
		//cout << "> Eliminate single white pixels Time : " << tthresh2 << endl;

		Mat threshANDprev;

		if(prevthresh.data){

			threshANDprev = mapThreshold & prevthresh;

			if(saveRes)
				SaveImg::saveBMP(threshANDprev, "C:/Users/Yoan/Documents/GitHub/freeture/debug/res/resET_"+Conversion::intToString(c.getNumFrame()));

		}

		mapThreshold.copyTo(prevthresh);

		tStep1 = (((double)getTickCount() - tStep1)/getTickFrequency())*1000;
		cout << "> Time 1) : " << tStep1 << endl << endl;

		if(saveThresh2)
			SaveImg::saveBMP(mapThreshold, "C:/Users/Yoan/Documents/GitHub/freeture/debug/thresh2/thresh_"+Conversion::intToString(c.getNumFrame()));

		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% STEP 2 : FIND LOCAL EVENT %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

		cout << "### Starting step 2. ###" <<endl;

		// Execution time.
		double tStep2 = (double)getTickCount();

		// Event map for the current frame.
		Mat eventMap = Mat(imgH,imgW, CV_8UC3,Scalar(0,0,0));

		if(threshANDprev.data){

			// Try to complete existing GE.

			// Iterator on list of sub-regions.
			vector<Point>::iterator itR;

			// Search new LE if maxNbLE value has not been reached.
			for(itR = subdivisionPos.begin(); itR != subdivisionPos.end(); ++itR){

				 Mat subdivision = threshANDprev(Rect((*itR).x, (*itR).y, imgW/8, imgH/8));

				 searchROI( subdivision, threshANDprev, eventMap, listLocalEvents, imgH, imgW, imgW/8, imgH/8, (*itR), 20);
			
			}
		}

		cout << "> LE number : " << listLocalEvents.size() << endl;

		tStep2 = (((double)getTickCount() - tStep2)/getTickFrequency())*1000;
		cout << "> Time 2) : " << tStep2 << endl << endl;

		if(saveEventmap)
			SaveImg::saveBMP(eventMap, "C:/Users/Yoan/Documents/GitHub/freeture/debug/evMap/event"+Conversion::intToString(c.getNumFrame()));

		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%% STEP 3 : ATTACH LE TO GE OR CREATE NEW ONE %%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		
		cout << "### Starting step 3. ###" <<endl;

		// Iterator on list of global event.
		vector<GlobalEvent>::iterator itGE;
		// Iterator on list of local event.
		vector<LocalEvent>::iterator itLE;

		bool LELinked = false;
		bool LEAdded = false;

		int nbNewGe = 0;

		double tStep3 = (double)getTickCount();

		itLE = listLocalEvents.begin();

		while(itLE != listLocalEvents.end()){

			(*itLE).setNumFrame(c.getNumFrame());

			for(itGE = listGlobalEvents.begin(); itGE!=listGlobalEvents.end(); ++itGE){

				Mat res = (*itLE).getMap() & (*itGE).getMapEvent();

				int nbPixNonZero = countNonZero(res);

				if( nbPixNonZero > 0 ){

					LELinked = true;

					// Add the current LE to the current GE.
					(*itGE).addLE((*itLE));

					(*itGE).setNewLEStatus(true);

					(*itGE).setAgeLastElem(0);

				}
			}

			// The current LE has not been linked. It became a new GE.
			if(!LELinked && listGlobalEvents.size() <= 20){

				GlobalEvent newGE(c.getAcqDateMicro(), c.getNumFrame(), currImg.rows, currImg.cols);
				newGE.addLE((*itLE));

				//Add the new globalEvent to the globalEvent's list
				listGlobalEvents.push_back(newGE);

				nbNewGe++;

			}

			LELinked = false;

			itLE = listLocalEvents.erase(itLE);

		}

		cout << "> new GE : " << nbNewGe << endl;

		// Increment age of GE
		for(itGE = listGlobalEvents.begin(); itGE != listGlobalEvents.end(); ++itGE){

			(*itGE).setAge((*itGE).getAge() + 1);

			if(!(*itGE).getNewLEStatus()){

				(*itGE).setAgeLastElem((*itGE).getAgeLastElem()+1);

			}else{

				Mat temp;
				currImg.copyTo(temp);
				cvtColor(temp, temp, CV_GRAY2BGR);
				LocalEvent tempLE = (*itGE).LEList.back();
				circle(temp, tempLE.getMassCenter(), 3, Scalar(0,255,0), CV_FILLED, 8, 0);
				(*itGE).eventBuffer.push_back(temp);
				(*itGE).setNumLastFrame(c.getNumFrame());

			}

			(*itGE).setNewLEStatus(false);

		}

		tStep3 = (((double)getTickCount() - tStep3)/getTickFrequency())*1000;
		cout << "> Time 3) : " << tStep3 << endl;
		
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%% STEP 4 : MANAGE LIST GLOBAL EVENT %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

		cout << "### Starting step 4. ###" <<endl;
		
		double  tStep4 = (double)getTickCount();

		cout << "> GE number before management : " << listGlobalEvents.size() << endl;

		itGE = listGlobalEvents.begin();

		while (itGE != listGlobalEvents.end()){

			// CASE 1 : Finished event.
			if((*itGE).getAgeLastElem() > 5){

				if( ((*itGE).LEList.size() >= 10 && (*itGE).getLinearStatus() && !(*itGE).getGeStatic()) ||
					((*itGE).LEList.size() >= 10 && !(*itGE).getLinearStatus() && (*itGE).continuousGoodPos(10) && !(*itGE).getGeStatic())){

					//nbDet++;
					geToSave = itGE;

					cout << endl << endl<< endl<< endl << endl<< endl<< endl << endl<< endl; 
						cout << "!!!!!!!!!!!!!!!!!DETECTION 11 !!!!!!!!!!!!!!!!! "<< endl;
						cout << endl << endl<< endl<< endl << endl<< endl<< endl << endl<< endl; 


					return true;

				}else{

					// Delete the event.
					itGE = listGlobalEvents.erase(itGE);

				}

			//CASE 2 : Not finished event.
			}else{

				if( (*itGE).getAge() > 400 ||
					(*itGE).getBadPos()>= 1 && !(*itGE).continuousGoodPos(10)){

					// Delete the event.
					itGE = listGlobalEvents.erase(itGE);

				}else if((c.getFrameRemaining()< 10 && c.getFrameRemaining() != 0)){      // No more frames soon (in video or frames)

					 if((*itGE).LEList.size() >= 10 && (*itGE).getLinearStatus() && !(*itGE).getGeStatic()){
						 			cout << endl << endl<< endl<< endl << endl<< endl<< endl << endl<< endl; 
						cout << "!!!!!!!!!!!!!!!!!DETECTION 12  !!!!!!!!!!!!!!!!! "<< endl;
						cout << endl << endl<< endl<< endl << endl<< endl<< endl << endl<< endl; 
						geToSave = itGE;
						return true;

					 }else{

						itGE = listGlobalEvents.erase(itGE);

					}

				}else{

					++itGE;

				}
			}
		}

		tStep4 = (((double)getTickCount() - tStep4)/getTickFrequency())*1000;
		cout << "Time step4 : " << tStep4 << endl;

		cout << "> GE number : " << listGlobalEvents.size() << "/" << nbGE << endl;
		
	}

	return false;

}

int DetectionTemporal::getNumFirstEventFrame(){

	return (*geToSave).getNumFirstFrame();

}

string DetectionTemporal::getDateEvent(){
	cout <<"(*geToSave).getDate() : " <<(*geToSave).getDate() << endl;
	return (*geToSave).getDate();
}

int DetectionTemporal::getNumLastEventFrame(){

	return (*geToSave).getNumLastFrame();

}

Point DetectionTemporal::roiPositionToFramePosition(Point roiPix, Point newOrigine, Point framePix){

    Point resPix = Point(0,0);

    if(roiPix.x > newOrigine.x){

        resPix.x = roiPix.x - newOrigine.x;

    }else if(roiPix.x < newOrigine.x){

        resPix.x = -(newOrigine.x - roiPix.x);

    }

    if(roiPix.y > newOrigine.y){

        resPix.y = -(roiPix.y - newOrigine.y);

    }else if(roiPix.y < newOrigine.y){

        resPix.y = newOrigine.y - roiPix.y;

    }

    framePix.x += resPix.x;
    framePix.y += resPix.y;

    return framePix;

}

vector<Scalar> DetectionTemporal::getColorInEventMap(Mat &eventMap, Point roiCenter){

    // Same ROI but in the eventmap.
    Mat roi;

    // ROI extraction from the eventmap.
    eventMap(Rect(roiCenter.x-roiSize[0]/2, roiCenter.y-roiSize[1]/2, roiSize[0], roiSize[1])).copyTo(roi);

    unsigned char *ptr = (unsigned char*)roi.data;

    int cn = roi.channels();

    vector<Scalar> listColor;

    bool exist = false;

    for(int i = 0; i < roi.rows; i++){

        for(int j = 0; j < roi.cols; j++){

            Scalar bgrPixel;
            bgrPixel.val[0] = ptr[i*roi.cols*cn + j*cn + 0]; // B
            bgrPixel.val[1] = ptr[i*roi.cols*cn + j*cn + 1]; // G
            bgrPixel.val[2] = ptr[i*roi.cols*cn + j*cn + 2]; // R

            if(bgrPixel.val[0] != 0 || bgrPixel.val[1] != 0 || bgrPixel.val[2] != 0){

                for(int k = 0; k < listColor.size(); k++){

                    if(bgrPixel == listColor.at(k)){

                        exist = true;
                        break;

                    }
                }

                if(!exist)
                    listColor.push_back(bgrPixel);

                exist = false;

            }
        }
    }

    return listColor;

}

int DetectionTemporal::defineThreshold(Mat i , Mat m){

    Scalar imgMean;
    Scalar imgStddev;

    meanStdDev(i, imgMean, imgStddev, m);

    cout << "stdev : " << 5*(int)(cvRound(imgStddev.val[0])+1) << endl;

    return 5*(int)(cvRound(imgStddev.val[0])+1);

}

void DetectionTemporal::colorInBlack(int j, int i, int areaPosX, int areaPosY, Point areaPosition, Mat &area, Mat &frame){

    int height  = roiSize[1];
    int width   = roiSize[0];
    int posX    = j - roiSize[0]/2;
    int posY    = i - roiSize[1]/2;

    if(j - roiSize[0]/2 < 0){

        width = j + roiSize[0]/2;
        posX = 0;

    }else if(j + roiSize[0]/2 > areaPosX){

        width = areaPosX - j + roiSize[0]/2;

    }

    if(i - roiSize[1]/2 < 0){

        height = i + roiSize[1];
        posY = 0;


    }else if(i + roiSize[1]/2 > areaPosY){

        height = areaPosY - i + roiSize[0]/2;

    }

    // Color roi in black in the current area.
    Mat roiBlackRegion(height,width,CV_8UC1,Scalar(0));
    roiBlackRegion.copyTo(area(Rect(posX, posY, width, height)));

    // Color roi in black in thresholded frame.
    Mat roiBlack(roiSize[1],roiSize[0],CV_8UC1,Scalar(0));
    roiBlack.copyTo(frame(Rect(areaPosition.x + j-roiSize[0]/2, areaPosition.y + i-roiSize[1]/2,roiSize[0],roiSize[1])));

}

void DetectionTemporal::searchROI( Mat &area,                      // One frame's region
                Mat &frame,                     // Thresholded frame
                Mat &eventMap,
                vector<LocalEvent> &listLE,     // List of local events
                int imgH,                       // Frame height
                int imgW,                       // Frame width
                int areaPosX,                   // Size of region where to search ROI
                int areaPosY,                   // Size of region where to search ROI
                Point areaPosition,
                int maxNbLE){            // Position of the area in the frame (up top corner)

    int situation;

    unsigned char * ptr;

    for(int i = 0; i < area.rows; i++){

        ptr = area.ptr<unsigned char>(i);

        for(int j = 0; j < area.cols; j++){

            if((int)ptr[j] > 0){

                // Check if ROI is not out of range in the frame
                if((areaPosition.y + i - roiSize[1]/2 > 0) && (areaPosition.y + i + roiSize[1]/2 < imgH) && ( areaPosition.x + j-roiSize[0]/2 > 0) && (areaPosition.x + j + roiSize[0]/2 < imgW)){

                    // Get colors in eventMap at the current ROI location.
                    vector<Scalar> listColorInRoi = getColorInEventMap(eventMap, Point(areaPosition.x + j, areaPosition.y + i));

                    if(listColorInRoi.size() == 0)  situation = 0;  // black color      = create a new local event
                    if(listColorInRoi.size() == 1)  situation = 1;  // one color        = add the current roi to an existing local event
                    if(listColorInRoi.size() >  1)  situation = 2;  // several colors   = make a decision

                    switch(situation){

                        case 0 :

                            {
                                vector<LocalEvent>::iterator it;

                                bool addNew = false;

                               /* for (it = listLE.begin(); it != listLE.end(); ++it){

                                    Point com = (*it).getMassCenter();

                                    if(sqrt(pow((areaPosition.x + j) - (com.x ),2) + pow((areaPosition.y + i) - (com.y),2)) < 40){

                                        // Add the current roi.
                                        (*it).LE_Roi.push_back(Point(areaPosition.x + j, areaPosition.y + i));

                                        // Set local event 's map
                                        Mat tempMat = (*it).getMap();
                                        Mat roiTemp(roiSize[1],roiSize[0],CV_8UC1,Scalar(255));
                                        roiTemp.copyTo(tempMat(Rect(areaPosition.x + j - roiSize[0]/2, areaPosition.y + i - roiSize[1]/2, roiSize[0], roiSize[1])));
                                        (*it).setMap(tempMat);

                                        // Update center of mass
                                        (*it).computeMassCenterWithRoi();

                                        // Update eventMap with the color of the new localEvent group
                                        Mat roi(roiSize[1], roiSize[0], CV_8UC3, (*it).getColor());
                                        roi.copyTo(eventMap(Rect(areaPosition.x + j-roiSize[0]/2, areaPosition.y + i-roiSize[1]/2,roiSize[0],roiSize[1])));

                                        colorInBlack(roiSize, j, i, areaPosX, areaPosY, areaPosition, area, frame);

                                        addNew = true;

                                        break;
                                    }

                                }*/


                                if(listLE.size() < 20 && !addNew){

                                    // Create new localEvent object


                                    LocalEvent newLocalEvent(listColors.at(listLE.size()), Point(areaPosition.x + j, areaPosition.y + i), imgH, imgW, roiSize);
                                    newLocalEvent.computeMassCenterWithRoi();

                                    // And add it in the list of localEvent
                                    listLE.push_back(newLocalEvent);

                                    // Update eventMap with the color of the new localEvent group
                                    Mat roi(roiSize[1], roiSize[0], CV_8UC3, listColors.at(listLE.size()));
                                    roi.copyTo(eventMap(Rect(areaPosition.x + j-roiSize[0]/2, areaPosition.y + i-roiSize[1]/2,roiSize[0],roiSize[1])));

                                    colorInBlack(j, i, areaPosX, areaPosY, areaPosition, area, frame);

                                }
                            }

                            break;

                        case 1 :

                            {

                                vector<LocalEvent>::iterator it;

                                for (it=listLE.begin(); it!=listLE.end(); ++it){

                                    // Try to find a local event which has the same color.
                                    if((*it).getColor() == listColorInRoi.at(0)){

                                        // Add the current roi.
                                        (*it).LE_Roi.push_back(Point(areaPosition.x + j, areaPosition.y + i));

                                        // Set local event 's map
                                        Mat tempMat = (*it).getMap();
                                        Mat roiTemp(roiSize[1],roiSize[0],CV_8UC1,Scalar(255));
                                        roiTemp.copyTo(tempMat(Rect(areaPosition.x + j - roiSize[0]/2, areaPosition.y + i - roiSize[1]/2, roiSize[0], roiSize[1])));
                                        (*it).setMap(tempMat);

                                        // Update center of mass
                                        (*it).computeMassCenterWithRoi();

                                        // Update eventMap with the color of the new localEvent group
                                        Mat roi(roiSize[1], roiSize[0], CV_8UC3, listColorInRoi.at(0));
                                        roi.copyTo(eventMap(Rect(areaPosition.x + j-roiSize[0]/2, areaPosition.y + i-roiSize[1]/2,roiSize[0],roiSize[1])));

                                        colorInBlack( j, i, areaPosX, areaPosY, areaPosition, area, frame);

                                        break;

                                    }
                                }
                            }

                            break;

                        case 2 :

                            {
                                vector<LocalEvent>::iterator it;
                                vector<LocalEvent>::iterator itLEbase;
                                it = listLE.begin();

                                vector<Scalar>::iterator it2;
                                it2 = listColorInRoi.begin();

                                bool LE = false;
                                bool colorFound = false;

                                while (it != listLE.end()){

                                    // Check if the current LE have a color.
                                    while (it2 != listColorInRoi.end()){

                                        if((*it).getColor() == (*it2)){

                                           colorFound = true;
                                           it2 = listColorInRoi.erase(it2);
                                           break;

                                        }

                                        ++it2;
                                    }

                                    if(colorFound){

                                        if(!LE){

                                            itLEbase = it;
                                            LE = true;

                                            (*it).LE_Roi.push_back(Point(areaPosition.x + j, areaPosition.y + i));

                                            Mat tempMat = (*it).getMap();
                                            Mat roiTemp(roiSize[1],roiSize[0],CV_8UC1,Scalar(255));
                                            roiTemp.copyTo(tempMat(Rect(areaPosition.x + j - roiSize[0]/2, areaPosition.y + i - roiSize[1]/2, roiSize[0], roiSize[1])));
                                            (*it).setMap(tempMat);

                                            // Update center of mass
                                            (*it).computeMassCenterWithRoi();

                                            // Update eventMap with the color of the new localEvent group
                                            Mat roi(roiSize[1], roiSize[0], CV_8UC3, listColorInRoi.at(0));
                                            roi.copyTo(eventMap(Rect(areaPosition.x + j-roiSize[0]/2, areaPosition.y + i-roiSize[1]/2,roiSize[0],roiSize[1])));

                                            colorInBlack(j, i, areaPosX, areaPosY, areaPosition, area, frame);

                                        }else{

                                            // Merge LE data

                                            Mat temp = (*it).getMap();
                                            Mat temp2 = (*itLEbase).getMap();
                                            Mat temp3 = temp + temp2;
                                            (*itLEbase).setMap(temp3);

                                            (*itLEbase).LE_Roi.insert((*itLEbase).LE_Roi.end(), (*it).LE_Roi.begin(), (*it).LE_Roi.end());

                                            it = listLE.erase(it);

                                        }

                                        colorFound = false;

                                    }else{

                                        ++it;

                                    }
                                }

                            }

                            break;
                    }
                }
            }
        }
    }
}
