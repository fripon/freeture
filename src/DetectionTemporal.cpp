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

boost::log::sources::severity_logger< LogSeverityLevel > DetectionTemporal::logger;

DetectionTemporal::Init DetectionTemporal::initializer;

DetectionTemporal::DetectionTemporal() {

    mSubdivisionStatus = false;

    mDebugEnabled = false;
    mDebugVideo = false;
    mStaticMaskEnabled = false;

    mMaskToCreate = false;

    mListColors.push_back(Scalar(0,0,139));      // DarkRed
    mListColors.push_back(Scalar(0,0,255));      // Red
    mListColors.push_back(Scalar(60,20,220));    // Crimson
    mListColors.push_back(Scalar(0,100,100));    // IndianRed
    mListColors.push_back(Scalar(92,92,205));    // Salmon
    mListColors.push_back(Scalar(0,140,255));    // DarkOrange
    mListColors.push_back(Scalar(30,105,210));   // Chocolate
    mListColors.push_back(Scalar(0,255,255));    // Yellow
    mListColors.push_back(Scalar(140,230,240));  // Khaki
    mListColors.push_back(Scalar(224,255,255));  // LightYellow
    mListColors.push_back(Scalar(211,0,148));    // DarkViolet
    mListColors.push_back(Scalar(147,20,255));   // DeepPink
    mListColors.push_back(Scalar(255,0,255));    // Magenta
    mListColors.push_back(Scalar(0,100,0));      // DarkGreen
    mListColors.push_back(Scalar(0,128,128));    // Olive
    mListColors.push_back(Scalar(0,255,0));      // Lime
    mListColors.push_back(Scalar(212,255,127));  // Aquamarine
    mListColors.push_back(Scalar(208,224,64));   // Turquoise
    mListColors.push_back(Scalar(205,0,0));      // Blue
    mListColors.push_back(Scalar(255,191,0));    // DeepSkyBlue
    mListColors.push_back(Scalar(255,255,0));    // Cyan

    mImgNum = 0;
    mRoiSize[0] = 10;
    mRoiSize[1] = 10;

}

bool DetectionTemporal::initMethod(string cfgPath) {

    try {

        Configuration cfg;
        cfg.Load(cfgPath);

        // Get acquisition frequency.
        int ACQ_FPS = 1;
        cfg.Get("ACQ_FPS", ACQ_FPS);
        BOOST_LOG_SEV(logger, notification) << "ACQ_FPS : " << ACQ_FPS;

        // Get downsample option.
        cfg.Get("DET_DOWNSAMPLE_ENABLED", mDownsampleEnabled);
        BOOST_LOG_SEV(logger, notification) << "DET_DOWNSAMPLE_ENABLED : " << mDownsampleEnabled;

        // Get gemap option.
        cfg.Get("DET_SAVE_GEMAP", mSaveGeMap);
        BOOST_LOG_SEV(logger, notification) << "DET_SAVE_GEMAP : " << mSaveGeMap;

        // Get dirmap option.
        cfg.Get("DET_SAVE_DIRMAP", mSaveDirMap);
        BOOST_LOG_SEV(logger, notification) << "DET_SAVE_DIRMAP : " << mSaveDirMap;

        // Get save position option.
        cfg.Get("DET_SAVE_POS", mSavePos);
        BOOST_LOG_SEV(logger, notification) << "DET_SAVE_POS : " << mSavePos;

        // Get save ge infos option.
        cfg.Get("DET_SAVE_GE_INFOS", mSaveGeInfos);
        BOOST_LOG_SEV(logger, notification) << "DET_SAVE_GE_INFOS : " << mSaveGeInfos;

        // Get use mask option.
        cfg.Get("ACQ_MASK_ENABLED", mMaskEnabled);
        BOOST_LOG_SEV(logger, notification) << "ACQ_MASK_ENABLED : " << mMaskEnabled;

        if(mMaskEnabled){

            cfg.Get("ACQ_MASK_PATH", mMaskPath);
            BOOST_LOG_SEV(logger, notification) << "ACQ_MASK_PATH : " << mMaskPath;

            mOriginalMask = imread(mMaskPath, CV_LOAD_IMAGE_GRAYSCALE);

            if(!mOriginalMask.data){
                cout << " Can't load the mask from this location : " << mMaskPath;
                BOOST_LOG_SEV(logger, notification) << " Can't load the mask from this location : " << mMaskPath;
                throw "Can't load the mask. Wrong location.";
            }

            if(mDownsampleEnabled){

                int imgH = mOriginalMask.rows; imgH /= 2;
                int imgW = mOriginalMask.cols; imgW /= 2;

                pyrDown(mOriginalMask, mOriginalMask, Size(imgW, imgH));

            }


            mOriginalMask.copyTo(mMask);

        }else{

            mMaskToCreate = true;

        }

        // Create local mask to eliminate single white pixels.
        Mat maskTemp(3,3,CV_8UC1,Scalar(255));
        maskTemp.at<uchar>(1, 1) = 0;
        maskTemp.copyTo(mLocalMask);

        // Get debug option.
        cfg.Get("DET_DEBUG", mDebugEnabled);
        BOOST_LOG_SEV(logger, notification) << "DET_DEBUG : " << mDebugEnabled;

        // Get debug path.
        cfg.Get("DET_DEBUG_PATH", mDebugPath);
        BOOST_LOG_SEV(logger, notification) << "DET_DEBUG_PATH : " << mDebugPath;

        // Get debug video option.
        cfg.Get("DET_DEBUG_VIDEO", mDebugVideo);
        BOOST_LOG_SEV(logger, notification) << "DET_DEBUG_VIDEO : " << mDebugVideo;

        cfg.Get("STATIC_MASK_OPTION", mStaticMaskEnabled);
        BOOST_LOG_SEV(logger, notification) << "STATIC_MASK_OPTION : " << mStaticMaskEnabled;


        cfg.Get("STATIC_MASK_INTERVAL", mStaticMaskInterval);
        BOOST_LOG_SEV(logger, notification) << "STATIC_MASK_INTERVAL : " << mStaticMaskInterval;
        if(mStaticMaskInterval<= 0){
            mStaticMaskInterval = ACQ_FPS * 10;
            BOOST_LOG_SEV(logger, fail) << "Error about STATIC_MASK_INTERVAL's value. Value changed to : " << mStaticMaskInterval;
        }

        // Create directories for debugging method.
        if(mDebugEnabled)
            initDebug();

        // Create debug video.
        if(mDebugVideo)
            mVideoDebug = VideoWriter(mDebugPath + "debug-video.avi", CV_FOURCC('M', 'J', 'P', 'G'), 5, Size(static_cast<int>(1280), static_cast<int>(960)), true);

	}catch(exception& e){

		cout << e.what() << endl;
		return false;

	}catch(const char * msg){

		cout << msg << endl;
		return false;

	}

	return true;

}

DetectionTemporal::~DetectionTemporal() {

}

void DetectionTemporal::resetDetection(){

    BOOST_LOG_SEV(logger, notification) << "Start clear listGlobalEvents";
    //cout << "Start clear listGlobalEvents" << endl;
    mListGlobalEvents.clear();
    BOOST_LOG_SEV(logger, notification) << "Clear finished";
    //cout << "Clear finished" << endl;

    mSubdivisionStatus = false;
    mPrevThresholdedMap.release();
    cout << "release mPrevFrame" << endl;

    mPrevFrame.release();

    if(mPrevFrame.data)
        cout << "mPrevFrame.data" << endl;
    else
        cout << "!mPrevFrame.data" << endl;


}

void DetectionTemporal::resetMask(){

    mMaskToCreate = true;


}

void DetectionTemporal::initDebug(){

    vector<string> debugSubDir;
    debugSubDir.push_back("curr");
    debugSubDir.push_back("prev");
    debugSubDir.push_back("diff");
    debugSubDir.push_back("thsh");
    debugSubDir.push_back("evMp");
    debugSubDir.push_back("motionMap");
    debugSubDir.push_back("motionMap2");
    debugSubDir.push_back("GEMAP");
    debugSubDir.push_back("mHighIntensityMap");
    debugSubDir.push_back("static");
    debugSubDir.push_back("mask");
    debugSubDir.push_back("dilateMotionMap");
    debugSubDir.push_back("currwithmask");

    for(int i = 0; i< debugSubDir.size(); i++){

        const boost::filesystem::path path(mDebugPath + debugSubDir.at(i));

        if(boost::filesystem::exists(path))
            boost::filesystem::remove_all(path);

        if(!boost::filesystem::exists(path))
            boost::filesystem::create_directories(path);

    }

}

void DetectionTemporal::saveDetectionInfos(string p){

    // Save ge map.
    if(mSaveGeMap)
        SaveImg::saveBMP((*mGeToSave).getMapEvent(), p + "GeMap");

    // Save dir map.
    if(mSaveDirMap)
        SaveImg::saveBMP((*mGeToSave).getDirMap(), p + "DirMap");

	// Save infos.
    if(mSaveGeInfos){

		ofstream infFile;
		string infFilePath = p + "GeInfos.txt";
		infFile.open(infFilePath.c_str());

		infFile << " * AGE              : " << (*mGeToSave).getAge()             << "\n";
		infFile << " * AGE LAST ELEM    : " << (*mGeToSave).getAgeLastElem()     << "\n";
		infFile << " * LINEAR STATE     : " << (*mGeToSave).getLinearStatus()    << "\n";
		infFile << " * BAD POS          : " << (*mGeToSave).getBadPos()          << "\n";
		infFile << " * GOOD POS         : " << (*mGeToSave).getGoodPos()         << "\n";
		infFile << " * NUM FIRST FRAME  : " << (*mGeToSave).getNumFirstFrame()   << "\n";
		infFile << " * NUM LAST FRAME   : " << (*mGeToSave).getNumLastFrame()    << "\n";

		float d = sqrt(pow((*mGeToSave).mainPts.back().x - (*mGeToSave).mainPts.front().x,2.0) + pow((*mGeToSave).mainPts.back().y - (*mGeToSave).mainPts.front().y,2.0));
		infFile << "\n * Distance between first and last  : " << d << "\n";

		infFile << "\n * MainPoints position : \n";

		for(int i = 0; i < (*mGeToSave).mainPts.size(); i++){

			infFile << "    (" << (*mGeToSave).mainPts.at(i).x << ";"<<  (*mGeToSave).mainPts.at(i).y << ")\n";

		}


		infFile << "\n * MainPoints details : \n";

		for(int i = 0; i < (*mGeToSave).listA.size(); i++){

			infFile << "    A(" << (*mGeToSave).listA.at(i).x << ";" << (*mGeToSave).listA.at(i).y << ") ----> ";
			infFile << "    B(" << (*mGeToSave).listB.at(i).x << ";" << (*mGeToSave).listB.at(i).y << ") ----> ";
			infFile << "    C(" << (*mGeToSave).listC.at(i).x << ";" << (*mGeToSave).listC.at(i).y << ")\n";
			infFile << "    u(" << (*mGeToSave).listu.at(i).x << ";" << (*mGeToSave).listu.at(i).y << ")       ";
			infFile << "    v(" << (*mGeToSave).listv.at(i).x << ";" << (*mGeToSave).listv.at(i).y << ")\n";
			infFile << "    Angle rad between BA' / BC = " << (*mGeToSave).listRad.at(i) << "\n";
			infFile << "    Angle between BA' / BC = " << (*mGeToSave).listAngle.at(i) << "\n";

			if((*mGeToSave).mainPtsValidity.at(i))
                infFile << "    NEW POSITION ACCEPTED\n\n";
            else
                infFile << "    NEW POSITION REFUSED\n\n";

		}

		infFile.close();
	}

    // Save approximate positions.
    if(mSavePos){

        ofstream posFile;
        string posFilePath = p + "positions.txt";
        posFile.open(posFilePath.c_str());

        vector<LocalEvent>::iterator itLe;
        for(itLe = (*mGeToSave).LEList.begin(); itLe!=(*mGeToSave).LEList.end(); ++itLe){

            Point pos = (*itLe).getMassCenter();

			if(mDownsampleEnabled) pos*=2;

            string line = Conversion::intToString((*itLe).getNumFrame()) + "               (" + Conversion::intToString(pos.x)  + ";" + Conversion::intToString(pos.y) + ")\n";
            posFile << line;

        }

        posFile.close();

    }

/*
    if(DET_SAVE_STDEV){

        Point graphSize = Point(1280, 960);

        const string labelAxisX = "frame";
        const string labelAxisY = "stdev";

        Mat graph_stdev = Mat(graphSize.y, graphSize.x, CV_8UC3, Scalar(0,0,0));

        // axis x
        line(graph_stdev, Point(40, graphSize.y - 40), Point(graphSize.x - 40, graphSize.y - 40), Scalar(255,255,255), 2);
        putText(graph_stdev, labelAxisX, cvPoint(graphSize.x - 70,graphSize.y - 20),FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(255,255,255), 1, CV_AA);

        // axis y
        line(graph_stdev, Point(40, graphSize.y - 40), Point( 40, 40), Scalar(255,255,255), 2);
        putText(graph_stdev, labelAxisY, cvPoint(10,20),FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(255,255,255), 1, CV_AA);

        // origin
        putText(graph_stdev, "0", cvPoint(30, graphSize.y - 20),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(255,255,255), 1, CV_AA);

        float maxH = maxstdev;

        int mawW = maxNumFrame;

        int nbDigitMaxH = Conversion::roundToUpperRange((int)maxH);
        int nbDigitMaxW = Conversion::roundToUpperRange((int)mawW);

        cout << "nbDigitMaxH : " << nbDigitMaxH<< endl;
        cout << "nbDigitMaxW : " << nbDigitMaxW<< endl;

        int sizeH = nbDigitMaxH / 5; //-> 20
        int sizeW = nbDigitMaxW / 5; // -> 200

        for(int i =1; i< 5; i++){

            // axe x
            line(graph_stdev, Point(((i * sizeW * 1200)/nbDigitMaxW) + 40, graphSize.y - 40 - 10), Point(((i * sizeW * 1200)/nbDigitMaxW)  + 40, graphSize.y - 40 + 10), Scalar(255,255,255), 1);
            putText(graph_stdev, Conversion::intToString(i * sizeW), Point(((i * sizeW * 1200)/nbDigitMaxW) + 40, graphSize.y - 40 +30),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(255,255,255), 1, CV_AA);

            // axe y
            line(graph_stdev, Point(40-10, graphSize.y - 40 - ((i * sizeH * 880)/nbDigitMaxH) ), Point(40+10, graphSize.y - 40 - ((i * sizeH * 880)/nbDigitMaxH)), Scalar(255,255,255), 1);
            putText(graph_stdev, Conversion::intToString(i * sizeH), Point(1, graphSize.y - 40 - ((i * sizeH * 880)/nbDigitMaxH) ),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(255,255,255), 1, CV_AA);

        }

        int s = numFrame.size();

        if(numFrame.size() == stdev.size()){

            Point p0 = Point((( numFrame.at(0) * 1200)/nbDigitMaxW) + 40, graphSize.y - 40 - ((stdev.at(0) * 880)/nbDigitMaxH));

            for(int i = 1; i< s; i++){

                Point p1 = Point((( numFrame.at(i) * 1200)/nbDigitMaxW) + 40, graphSize.y - 40 - ((stdev.at(i) * 880)/nbDigitMaxH));
                line(graph_stdev, p0, p1, Scalar(0,0,255), 1);

                p0 = p1;

            }

        }

        if(mDebugEnabled) SaveImg::saveBMP(graph_stdev, mDebugPath + "graph_stdev");

    }
*/
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

bool DetectionTemporal::run(Frame &c){

	if(!mSubdivisionStatus){

        mSubdivisionPos.clear();

		if(mDownsampleEnabled)
            subdivideFrame(mSubdivisionPos, 8, c.getImg().rows/2, c.getImg().cols/2);
		else
            subdivideFrame(mSubdivisionPos, 8, c.getImg().rows, c.getImg().cols);

		mSubdivisionStatus = true;

	}else{

		// Frame's size.
		int imgH = c.getImg().rows, imgW = c.getImg().cols;

		// Num of the current frame.
		mImgNum = c.getNumFrame();

        // Get frame.
		Mat cc, currImg;

		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%% STEP 1 : FILETRING / THRESHOLDING %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

		double t1 = (double)getTickCount();

        if(mDebugEnabled) SaveImg::saveBMP(c.getImg(), mDebugPath + "/curr/curr_"+Conversion::intToString(mImgNum));
        if(mDebugEnabled && mPrevFrame.data) SaveImg::saveBMP(mPrevFrame, mDebugPath + "/prev/prev"+Conversion::intToString(mImgNum));

        // Not use mask.
        if(mMaskToCreate && !mMaskEnabled){

            if(mDownsampleEnabled){

                mOriginalMask = Mat(imgH/2,imgW/2, CV_8UC1,Scalar(255));
                mMask = Mat(imgH/2,imgW/2, CV_8UC1,Scalar(255));

            }else{

                mOriginalMask = Mat(imgH,imgW, CV_8UC1,Scalar(255));
                mMask = Mat(imgH,imgW, CV_8UC1,Scalar(255));

            }

            mMaskToCreate = false;

        }

        c.getImg().copyTo(cc);

        // Downsample current image.
		if(mDownsampleEnabled){

            BOOST_LOG_SEV(logger, normal) << "Downsampling current frame ... ";

            double t1_downsample = (double)getTickCount();

			imgH /= 2;
			imgW /= 2;

			pyrDown(cc, cc, Size(imgW, imgH));

			t1_downsample = (((double)getTickCount() - t1_downsample)/getTickFrequency())*1000;
            cout << "> Downsample Time : " << t1_downsample << endl;

		}

        // Convert to 8 bits.
        Mat ccc;
		Conversion::convertTo8UC1(cc).copyTo(ccc);

        if(mStaticMaskEnabled){

            int dilation_type = MORPH_RECT;
            int dilation_size = 5;

            double t_static = (double)getTickCount();

            if(!mStaticMask.data){

                // Threshold current image to find pixels with high intensity value.
                Mat map1 = Mat(imgH,imgW, CV_8UC1,Scalar(0));
                threshold(ccc, map1, selectThreshold(ccc), 255, THRESH_BINARY);

                Mat element = getStructuringElement(dilation_type, Size(2*dilation_size + 1, 2*dilation_size+1), Point(dilation_size, dilation_size));

                dilate(map1, mStaticMask, element);

            }else{

                // Update mStaticMask
                if(mImgNum % mStaticMaskInterval == 0){

                    BOOST_LOG_SEV(logger, normal) << "Updating mStaticMask ... ";

                    // Threshold current image to find pixels with high intensity value.
                    Mat map1 = Mat(imgH,imgW, CV_8UC1,Scalar(0));
                    threshold(ccc, map1, selectThreshold(ccc), 255, THRESH_BINARY);

                    Mat tempHighIntensityMap;
                    Mat element = getStructuringElement(dilation_type, Size(2*dilation_size + 1, 2*dilation_size+1), Point(dilation_size, dilation_size));

                    dilate( map1, tempHighIntensityMap, element );

                    mStaticMask = mStaticMask & tempHighIntensityMap;

                    Mat invertStaticMask = cv::Mat::ones(mStaticMask.size(), mStaticMask.type()) * 255 - mStaticMask;

                    // Update mask.
                    invertStaticMask.copyTo(mMask,mOriginalMask);

                    if(mDebugEnabled) SaveImg::saveBMP(mStaticMask, mDebugPath + "/static/static_"+Conversion::intToString(mImgNum));
                    if(mDebugEnabled) SaveImg::saveBMP(mMask, mDebugPath + "/mMask/mask_"+Conversion::intToString(mImgNum));

                }
            }

            t_static = (((double)getTickCount() - t_static)/getTickFrequency())*1000;
            cout << " [- STATIC TIME-] : " << std::setprecision(3) << std::fixed << t_static << " ms " << endl;

        }


        if(cc.rows == mMask.rows && cc.cols == mMask.cols)
            cc.copyTo(currImg, mMask);
        else
            throw "ERROR : Mask size is not correct according to the frame size.";

        /// If previous image has no data.
        /// The current image become the previous and we do not continue the detection process for the current image.
		if(!mPrevFrame.data){

            currImg.copyTo(mPrevFrame);
            return false;

        }

		/// Search pixels which have changed.
        double t1_difference = (double)getTickCount();

        BOOST_LOG_SEV(logger, normal) << "Computing absolute difference ... ";

		Mat diffImg;

		absdiff(currImg, mPrevFrame, diffImg);

		Conversion::convertTo8UC1(diffImg).copyTo(diffImg);

		t1_difference = (((double)getTickCount() - t1_difference)/getTickFrequency())*1000;
		cout << "> Difference Time : " << t1_difference << endl;

		if(mDebugEnabled) SaveImg::saveBMP(diffImg, mDebugPath + "/diff/diff_"+Conversion::intToString(mImgNum));

        /// The current image become the previous image.
		currImg.copyTo(mPrevFrame);

        /// Apply threshold to create a binary map of changed pixels.

		double t1_threshold = (double)getTickCount();

		// Threshold map.
		Mat mapThreshold = Mat(imgH,imgW, CV_8UC1,Scalar(0));

		// Thresholding diff.
		BOOST_LOG_SEV(logger, normal) << "Thresholding absolute difference ... ";
		threshold(diffImg, mapThreshold, selectThreshold(diffImg), 255, THRESH_BINARY);


		t1_threshold = (((double)getTickCount() - t1_threshold)/getTickFrequency())*1000;
		cout << "> Threshold Time : " << t1_threshold << endl;

		if(mDebugEnabled) SaveImg::saveBMP(mapThreshold, mDebugPath + "/thsh/thsh_"+Conversion::intToString(mImgNum));

		double t1_threshold2 = (double)getTickCount();

		/// Remove single white pixel.
		BOOST_LOG_SEV(logger, normal) << "Removing single white pixel ... ";
		unsigned char * ptrT;

		Mat black(3,3,CV_8UC1,Scalar(0));

		for(int i = 0; i < imgH; i++){

			ptrT = mapThreshold.ptr<unsigned char>(i);

			for(int j = 0; j < imgW; j++){

				if(ptrT[j] > 0){

					if(j-1 > 0 && j+1 < imgW && i-1 > 0 && i +1 < imgH){

						Mat t = mapThreshold(Rect(j-1, i-1, 3, 3));

						Mat res = t & mLocalMask;

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

		t1_threshold2 = (((double)getTickCount() - t1_threshold2)/getTickFrequency())*1000;
		cout << "> Eliminate single white pixels Time : " << t1_threshold2 << endl;

        /// Difference of 2 last threshold map.
        BOOST_LOG_SEV(logger, normal) << "Differencing of 2 last threshold map ... ";
        Mat motionMap;

		if(mPrevThresholdedMap.data){

			motionMap = mapThreshold & mPrevThresholdedMap;

			if(mDebugEnabled) SaveImg::saveBMP(motionMap, mDebugPath + "/motionMap/motionMap_"+Conversion::intToString(mImgNum));

            // Create the final map according to the map of pixels with high values and the map of changed pixels.
			//motionMap = motionMap & mapC;

			if(mDebugEnabled) SaveImg::saveBMP(motionMap, mDebugPath + "/motionMap2/motionMap2_"+Conversion::intToString(mImgNum));

			mapThreshold.copyTo(mPrevThresholdedMap);

		}else{

		    mapThreshold.copyTo(mPrevThresholdedMap);
		    return false;

        }

		t1 = (((double)getTickCount() - t1)/getTickFrequency())*1000;
		cout << "> Time 1) : " << t1 << endl << endl;

		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% STEP 2 : FIND LOCAL EVENT %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

        // List of localEvent objects.
		vector <LocalEvent> listLocalEvents;

		// Execution time.
		double t2 = (double)getTickCount();

		// Event map for the current frame.
		Mat eventMap = Mat(imgH,imgW, CV_8UC3,Scalar(0,0,0));

        // Iterator on list of sub-regions.
        vector<Point>::iterator itR;

        // Analyse regions.
        BOOST_LOG_SEV(logger, normal) << "Loop subdivisions on frame " << Conversion::intToString(mImgNum)<< " ... ";
        cout << "curr frame " << Conversion::intToString(c.getNumFrame())<< " ... " << endl;

        /*double td = (double)getTickCount();
        int dilation_size = 10;
        Mat element = getStructuringElement(MORPH_RECT, Size(2*dilation_size + 1, 2*dilation_size+1), Point(dilation_size, dilation_size));
        dilate(motionMap, motionMap, element);
        td = (((double)getTickCount() - td)/getTickFrequency())*1000;
		cout << "> Dilatation time : " << td << endl << endl;

        if(mDebugEnabled) SaveImg::saveBMP(motionMap, mDebugPath + "/dilateMotionMap/dilate_"+Conversion::intToString(mImgNum));*/

        for(itR = mSubdivisionPos.begin(); itR != mSubdivisionPos.end(); ++itR){

            // Extract regions from motionMap.
            Mat subdivision = motionMap(Rect((*itR).x, (*itR).y, imgW/8, imgH/8));

            // Check if there is white pixels.
            if(countNonZero(subdivision) > 0){

                cout << (*itR)<< endl;

                searchROI(subdivision, motionMap, eventMap, listLocalEvents, (*itR), 10);

            }

        }

		BOOST_LOG_SEV(logger, normal) << "LE number : " << listLocalEvents.size();

		if(mDebugEnabled) SaveImg::saveBMP(eventMap, mDebugPath + "/evMp/evMp_" + Conversion::intToString(mImgNum));

		t2 = (((double)getTickCount() - t2)/getTickFrequency())*1000;
		cout << "> Time 2) : " << t2 << endl << endl;

		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%% STEP 3 : ATTACH LE TO GE OR CREATE NEW ONE %%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

        BOOST_LOG_SEV(logger, normal) << "Running step 3 ... ";

		// Iterator on list of global event.
		vector<GlobalEvent>::iterator itGE;
		// Iterator on list of local event.
		vector<LocalEvent>::iterator itLE;

		double t3 = (double)getTickCount();

		itLE = listLocalEvents.begin();

		while(itLE != listLocalEvents.end()){

            bool LELinked = false;
            vector<GlobalEvent>::iterator itGESelected;
            bool GESelected = false;

			(*itLE).setNumFrame(mImgNum);

			for(itGE = mListGlobalEvents.begin(); itGE != mListGlobalEvents.end(); ++itGE){

				Mat res = (*itLE).getMap() & (*itGE).getMapEvent();

				if(countNonZero(res) > 0){

					LELinked = true;

					// The current LE has found a possible global event.
                    if(GESelected){

                        //cout << "The current LE has found a possible global event."<< endl;

                        // Choose the older global event.
                        if((*itGE).getAge() > (*itGESelected).getAge()){

                            //cout << "Choose the older global event."<< endl;
                            itGESelected = itGE;

                        }

                    }else{

                        //cout << "Keep same"<< endl;
                        itGESelected = itGE;
                        GESelected = true;

                    }

                    break;

				}
			}

			// Add current LE to an existing GE
            if(GESelected){

                //cout << "Add current LE to an existing GE ... "<< endl;
                // Add LE.
                (*itGESelected).addLE((*itLE));
                //cout << "Flag to indicate that a local event has been added ... "<< endl;
                // Flag to indicate that a local event has been added.
                (*itGESelected).setNewLEStatus(true);
                //cout << "reset age of the last local event received by the global event.... "<< endl;
                // reset age of the last local event received by the global event.
                (*itGESelected).setAgeLastElem(0);

            }else{

                // The current LE has not been linked. It became a new GE.
                if(mListGlobalEvents.size() < 10){

                    //cout << "Selecting last available color ... "<< endl;
                    Scalar geColor = Scalar(255,255,255);//availableGeColor.back();
                    //cout << "Deleting last available color ... "<< endl;
                    //availableGeColor.pop_back();
                    //cout << "Creating new GE ... "<< endl;
                    GlobalEvent newGE(c.getAcqDateMicro(), c.getNumFrame(), currImg.rows, currImg.cols, geColor);
                    //cout << "Adding current LE ... "<< endl;
                    newGE.addLE((*itLE));
                    //cout << "Pushing new LE to GE list  ... "<< endl;
                    //Add the new globalEvent to the globalEvent's list
                    mListGlobalEvents.push_back(newGE);

                }
            }
            //cout << "Deleting the current localEvent  ... "<< endl;
            // Delete the current localEvent.
			itLE = listLocalEvents.erase(itLE);

		}

		t3 = (((double)getTickCount() - t3)/getTickFrequency())*1000;
		cout << "> Time 3) : " << t3 << endl;

		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%% STEP 4 : MANAGE LIST GLOBAL EVENT %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

        BOOST_LOG_SEV(logger, normal) << "Running step 4 ... ";

		double  t4 = (double)getTickCount();

		itGE = mListGlobalEvents.begin();

		bool saveSignal = false;

		while(itGE != mListGlobalEvents.end()){

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
				if((*itGE).LEList.size() >= 8 && (*itGE).continuousGoodPos(4) && (*itGE).ratioFramesDist()){

					mGeToSave = itGE;

                    saveSignal = true;

                    break;


				}else{

					// Delete the event.
					itGE = mListGlobalEvents.erase(itGE);

				}

			//CASE 2 : Not finished event.
			}else{

                // Too long event ? or not linear ?
				if( (*itGE).getAge() > 500 ||
					(!(*itGE).getLinearStatus() && !(*itGE).continuousGoodPos(5)) ||
                    (!(*itGE).getLinearStatus() && (*itGE).continuousBadPos((int)(*itGE).getAge()/2))){

					// Delete the event.
					itGE = mListGlobalEvents.erase(itGE);

                // Let the GE alive.
				}else if(c.getFrameRemaining() < 10 && c.getFrameRemaining() != 0){

				    if((*itGE).LEList.size() >= 8 && (*itGE).continuousGoodPos(4) && (*itGE).ratioFramesDist()){

                        mGeToSave = itGE;

                        saveSignal = true;

                        break;


                    }else{

                        // Delete the event.
                        itGE = mListGlobalEvents.erase(itGE);

                    }

				}else{

					++itGE;

				}
			}
		}

		t4 = (((double)getTickCount() - t4)/getTickFrequency())*1000;
		cout << "Time 4 : " << t4 << endl;

		/// ----------------------------------- DEBUG VIDEO ----------------------------------------

		if(mDebugVideo){

            // Create GE memory image
            Mat GEMAP = Mat(imgH, imgW, CV_8UC3, Scalar(0,0,0));
            for(itGE = mListGlobalEvents.begin(); itGE!= mListGlobalEvents.end(); ++itGE){

                GEMAP = GEMAP + (*itGE).getGeMapColor();

            }

            if(mDebugEnabled) SaveImg::saveBMP(GEMAP, mDebugPath + "/GEMAP/GEMAP_"+Conversion::intToString(mImgNum));

            Mat VIDEO               = Mat(960,1280, CV_8UC3,Scalar(255,255,255));
            Mat VIDEO_frame         = Mat(470,630, CV_8UC3,Scalar(0,0,0));
            Mat VIDEO_diffFrame     = Mat(470,630, CV_8UC3,Scalar(0,0,0));
            Mat VIDEO_threshFrame   = Mat(470,630, CV_8UC3,Scalar(0,0,0));
            Mat VIDEO_eventFrame    = Mat(470,630, CV_8UC3,Scalar(0,0,0));
            Mat VIDEO_geFrame       = Mat(470,630, CV_8UC3,Scalar(0,0,0));

            cvtColor(currImg, currImg, CV_GRAY2BGR);
            resize(currImg, VIDEO_frame, Size(630,470), 0, 0, INTER_LINEAR );
            cvtColor(mapThreshold, mapThreshold, CV_GRAY2BGR);
            resize(mapThreshold, VIDEO_threshFrame, Size(630,470), 0, 0, INTER_LINEAR );
            resize(eventMap, VIDEO_eventFrame, Size(630,470), 0, 0, INTER_LINEAR );
            resize(GEMAP, VIDEO_geFrame, Size(630,470), 0, 0, INTER_LINEAR );

            copyMakeBorder(VIDEO_frame, VIDEO_frame, 5, 5, 5, 5, BORDER_CONSTANT, Scalar(255,255,255) );
            copyMakeBorder(VIDEO_threshFrame, VIDEO_threshFrame, 5, 5, 5, 5, BORDER_CONSTANT, Scalar(255,255,255) );
            copyMakeBorder(VIDEO_eventFrame, VIDEO_eventFrame, 5, 5, 5, 5, BORDER_CONSTANT, Scalar(255,255,255) );
            copyMakeBorder(VIDEO_geFrame, VIDEO_geFrame, 5, 5, 5, 5, BORDER_CONSTANT, Scalar(255,255,255) );

            putText(VIDEO_frame, "Original", cvPoint(300,450),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,0,255), 1, CV_AA);
            putText(VIDEO_threshFrame, "Filtering", cvPoint(300,450),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,0,255), 1, CV_AA);
            putText(VIDEO_eventFrame, "Local Event Map", cvPoint(300,450),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,0,255), 1, CV_AA);
            putText(VIDEO_geFrame, "Global Event Map", cvPoint(300,450),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,0,255), 1, CV_AA);

            VIDEO_frame.copyTo(VIDEO(Rect(0, 0, 640, 480)));
            VIDEO_threshFrame.copyTo(VIDEO(Rect(640, 0, 640, 480)));
            VIDEO_eventFrame.copyTo(VIDEO(Rect(0, 480, 640, 480)));
            VIDEO_geFrame.copyTo(VIDEO(Rect(640, 480, 640, 480)));

            string fn = Conversion::intToString(mImgNum);
            const char * fn_c;
            fn_c = fn.c_str();

            putText(VIDEO, fn_c, cvPoint(30,50),FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(0,255,0), 2, CV_AA);

            if(mVideoDebug.isOpened()){

                mVideoDebug << VIDEO;

            }

        }

		return saveSignal;

	}

	return false;

}

vector<Scalar> DetectionTemporal::getColorInEventMap(Mat &eventMap, Point roiCenter){

    // ROI in the eventMap.
    Mat roi;

    // ROI extraction from the eventmap.
    eventMap(Rect(roiCenter.x-mRoiSize[0]/2, roiCenter.y-mRoiSize[1]/2, mRoiSize[0], mRoiSize[1])).copyTo(roi);

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


void DetectionTemporal::colorRoiInBlack(Point p, int h, int w, Mat &region){

    int posX    = p.x - w;
    int posY    = p.y - h;

    if(p.x - w < 0){

        w = p.x + w/2;
        posX = 0;

    }else if(p.x + w/2 > region.cols){

        w = region.cols - p.x + w/2;

    }

    if(p.y - h < 0){

        h = p.y + h/2;
        posY = 0;


    }else if(p.y + h/2 > region.rows){

        h = region.rows - p.y + h/2;

    }

    // Color roi in black in the current area.
    Mat roiBlackRegion(h, w, CV_8UC1, Scalar(0));
    roiBlackRegion.copyTo(region(Rect(posX, posY, w, h)));

}

void DetectionTemporal::searchROI( Mat &region,
                                    Mat &frame,                 // Thresholded frame
                                    Mat &eventMap,
                                    vector<LocalEvent> &listLE,
                                    Point regionPosInFrame,     // Origin position of a region in frame (corner top left)
                                    int maxNbLE){

    int situation = 0;

    unsigned char * ptr;

    // Loop pixel's region.
    for(int i = 0; i < region.rows; i++){

        ptr = region.ptr<unsigned char>(i);

        for(int j = 0; j < region.cols; j++){

            if((int)ptr[j] > 0){

                // Check we are not out of range in the frame if a ROI is defined in the current pixel location.
                if((regionPosInFrame.y + i - mRoiSize[1]/2 > 0)          &&
                   (regionPosInFrame.y + i + mRoiSize[1]/2 < frame.rows) &&
                   (regionPosInFrame.x + j - mRoiSize[0]/2 > 0)          &&
                   (regionPosInFrame.x + j + mRoiSize[0]/2 < frame.cols)){

                    // Get colors in eventMap at the current ROI location.
                    vector<Scalar> listColorInRoi = getColorInEventMap(eventMap, Point(regionPosInFrame.x + j, regionPosInFrame.y + i));

                    if(listColorInRoi.size() == 0)  situation = 0;  // black color      = create a new local event
                    if(listColorInRoi.size() == 1)  situation = 1;  // one color        = add the current roi to an existing local event
                    if(listColorInRoi.size() >  1)  situation = 2;  // several colors   = make a decision

                    switch(situation){

                        case 0 :

                            {

                                if(listLE.size() < maxNbLE){

                                    // Create new localEvent object.
                                    LocalEvent newLocalEvent(mListColors.at(listLE.size()), Point(regionPosInFrame.x + j, regionPosInFrame.y + i), frame.rows, frame.cols, mRoiSize);
                                    // Update center of mass.
                                    newLocalEvent.computeMassCenter();
                                    // Save the frame number where the local event has been created.
                                    newLocalEvent.setNumFrame(mImgNum);
                                    // Add LE in the list of localEvent.
                                    listLE.push_back(newLocalEvent);
                                    // Update eventMap with the color of the new localEvent.
                                    Mat roi(mRoiSize[1], mRoiSize[0], CV_8UC3, mListColors.at(listLE.size()-1));
                                    roi.copyTo(eventMap(Rect(regionPosInFrame.x + j-mRoiSize[0]/2, regionPosInFrame.y + i-mRoiSize[1]/2, mRoiSize[0], mRoiSize[1])));
                                    // Color roi in black in the current region.
                                    colorRoiInBlack(Point(j,i), mRoiSize[1], mRoiSize[0], region);

                                }
                            }

                            break;

                        case 1 :

                            {

                                vector<LocalEvent>::iterator it;

                                for(it=listLE.begin(); it!=listLE.end(); ++it){

                                    // Try to find a local event which has the same color.
                                    if((*it).getColor() == listColorInRoi.at(0)){

                                        // Add the current roi.
                                        (*it).mLeRoiList.push_back(Point(regionPosInFrame.x + j, regionPosInFrame.y + i));
                                        // Set local event 's map
                                        (*it).setMap(Point(regionPosInFrame.x + j - mRoiSize[0]/2, regionPosInFrame.y + i - mRoiSize[1]/2), mRoiSize[1], mRoiSize[0]);
                                        // Update center of mass
                                        (*it).computeMassCenter();
                                        // Update eventMap with the color of the new localEvent
                                        Mat roi(mRoiSize[1], mRoiSize[0], CV_8UC3, listColorInRoi.at(0));
                                        roi.copyTo(eventMap(Rect(regionPosInFrame.x + j-mRoiSize[0]/2, regionPosInFrame.y + i-mRoiSize[1]/2,mRoiSize[0],mRoiSize[1])));
                                        // Color roi in black in thresholded frame.
                                        Mat roiBlack(mRoiSize[1],mRoiSize[0],CV_8UC1,Scalar(0));
                                        roiBlack.copyTo(frame(Rect(regionPosInFrame.x + j-mRoiSize[0]/2, regionPosInFrame.y + i-mRoiSize[1]/2,mRoiSize[0],mRoiSize[1])));
                                        // Color roi in black in the current region.
                                        colorRoiInBlack(Point(j,i), mRoiSize[1], mRoiSize[0], region);

                                        break;

                                    }
                                }
                            }

                            break;

                        case 2 :

                            {
                               /* vector<LocalEvent>::iterator it;
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
                                }*/

                            }

                            break;
                    }
                }
            }
        }
    }
}

int DetectionTemporal::selectThreshold(Mat i){

    Scalar imgMean;
    Scalar imgStddev;

    meanStdDev(i, imgMean, imgStddev);

    /*if(DET_SAVE_STDEV){

        stdev.push_back(imgStddev.val[0]);

        if(imgStddev[0] > maxstdev)
            maxstdev = imgStddev[0];

    }*/

    return 5*(int)(cvRound(imgStddev.val[0])+1);

}
