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

    mMaskToCreate = false;
    mDataSetCounter = 0;

    mCapBuffer = boost::circular_buffer<Mat>(5);

    mListColors.push_back(Scalar(0,0,139));      // DarkRed
    mListColors.push_back(Scalar(0,0,255));      // Red
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

    mRoiSize[0] = 10;
    mRoiSize[1] = 10;
    mCapCounter = 0;

}

bool DetectionTemporal::initMethod(string cfgPath) {

    try {

        Configuration cfg;
        cfg.Load(cfgPath);

        // Get debug option.
        cfg.Get("DET_DEBUG", mDebugEnabled);
        BOOST_LOG_SEV(logger, notification) << "DET_DEBUG : " << mDebugEnabled;

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

        if(mMaskEnabled) {

            cfg.Get("ACQ_MASK_PATH", mMaskPath);
            BOOST_LOG_SEV(logger, notification) << "ACQ_MASK_PATH : " << mMaskPath;

            mMask = imread(mMaskPath, CV_LOAD_IMAGE_GRAYSCALE);

            if(!mMask.data){
                cout << " Can't load the mask from this location : " << mMaskPath;
                BOOST_LOG_SEV(logger, notification) << " Can't load the mask from this location : " << mMaskPath;
                throw "Can't load the mask. Wrong location.";
            }

            if(mDownsampleEnabled){

                int imgH = mMask.rows/2;
                int imgW = mMask.cols/2;

                pyrDown(mMask, mMask, Size(imgW, imgH));

            }

            mMask.copyTo(mOriginalMask);

        }else{

            mMaskToCreate = true;

        }

        // Create local mask to eliminate single white pixels.
        Mat maskTemp(3,3,CV_8UC1,Scalar(255));
        maskTemp.at<uchar>(1, 1) = 0;
        maskTemp.copyTo(mLocalMask);

        // Get debug path.
        cfg.Get("DET_DEBUG_PATH", mDebugPath);
        BOOST_LOG_SEV(logger, notification) << "DET_DEBUG_PATH : " << mDebugPath;
        mDebugCurrentPath = mDebugPath;

        // Get debug video option.
        cfg.Get("DET_DEBUG_VIDEO", mDebugVideo);
        BOOST_LOG_SEV(logger, notification) << "DET_DEBUG_VIDEO : " << mDebugVideo;

        // Create directories for debugging method.
        if(mDebugEnabled)
            createDebugDirectories(true);

        // Create debug video.
        if(mDebugVideo)
            mVideoDebug = VideoWriter(mDebugCurrentPath + "debug-video.avi", CV_FOURCC('M', 'J', 'P', 'G'), 5, Size(static_cast<int>(1280), static_cast<int>(960)), true);

        
        cfg.Get("DET_UPDATE_MASK", mUpdateMask);
        BOOST_LOG_SEV(logger, notification) << "DET_UPDATE_MASK : " << mUpdateMask;

        cfg.Get("DET_DEBUG_UPDATE_MASK", mDebugUpdateMask);
        BOOST_LOG_SEV(logger, notification) << "DET_DEBUG_UPDATE_MASK : " << mDebugUpdateMask;
        if(mDebugUpdateMask)
            mVideoDebugAutoMask = VideoWriter(mDebugCurrentPath + "debug-mask.avi", CV_FOURCC('M', 'J', 'P', 'G'), 5, Size(static_cast<int>(1280), static_cast<int>(960)), true);


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

void DetectionTemporal::resetDetection(bool loadNewDataSet){

    cout << "Clean Global Events list." << endl;
    mListGlobalEvents.clear();

    mSubdivisionStatus = false;
    mPrevThresholdedMap.release();
    mPrevFrame.release();

    if(mDebugEnabled && loadNewDataSet) {
        mDataSetCounter++;
        createDebugDirectories(false);
    }
}

void DetectionTemporal::resetMask(){

    mMaskToCreate = true;

}

void DetectionTemporal::createDebugDirectories(bool cleanDebugDirectory){

    cout << "Debug mode : enabled." << endl;

    mDebugCurrentPath = mDebugPath + "debug_" + Conversion::intToString(mDataSetCounter) + "/" ;

    if(cleanDebugDirectory) {

        cout << "Clean and create debug directories..." << endl;

        const boost::filesystem::path p0 = path(mDebugPath); // .../debug/

        if(boost::filesystem::exists(p0))
            boost::filesystem::remove_all(p0);

        if(!boost::filesystem::exists(p0))
            boost::filesystem::create_directories(p0);

    }else {

        cout << "Create debug directories..." << endl;

    }

    const boost::filesystem::path p1 = path(mDebugCurrentPath);             // .../debug/debug_0/
    const boost::filesystem::path p2 = path(mDebugCurrentPath + "local");   // .../debug/debug_0/local/
    const boost::filesystem::path p3 = path(mDebugCurrentPath + "global");  // .../debug/debug_0/global/

    if(!boost::filesystem::exists(p1))
        boost::filesystem::create_directories(p1);

    if(!boost::filesystem::exists(p2))
        boost::filesystem::create_directories(p2);

    if(!boost::filesystem::exists(p3))
        boost::filesystem::create_directories(p3);

    vector<string> debugSubDir;
    debugSubDir.push_back("absolute_difference");
    debugSubDir.push_back("absolute_difference_thresholded");
    debugSubDir.push_back("event_map_initial");
    debugSubDir.push_back("event_map_filtered");
    debugSubDir.push_back("absolute_difference_dilated");
    debugSubDir.push_back("neg_difference_thresholded");
    debugSubDir.push_back("pos_difference_thresholded");
    debugSubDir.push_back("neg_difference");
    debugSubDir.push_back("pos_difference");

    for(int i = 0; i< debugSubDir.size(); i++){

        const boost::filesystem::path path(mDebugCurrentPath + "global/" + debugSubDir.at(i));

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

bool DetectionTemporal::run(Frame &c) {

    BOOST_LOG_SEV(logger, normal) << "************************ DETECTION FRAME " << c.mFrameNumber << "************************";

    double mDownsampleTime  = 0;
    double absdiffTime      = 0;
    double posdiffTime      = 0;
    double negdiffTime      = 0;
    double dilateTime       = 0;
    double thresholdTime    = 0;
    double timeStep1        = 0;
    double timeStep2        = 0;
    double timeStep3        = 0;
    double timeStep4        = 0;
    double timeTotal        = 0;

    string pFrame0 = mDebugCurrentPath + "local/frame_" + Conversion::intToString(c.mFrameNumber);
    ofstream fileFrame;

    if(mDebugEnabled) {

        const boost::filesystem::path pFrame1 = path(pFrame0);
        if(!boost::filesystem::exists(pFrame1))
            boost::filesystem::create_directories(pFrame1);


        string pathFileFrame = pFrame0 + "/locallog.txt";
        fileFrame.open(pathFileFrame.c_str());

    }

    int h = 1, w = 1;

    if(mDownsampleEnabled) {

        h = c.mImg.rows/2;
        w = c.mImg.cols/2;

    }else {

        h = c.mImg.rows;
        w = c.mImg.cols;

    }

	if(!mSubdivisionStatus) {

        // --------------------------------------------------------------------------------------------------
        // -------------------------------- ---- FRAME SUBDIVISION ------------------------------------------
        // --------------------------------------------------------------------------------------------------

        mSubdivisionPos.clear();

        subdivideFrame(mSubdivisionPos, 8, h, w);

        if(mDebugEnabled) {

            ofstream fileSub;
            string pathFileSub = mDebugCurrentPath + "subdivisions.txt";
            fileSub.open(pathFileSub.c_str());

            fileSub << "There are " << mSubdivisionPos.size() << " subdivisions.\n";

            for(int i = 0; i < mSubdivisionPos.size(); i++) {

                fileSub << i << " : " << mSubdivisionPos.at(i) << "\n";

            }

            fileSub.close();

            Mat s;

            s = Mat(h, w,CV_8UC1,Scalar(0));

            for(int i = 0; i < 8; i++) {

               line(s, Point(0, i * (h/8)), Point(w - 1, i * (h/8)), Scalar(255), 1);
               line(s, Point(i * (w/8), 0), Point(i * (w/8), h-1), Scalar(255), 1);

            }

            SaveImg::saveBMP(s, mDebugCurrentPath + "subdivisions_map");

        }

		mSubdivisionStatus = true;

	}else {

        /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%% STEP 1 : FILETRING / THRESHOLDING %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

        // SUMMARY :
        // - Downsample.
        // - Check mask.
        // - Check previous frame.
        // - Absolute difference (absdiff).
        // - Positive difference.
        // - Negative difference.
        // - Dilatate absdiff.
        // - Threshold absdiff (3 x mean(absdiff)).

        // Time counter.
        double tstep1 = (double)getTickCount();

        // Current frame.
		Mat currImg;

        // ------------------------------
        //    Downsample current frame.
        // -------------------------------

		if(mDownsampleEnabled) {

            mDownsampleTime = (double)getTickCount();
			pyrDown(c.mImg, currImg, Size(c.mImg.cols / 2, c.mImg.rows / 2));
			mDownsampleTime = ((double)getTickCount() - mDownsampleTime);

		}else {

            c.mImg.copyTo(currImg);

        }

        // -------------------------------
        //  Create default mask if needed.
        // -------------------------------

        if(mMaskToCreate && !mMaskEnabled) {

            mMask = Mat(h, w, CV_8UC1,Scalar(255));
            mMask.copyTo(mOriginalMask);
            mMaskToCreate = false;

        }

        // Update mask by masking static and saturated objects.
        if(mCapCounter >= 1800 && mUpdateMask) {

            if(mCapBuffer.size() == 5) {

                // Sum the buffer.
                Mat sum = Mat::zeros(currImg.rows, currImg.cols, CV_32FC1);

                for(int i = 0; i< mCapBuffer.size(); i++)
                    accumulate(mCapBuffer.at(i), sum);

                Mat finalSaturatedMap = Mat::zeros(currImg.rows, currImg.cols, CV_8UC1);

                int max = 250 * 2;

                float * ptr;
                unsigned char * ptr2;

		        for(int i = 0; i < sum.rows; i++) {

			        ptr = sum.ptr<float>(i);
                    ptr2 = finalSaturatedMap.ptr<unsigned char>(i);

			        for(int j = 0; j < sum.cols; j++){

                        if(ptr[j] >= max) {
				            ptr2[j] = 255;
                        }

			        }
		        }

                // Merge with original mask
                Mat temp, temp1;
                bitwise_not(finalSaturatedMap,temp);
                temp.copyTo(mMask, mOriginalMask);

            }

            Mat saturatedMap = Mat::zeros(currImg.rows, currImg.cols, CV_8UC1);

            //Compute next static mask.
            if(currImg.type() == CV_16UC1) {

                unsigned short * ptr;
                unsigned char * ptrSat;

                for(int i = 0; i < currImg.rows; i++) {

                    ptr = currImg.ptr<unsigned short>(i);
                    ptrSat = saturatedMap.ptr<unsigned char>(i);

                    for(int j = 0; j < currImg.cols; j++){

                        if(ptr[j] >= 4095) {
                            ptrSat[j] = 255;
                        }

                    }
                }

            }else {

                unsigned char * ptr;
                unsigned char * ptrSat;

                for(int i = 0; i < currImg.rows; i++) {

                    ptr = currImg.ptr<unsigned char>(i);
                    ptrSat = saturatedMap.ptr<unsigned char>(i);

                    for(int j = 0; j < currImg.cols; j++){

                        if(ptr[j] >= 255) {
                            ptrSat[j] = 255;
                        }

                    }
                }
            }

            // Dilatation of the saturated map.
            int dilation_size = 10;
            Mat element = getStructuringElement(MORPH_RECT, Size(2*dilation_size + 1, 2*dilation_size+1), Point(dilation_size, dilation_size));
            cv::dilate(saturatedMap, saturatedMap, element);

            mCapBuffer.push_back(saturatedMap);

            mCapCounter = 0;

            if(mDebugUpdateMask) {

                Mat VIDEO               = Mat(960,1280, CV_8UC3,Scalar(255,255,255));
                Mat VIDEO_current_frame = Mat(470,630, CV_8UC3,Scalar(0,0,0));
                Mat VIDEO_current_mask  = Mat(470,630, CV_8UC3,Scalar(0,0,0));
                Mat VIDEO_original_mask = Mat(470,630, CV_8UC3,Scalar(0,0,0));
                Mat VIDEO_saturate_map  = Mat(470,630, CV_8UC3,Scalar(0,0,0));

                Mat tempCurr;
                Conversion::convertTo8UC1(currImg).copyTo(tempCurr);
                cvtColor(tempCurr, tempCurr, CV_GRAY2BGR);
                resize(tempCurr, VIDEO_current_frame, Size(630,470), 0, 0, INTER_LINEAR );
                Mat tempCurrMask;
                cvtColor(mMask, tempCurrMask, CV_GRAY2BGR);
                resize(tempCurrMask, VIDEO_current_mask, Size(630,470), 0, 0, INTER_LINEAR );
                Mat tempOriginalMask;
                cvtColor(mOriginalMask, tempOriginalMask, CV_GRAY2BGR);
                resize(tempOriginalMask, VIDEO_original_mask, Size(630,470), 0, 0, INTER_LINEAR );
                Mat tempSaturateMap;
                cvtColor(saturatedMap, tempSaturateMap, CV_GRAY2BGR);
                resize(tempSaturateMap, VIDEO_saturate_map, Size(630,470), 0, 0, INTER_LINEAR );

                copyMakeBorder(VIDEO_current_frame, VIDEO_current_frame, 5, 5, 5, 5, BORDER_CONSTANT, Scalar(255,255,255) );
                copyMakeBorder(VIDEO_original_mask, VIDEO_original_mask, 5, 5, 5, 5, BORDER_CONSTANT, Scalar(255,255,255) );
                copyMakeBorder(VIDEO_saturate_map, VIDEO_saturate_map, 5, 5, 5, 5, BORDER_CONSTANT, Scalar(255,255,255) );
                copyMakeBorder(VIDEO_current_mask, VIDEO_current_mask, 5, 5, 5, 5, BORDER_CONSTANT, Scalar(255,255,255) );

                /*putText(VIDEO_frame, "Original", cvPoint(300,450),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,0,255), 1, CV_AA);
                putText(VIDEO_threshFrame, "Filtering", cvPoint(300,450),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,0,255), 1, CV_AA);
                putText(VIDEO_eventFrame, "Local Event Map", cvPoint(300,450),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,0,255), 1, CV_AA);
                putText(VIDEO_geFrame, "Global Event Map", cvPoint(300,450),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,0,255), 1, CV_AA);*/

                VIDEO_current_frame.copyTo(VIDEO(Rect(0, 0, 640, 480)));
                VIDEO_original_mask.copyTo(VIDEO(Rect(640, 0, 640, 480)));
                VIDEO_saturate_map.copyTo(VIDEO(Rect(0, 480, 640, 480)));
                VIDEO_current_mask.copyTo(VIDEO(Rect(640, 480, 640, 480)));

                if(mVideoDebugAutoMask.isOpened()){

                    mVideoDebugAutoMask << VIDEO;

                }

            }

        }else {

            if(mUpdateMask) mCapCounter++;

            // --------------------------------
            //           Apply mask.
            // --------------------------------

            if(currImg.rows == mMask.rows && currImg.cols == mMask.cols) {

                Mat temp;
                currImg.copyTo(temp, mMask);
                temp.copyTo(currImg);

            }else {

                throw "ERROR : Mask size is not correct according to the frame size.";

            }

            // --------------------------------
            //      Check previous frame.
            // --------------------------------

            if(!mPrevFrame.data) {

                currImg.copyTo(mPrevFrame);
                return false;

            }

            // --------------------------------
		    //          Differences.
            // --------------------------------

            Mat absdiffImg, posDiffImg, negDiffImg;

            // Absolute difference.
            absdiffTime = (double)getTickCount();
            cv::absdiff(currImg, mPrevFrame, absdiffImg);
            absdiffTime = (double)getTickCount() - absdiffTime;

            // Positive difference.
            posdiffTime = (double)getTickCount();
            cv::subtract(currImg,mPrevFrame,posDiffImg,mMask);
            posdiffTime = (double)getTickCount() - posdiffTime;

            // Negative difference.
            negdiffTime = (double)getTickCount();
            cv::subtract(mPrevFrame,currImg,negDiffImg,mMask);
            negdiffTime = (double)getTickCount() - negdiffTime;

            // ---------------------------------
            //  Dilatation absolute difference.
            // ---------------------------------

            dilateTime = (double)getTickCount();
            int dilation_size = 2;
            Mat element = getStructuringElement(MORPH_RECT, Size(2*dilation_size + 1, 2*dilation_size+1), Point(dilation_size, dilation_size));
            cv::dilate(absdiffImg, absdiffImg, element);
            dilateTime = (double)getTickCount() - dilateTime;

            if(mDebugEnabled) {

                SaveImg::saveBMP(absdiffImg, mDebugCurrentPath + "/global/absolute_difference_dilated/frame_" + Conversion::intToString(c.mFrameNumber));
                SaveImg::saveBMP(absdiffImg, pFrame0 + "/absolute_difference_dilated");
            }

            // ----------------------------------
            //   Threshold absolute difference.
            // ----------------------------------

	        thresholdTime = (double)getTickCount();
		    Mat absDiffBinaryMap = Mat(currImg.rows,currImg.cols, CV_8UC1,Scalar(0));
            Scalar meanAbsDiff, stddevAbsDiff;
            cv::meanStdDev(absdiffImg, meanAbsDiff, stddevAbsDiff, mMask);
            int absDiffThreshold = /*stddevAbsDiff[0] * 5 + 10;//*/meanAbsDiff[0] * 3;

            if(absdiffImg.type() == CV_16UC1) {

                unsigned short * ptrAbsDiff;
                unsigned char * ptrMap;

	            for(int i = 0; i < absdiffImg.rows; i++) {

		            ptrAbsDiff = absdiffImg.ptr<unsigned short>(i);
                    ptrMap = absDiffBinaryMap.ptr<unsigned char>(i);

		            for(int j = 0; j < absdiffImg.cols; j++){

                        if(ptrAbsDiff[j] > absDiffThreshold) {
				            ptrMap[j] = 255;
                        }
		            }
	            }

            }else {

                unsigned char * ptrAbsDiff;
                unsigned char * ptrMap;

	            for(int i = 0; i < absdiffImg.rows; i++) {

		            ptrAbsDiff = absdiffImg.ptr<unsigned char>(i);
                    ptrMap = absDiffBinaryMap.ptr<unsigned char>(i);

		            for(int j = 0; j < absdiffImg.cols; j++){

                        if(ptrAbsDiff[j] > absDiffThreshold) {
				            ptrMap[j] = 255;
                        }
		            }
	            }
            }

            thresholdTime = (double)getTickCount() - thresholdTime;

            // ----------------------------------
            //   Threshold pos / neg difference.
            // ----------------------------------

            Scalar meanPosDiff, stddevPosDiff, meanNegDiff, stddevNegDiff;
            meanStdDev(posDiffImg, meanPosDiff, stddevPosDiff, mMask);
            meanStdDev(negDiffImg, meanNegDiff, stddevNegDiff, mMask);
            int posThreshold = stddevPosDiff[0] * 5 + 10;
            int negThreshold = stddevNegDiff[0] * 5 + 10;

            if(mDebugEnabled) {

                Mat posBinaryMap = Mat(currImg.rows,currImg.cols, CV_8UC1,Scalar(0));
                Mat negBinaryMap = Mat(currImg.rows,currImg.cols, CV_8UC1,Scalar(0));

                if(absdiffImg.type() == CV_16UC1) {

                    unsigned short * ptrInc;
                    unsigned short * ptrDec;
                    unsigned char * ptrT_inc;
                    unsigned char * ptrT_dec;

		            for(int i = 0; i < absdiffImg.rows; i++) {

			            ptrInc = posDiffImg.ptr<unsigned short>(i);
                        ptrDec = negDiffImg.ptr<unsigned short>(i);
                        ptrT_inc = posBinaryMap.ptr<unsigned char>(i);
                        ptrT_dec = negBinaryMap.ptr<unsigned char>(i);

			            for(int j = 0; j < absdiffImg.cols; j++){

                            if(ptrInc[j] > posThreshold) {
				                ptrT_inc[j] = 255;
                            }

                            if(ptrDec[j] > negThreshold) {
				                ptrT_dec[j] = 255;
                            }
			            }
		            }

                }else {

                    unsigned char * ptrInc;
                    unsigned char * ptrDec;
                    unsigned char * ptrT_inc;
                    unsigned char * ptrT_dec;

		            for(int i = 0; i < absdiffImg.rows; i++) {

			            ptrInc = posDiffImg.ptr<unsigned char>(i);
                        ptrDec = negDiffImg.ptr<unsigned char>(i);
                        ptrT_inc = posBinaryMap.ptr<unsigned char>(i);
                        ptrT_dec = negBinaryMap.ptr<unsigned char>(i);

			            for(int j = 0; j < absdiffImg.cols; j++){

                            if(ptrInc[j] > posThreshold) {
				                ptrT_inc[j] = 255;
                            }

                            if(ptrDec[j] > negThreshold) {
				                ptrT_dec[j] = 255;
                            }
			            }
		            }

                }

                SaveImg::saveBMP(posBinaryMap, mDebugCurrentPath + "/global/pos_difference_thresholded/frame_" + Conversion::intToString(c.mFrameNumber));
                SaveImg::saveBMP(posBinaryMap, pFrame0 + "/pos_difference_thresholded");
                SaveImg::saveBMP(negBinaryMap, mDebugCurrentPath + "/global/neg_difference_thresholded/frame_" + Conversion::intToString(c.mFrameNumber));
                SaveImg::saveBMP(negBinaryMap, pFrame0 + "/neg_difference_thresholded");

                SaveImg::saveBMP(absDiffBinaryMap, mDebugCurrentPath + "/global/absolute_difference_thresholded/frame_" + Conversion::intToString(c.mFrameNumber));
                SaveImg::saveBMP(absDiffBinaryMap, pFrame0 + "/absolute_difference_thresholded");

                SaveImg::saveBMP(absdiffImg, mDebugCurrentPath + "/global/absolute_difference/frame_"+Conversion::intToString(c.mFrameNumber));
                SaveImg::saveBMP(absdiffImg, pFrame0 + "/absolute_difference");
                SaveImg::saveBMP(Conversion::convertTo8UC1(posDiffImg), mDebugCurrentPath + "/global/pos_difference/frame_" + Conversion::intToString(c.mFrameNumber));
                SaveImg::saveBMP(Conversion::convertTo8UC1(posDiffImg), pFrame0 + "/pos_difference");
                SaveImg::saveBMP(Conversion::convertTo8UC1(negDiffImg), mDebugCurrentPath + "/global/neg_difference/frame_" + Conversion::intToString(c.mFrameNumber));
                SaveImg::saveBMP(Conversion::convertTo8UC1(negDiffImg), pFrame0 + "/neg_difference");

            }

            // Current frame is stored as the previous frame.
		    currImg.copyTo(mPrevFrame);

            // Time counter of the first step.
            timeStep1 = (double)getTickCount() - timeStep1;

		    /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		    /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% STEP 2 : FIND LOCAL EVENT %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		    /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

            // SUMMARY :
            // Loop binarized absolute difference image.
            // For each white pixel, define a Region of interest (ROI) of 10x10 centered in this pixel.
            // Create a new Local Event initialized with this first ROI or attach this ROI to an existing Local Event.
            // Loop the ROI in the binarized absolute difference image to store position of white pixels.
            // Loop the ROI in the positive difference image to store positions of white pixels.
            // Loop the ROI in the negative difference image to store positions of white pixels.
            // Once the list of Local Event has been completed :
            // Analyze each local event in order to check that pixels can be clearly split in two groups (negative, positive).

            // List of localEvent objects.
		    vector <LocalEvent> listLocalEvents;

            // Iterator on list of sub-regions.
            vector<LocalEvent>::iterator itLE;

            // Local Event list Historic informations.
            ofstream fileLe;
            if(mDebugEnabled) {

                string pathFileLe = pFrame0 + "/LocalEvent.txt";
                fileLe.open(pathFileLe.c_str());

            }

		    // Time counter.
		    timeStep2 = (double)getTickCount();

		    // Event map for the current frame.
		    Mat eventMap = Mat(currImg.rows,currImg.cols, CV_8UC3,Scalar(0,0,0));

            // ----------------------------------
            //        Search local events.
            // ----------------------------------

            // Iterator on list of sub-regions.
            vector<Point>::iterator itR;

            // Analyse subdivisions.
            if(mDebugEnabled) fileFrame << "**************** ANALYSE SUBDIVISIONS. ****************\n";
            for(itR = mSubdivisionPos.begin(); itR != mSubdivisionPos.end(); ++itR) {

                // Extract subdivision from binary map.
                Mat subdivision = absDiffBinaryMap(Rect((*itR).x, (*itR).y, absDiffBinaryMap.cols/8, absDiffBinaryMap.rows/8));

                if(mDebugEnabled) {

                    fileLe << "***************************************************\n";
                    fileLe << "Nb LE : " << listLocalEvents.size() << "\n";
                    fileLe << "---------------------------------------------------\n";
                    for(int i = 0; i < listLocalEvents.size(); i++)
                        fileLe << "LE " << i << " : " << listLocalEvents.at(i).getMassCenter() << "\n";
                    fileLe << "---------------------------------------------------\n";
                }

                // Check if there is white pixels.
                if(countNonZero(subdivision) > 0){

                    if(mDebugEnabled) {

                        fileFrame << "REGION : " << (*itR) << " ----> NOT EMPTY ! \n" ;
                        fileLe << "Region : " << (*itR) << " ----> NOT EMPTY ! \n" ;

                    }

                    string debugMsg = "";

                    analyseRegion(  subdivision,
                                    absDiffBinaryMap,
                                    eventMap,
                                    posDiffImg,
                                    posThreshold,
                                    negDiffImg,
                                    negThreshold,
                                    listLocalEvents,
                                    (*itR),
                                    10,
                                    c.mFrameNumber,
                                    debugMsg);

                    if(mDebugEnabled)fileLe << debugMsg;

                }else {

                    if(mDebugEnabled) {

                        fileFrame << "REGION : " << (*itR) << " ----> EMPTY \n" ;
                        fileLe << "Region : " << (*itR) << " ----> EMPTY ! \n" ;

                    }
                }

            }

            for(int i = 0; i < listLocalEvents.size(); i++) {
                listLocalEvents.at(i).setLeIndex(i);
            }

            if(mDebugEnabled) {

                fileLe << "***************************************************\n\n\n";
                fileLe << "Final Nb LE : " << listLocalEvents.size() << "\n";
                fileLe << "---------------------------------------------------\n";
                for(int i = 0; i < listLocalEvents.size(); i++) {

                    fileLe << "------ LE " << listLocalEvents.at(i).getLeIndex() << "------- \n";
                    fileLe << "- vecteur neg to pos : (" + Conversion::intToString(listLocalEvents.at(i).getLeDir().x) + ";" + Conversion::intToString(listLocalEvents.at(i).getLeDir().y) + ")\n" ;
                    fileLe << "- mass center abs : (" + Conversion::intToString(listLocalEvents.at(i).getMassCenter().x) + ";" + Conversion::intToString(listLocalEvents.at(i).getMassCenter().y) + ")\n";
                    fileLe << "- mass center pos : (" + Conversion::intToString(listLocalEvents.at(i).getPosMassCenter().x) + ";" + Conversion::intToString(listLocalEvents.at(i).getPosMassCenter().y) + ")\n" ;
                    fileLe << "- mass center neg : (" + Conversion::intToString(listLocalEvents.at(i).getNegMassCenter().x) + ";" + Conversion::intToString(listLocalEvents.at(i).getNegMassCenter().y) + ")\n" ;
                    fileLe << "- mPosRadius : " + Conversion::floatToString(listLocalEvents.at(i). getPosRadius()) + "\n";
                    fileLe << "- mNegRadius : " + Conversion::floatToString(listLocalEvents.at(i). getNegRadius()) + "\n";
                    if(listLocalEvents.at(i).getPosCluster())
                        fileLe << "- mPosCluster : true \n";
                    else
                        fileLe << "- mPosCluster : false \n";

                    if(listLocalEvents.at(i).getNegCluster())
                        fileLe << "- mNegCluster : true \n";
                    else
                        fileLe << "- mNegCluster : false \n";

                    if(listLocalEvents.at(i).getMergedFlag())
                        fileLe << "- mergedFlag : true \n";
                    else
                        fileLe << "- mergedFlag : false \n";

                    SaveImg::saveBMP(listLocalEvents.at(i).createPosNegAbsMap(), pFrame0 + "/LE_" + Conversion::intToString(listLocalEvents.at(i).getLeIndex()));

                }
                fileLe << "---------------------------------------------------\n\n\n";

                SaveImg::saveBMP(eventMap, mDebugCurrentPath + "/global/event_map_initial/frame_" + Conversion::intToString(c.mFrameNumber));
                SaveImg::saveBMP(eventMap, pFrame0 + "/event_map_initial");
            }

            // ----------------------------------
            //         Link between LE.
            // ----------------------------------

            int leNumber = listLocalEvents.size();

            // Liste d'iterators sur la liste des localEvent contenant soit un cluster positif ou négatif.
            vector<vector<LocalEvent>::iterator > itLePos, itLeNeg;
            // Association d'un local event à cluster positif avec un local event à cluster negatif.
            vector<pair<vector<LocalEvent>::iterator, vector<LocalEvent>::iterator> > itPair;

            itLE = listLocalEvents.begin();

            // Search pos and neg alone.
            while(itLE != listLocalEvents.end()) {

                // Le has pos cluster but no neg cluster.
                if((*itLE).getPosClusterStatus() && !(*itLE).getNegClusterStatus()) {

                    if(mDebugEnabled)fileLe << "LE " << (*itLE).getLeIndex() << " has no negative cluster\n";
                    itLePos.push_back(itLE);

                }else if(!(*itLE).getPosClusterStatus() && (*itLE).getNegClusterStatus()){

                    if(mDebugEnabled)fileLe << "LE " << (*itLE).getLeIndex() << " has no negative cluster\n";
                    itLeNeg.push_back(itLE);

                }

                ++itLE;

            }

            int maxRadius = 50;

            // Try to link a positive cluster to a negative one.
            for(int i = 0; i < itLePos.size(); i++) {

                int nbPotentialNeg = 0;

                vector<LocalEvent>::iterator itChoose;
                vector<vector<LocalEvent>::iterator >::iterator c;

                for(vector<vector<LocalEvent>::iterator >::iterator j = itLeNeg.begin(); j != itLeNeg.end();) {

                    Point A = (*itLePos.at(i)).getMassCenter();
                    Point B = (*(*j)).getMassCenter();
                    float dist = sqrt(pow((A.x - B.x),2) + pow((A.y - B.y),2));

                    if(dist < 50) {

                        nbPotentialNeg++;
                        itChoose = (*j);
                        c = j;

                    }

                    ++j;

                }

                if(nbPotentialNeg == 1) {

                    if(mDebugEnabled)fileLe << "MERGE LE " << (*itLePos.at(i)).getLeIndex() << " with LE " << (*itChoose).getLeIndex() << " \n";
                    (*itLePos.at(i)).mergeWithAnOtherLE((*itChoose));
                    (*itLePos.at(i)).setMergedStatus(true);
                    (*itChoose).setMergedStatus(true);
                    itLeNeg.erase(c);

                }

            }

            // Delete pos cluster not merged and negative cluster not merged.
             itLE = listLocalEvents.begin();

            // Search pos and neg alone.
            while(itLE != listLocalEvents.end()) {

                // Le has pos cluster but no neg cluster.
                if(// ((*itLE).getPosClusterStatus() && !(*itLE).getNegClusterStatus() && !(*itLE).getMergedStatus())||
                    (!(*itLE).getPosClusterStatus() && (*itLE).getNegClusterStatus()&& (*itLE).getMergedStatus())) {

                    if(mDebugEnabled)fileLe << "Remove LE "  << (*itLE).getLeIndex() << " \n";
                    itLE = listLocalEvents.erase(itLE);

                }else {

                    ++itLE;

                }

            }

            // -----------------------------------
            //            Circle TEST.
            // -----------------------------------

            leNumber = listLocalEvents.size();

            itLE = listLocalEvents.begin();

            while(itLE != listLocalEvents.end()) {

                if((*itLE).getPosClusterStatus() && (*itLE).getNegClusterStatus()) {

                    if((*itLE).localEventIsValid()) {

                        ++itLE;

                    }else {

                        fileLe << "Remove LE "  << (*itLE).getLeIndex() << " \n";
                        itLE = listLocalEvents.erase(itLE);

                    }

                }else {
                    ++itLE;
                }
            }

            if(mDebugEnabled) {

                fileLe << "\n---------------------------------------------------\n";
                fileLe << "Final Nb LE : " << listLocalEvents.size() << "\n";
                for(int i = 0; i < listLocalEvents.size(); i++)
                    fileLe << "LE " << listLocalEvents.at(i).getLeIndex() << " : " << listLocalEvents.at(i).getMassCenter() << "\n";
                fileLe << "---------------------------------------------------\n\n\n";

                vector<LocalEvent>::iterator itLE_;
                itLE_ = listLocalEvents.begin();

                int a = 0;

                ofstream f;

                string fpath = pFrame0 + "/LE.txt";
                f.open(fpath.c_str());

		        while(itLE_ != listLocalEvents.end()){

                    string line = " ******* LE " + Conversion::intToString((*itLE_).getLeIndex()) + " ******* \n";
                    f << line;
                    line = "- vecteur neg to pos : (" + Conversion::intToString((*itLE_).getLeDir().x) + ";" + Conversion::intToString((*itLE_).getLeDir().y) + ")\n" ;
                    f << line ;
                    line = "- mass center abs : (" + Conversion::intToString((*itLE_).getMassCenter().x) + ";" + Conversion::intToString((*itLE_).getMassCenter().y) + ")\n" ;
                    f << line ;
                    line = "- mass center pos : (" + Conversion::intToString((*itLE_).getPosMassCenter().x) + ";" + Conversion::intToString((*itLE_).getPosMassCenter().y) + ")\n" ;
                    f << line ;
                    line = "- mass center neg : (" + Conversion::intToString((*itLE_).getNegMassCenter().x) + ";" + Conversion::intToString((*itLE_).getNegMassCenter().y) + ")\n" ;
                    f << line ;
                    line = "- mPosRadius : " + Conversion::floatToString((*itLE_). getPosRadius()) + "\n";
                    f << line ;
                    line = "- mNegRadius : " + Conversion::floatToString((*itLE_). getNegRadius()) + "\n";
                    f << line ;
                    if((*itLE_).getPosCluster())
                        line = "- mPosCluster : true \n";
                    else
                        line = "- mPosCluster : false \n";
                    f << line ;
                    if((*itLE_).getNegCluster())
                        line = "- mNegCluster : true \n";
                    else
                        line = "- mNegCluster : false \n";
                    f << line ;
                    if((*itLE_).getMergedFlag())
                        line = "- mergedFlag : true \n";
                    else
                        line = "- mergedFlag : false \n";
                    f << line ;

                    SaveImg::saveBMP((*itLE_).createPosNegAbsMap(), pFrame0 + "/LE_Filtered_" + Conversion::intToString((*itLE_).getLeIndex()));

                    ++itLE_;

                    f << "\n\n";

                    a++;

                }

                f.close();

                Mat eventMapFiltered = Mat(currImg.rows,currImg.cols, CV_8UC3,Scalar(0,0,0));

                for(int i = 0; i < listLocalEvents.size(); i++) {

                    Mat roiF(10, 10, CV_8UC3, listLocalEvents.at(i).getColor());

                    for(int j = 0; j < listLocalEvents.at(i).mLeRoiList.size();j++) {

                        if( listLocalEvents.at(i).mLeRoiList.at(j).x-5 > 0 &&
                            listLocalEvents.at(i).mLeRoiList.at(j).x+5 < eventMapFiltered.cols &&
                            listLocalEvents.at(i).mLeRoiList.at(j).y-5 > 0 &&
                            listLocalEvents.at(i).mLeRoiList.at(j).y+5 < eventMapFiltered.rows)

                        roiF.copyTo(eventMapFiltered(Rect(listLocalEvents.at(i).mLeRoiList.at(j).x - 5, listLocalEvents.at(i).mLeRoiList.at(j).y + 5, 10, 10)));

                    }

                }

                SaveImg::saveBMP(eventMapFiltered, mDebugCurrentPath + "/global/event_map_filtered/frame_" + Conversion::intToString(c.mFrameNumber));
                SaveImg::saveBMP(eventMapFiltered, pFrame0 + "/event_map_filtered");

                fileLe.close();

            }

		    timeStep2 = (double)getTickCount() - timeStep2;

		    /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		    /// %%%%%%%%%%%%%%%%%%%%%%%%%% STEP 3 : ATTACH LE TO GE OR CREATE NEW ONE %%%%%%%%%%%%%%%%%%%%%%%%%
		    /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

            // SUMMARY :
            // Loop list of local events.
            // Create a new global event initialized with the current Local event or attach it to an existing global event.
            // If attached, check the positive-negative couple of the global event.

		    // Iterator on list of global event.
		    vector<GlobalEvent>::iterator itGE;

		    timeStep3 = (double)getTickCount();

		    itLE = listLocalEvents.begin();

		    while(itLE != listLocalEvents.end()) {

                bool LELinked = false;
                vector<GlobalEvent>::iterator itGESelected;
                bool GESelected = false;

			    (*itLE).setNumFrame(c.mFrameNumber);

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
                        GlobalEvent newGE(c.mDate, c.mFrameNumber, currImg.rows, currImg.cols, geColor);
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

		    timeStep3 = (double)getTickCount() - timeStep3;

		    /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		    /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%% STEP 4 : MANAGE LIST GLOBAL EVENT %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		    /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


            ofstream fileGe;
            if(mDebugEnabled) {

                string pathFileGe = pFrame0 + "/GlobalEvent.txt";
                fileGe.open(pathFileGe.c_str());
                fileGe << "Size list GE : " << mListGlobalEvents.size() << "\n\n";
            }


		    timeStep4 = (double)getTickCount();

		    itGE = mListGlobalEvents.begin();

		    bool saveSignal = false;

            int nGe = 0;

		    while(itGE != mListGlobalEvents.end()){

                if(mDebugEnabled) fileGe << "----------- GE " << nGe << " -----------\n";

                // Increment age.
			    (*itGE).setAge((*itGE).getAge() + 1);
                if(mDebugEnabled) fileGe << "-> AGE :  " << (*itGE).getAge() << "\n";

                // The current global event has not received a new local event.
			    if(!(*itGE).getNewLEStatus()){

                    // Increment age without any new local event.
				    (*itGE).setAgeLastElem((*itGE).getAgeLastElem()+1);

			    }else{

				    (*itGE).setNumLastFrame(c.mFrameNumber);
				    (*itGE).setNewLEStatus(false);

			    }

                if(mDebugEnabled) fileGe << "-> AGE LAST ELEM :  " << (*itGE).getAgeLastElem() << "\n";
                if(mDebugEnabled) fileGe << "-> NUM LAST IMG  :  " << (*itGE).getNumLastFrame() << "\n";

                string msgGe = "";
			    // CASE 1 : Finished event.
			    if((*itGE).getAgeLastElem() > 5){

                    //cout << "Finished event." << endl;
                    if(mDebugEnabled) fileGe << "-> Finished event.\n";
                    fileGe << "Vec LE : " << (*itGE).leDir << "\n";
                    fileGe << "Vec GE : " << (*itGE).geDir << "\n";
                    fileGe << "NB LE : " << (*itGE).LEList.size() << "\n";
                    // Linear profil ? Minimum duration respected ?
				    if((*itGE).LEList.size() >= 5 && (*itGE).continuousGoodPos(4, msgGe) && (*itGE).ratioFramesDist(msgGe) && (*itGE).negPosClusterFilter(msgGe)){

                        if(mDebugEnabled) fileGe <<msgGe;
					    mGeToSave = itGE;
                        if(mDebugEnabled) fileGe << "-> SAVE THIS GE  !  \n";
                        saveSignal = true;

                        break;


				    }else{

                        if(mDebugEnabled) fileGe <<msgGe;
                        if(mDebugEnabled) fileGe << "-> REMOVE THIS GE   \n";
					    // Delete the event.
					    itGE = mListGlobalEvents.erase(itGE);

				    }





			    //CASE 2 : Not finished event.
			    }else{
                    if(mDebugEnabled) fileGe << "-> Not Finished event.\n";
                    // Too long event ? or not linear ?
				    if( (*itGE).getAge() > 500 ||
					    (!(*itGE).getLinearStatus() && !(*itGE).continuousGoodPos(5,msgGe)) ||
                        (!(*itGE).getLinearStatus() && (*itGE).continuousBadPos((int)(*itGE).getAge()/2))){

					    // Delete the event.
					    itGE = mListGlobalEvents.erase(itGE);

                    // Let the GE alive.
                    }else if(c.mFrameRemaining < 10 && c.mFrameRemaining != 0){

				        if((*itGE).LEList.size() >= 5 && (*itGE).continuousGoodPos(4,msgGe) && (*itGE).ratioFramesDist(msgGe)&& (*itGE).negPosClusterFilter(msgGe)){

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

                /*if(mDebugEnabled) fileGe << "-> LINEAR        :  " << (*itGE).getLinearStatus() << "\n";
                if(mDebugEnabled) fileGe << "-> BAD POS       :  " << (*itGE).getBadPos() << "\n";
                if(mDebugEnabled) fileGe << "-> GOOD POS      :  " << (*itGE).getGoodPos() << "\n";
                if(mDebugEnabled) fileGe << "-> VELOCITY      :  " << (*itGE).getVelocity() << "\n";*/

            }

		    timeStep4 = (double)getTickCount() - timeStep4;

		    /// ----------------------------------- DEBUG VIDEO ----------------------------------------

		    if(mDebugVideo){

                // Create GE memory image
                Mat GEMAP = Mat(currImg.rows, currImg.cols, CV_8UC3, Scalar(0,0,0));
                for(itGE = mListGlobalEvents.begin(); itGE!= mListGlobalEvents.end(); ++itGE){

                    GEMAP = GEMAP + (*itGE).getGeMapColor();

                }

                if(mDebugEnabled) SaveImg::saveBMP(GEMAP, mDebugCurrentPath + "/GEMAP/GEMAP_"+Conversion::intToString(c.mFrameNumber));

                Mat VIDEO               = Mat(960,1280, CV_8UC3,Scalar(255,255,255));
                Mat VIDEO_frame         = Mat(470,630, CV_8UC3,Scalar(0,0,0));
                Mat VIDEO_diffFrame     = Mat(470,630, CV_8UC3,Scalar(0,0,0));
                Mat VIDEO_threshFrame   = Mat(470,630, CV_8UC3,Scalar(0,0,0));
                Mat VIDEO_eventFrame    = Mat(470,630, CV_8UC3,Scalar(0,0,0));
                Mat VIDEO_geFrame       = Mat(470,630, CV_8UC3,Scalar(0,0,0));

                cvtColor(currImg, currImg, CV_GRAY2BGR);
                resize(currImg, VIDEO_frame, Size(630,470), 0, 0, INTER_LINEAR );
                cvtColor(absDiffBinaryMap, absDiffBinaryMap, CV_GRAY2BGR);
                resize(absDiffBinaryMap, VIDEO_threshFrame, Size(630,470), 0, 0, INTER_LINEAR );
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

                string fn = Conversion::intToString(c.mFrameNumber);
                const char * fn_c;
                fn_c = fn.c_str();

                putText(VIDEO, fn_c, cvPoint(30,50),FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(0,255,0), 2, CV_AA);

                if(mVideoDebug.isOpened()){

                    mVideoDebug << VIDEO;

                }

            }

            if(mDebugEnabled) {
                fileFrame.close();
                fileGe.close();
            }

		    return saveSignal;

        }
	}

    if(mDebugEnabled) fileFrame.close();

    timeTotal = (double)getTickCount() - timeTotal;

    if(mDownsampleTime!=0)
        cout << "mDownsampleTime : " << (mDownsampleTime/getTickFrequency())*1000 << " ms" << endl;
    if(absdiffTime!=0)
        cout << "absdiffTime : " << (absdiffTime/getTickFrequency())*1000 << " ms" << endl;
    if(posdiffTime!=0)
        cout << "posdiffTime : " << (posdiffTime/getTickFrequency())*1000 << " ms" << endl;
    if(negdiffTime!=0)
        cout << "negdiffTime : " << (negdiffTime/getTickFrequency())*1000 << " ms" << endl;
    if(dilateTime!=0)
        cout << "dilateTime : " << (dilateTime/getTickFrequency())*1000 << " ms" << endl;
    if(thresholdTime!=0)
        cout << "thresholdTime : " << (thresholdTime/getTickFrequency())*1000 << " ms" << endl;
    if(timeStep1!=0)
        cout << "timeStep1 : " << (timeStep1/getTickFrequency())*1000 << " ms" << endl;
    if(timeStep2!=0)
        cout << "timeStep2 : " << (timeStep2/getTickFrequency())*1000 << " ms" << endl;
    if(timeStep3!=0)
        cout << "timeStep3 : " << (timeStep3/getTickFrequency())*1000 << " ms" << endl;
    if(timeStep4!=0)
        cout << "timeStep4 : " << (timeStep4/getTickFrequency())*1000 << " ms" << endl;
    if(timeTotal!=0)
        cout << "timeTotal : " << (timeTotal/getTickFrequency())*1000 << " ms" << endl;

	return false;

}

vector<Scalar> DetectionTemporal::getColorInEventMap(Mat &eventMap, Point roiCenter) {

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

void DetectionTemporal::analyseRegion(  Mat &subdivision,
                                        Mat &absDiffBinaryMap,
                                        Mat &eventMap,
                                        Mat &posDiff,
                                        int posDiffThreshold,
                                        Mat &negDiff,
                                        int negDiffThreshold,
                                        vector<LocalEvent> &listLE,
                                        Point subdivisionPos,     // Origin position of a region in frame (corner top left)
                                        int maxNbLE,
                                        int numFrame,
                                        string &msg){

    int situation = 0;
    int nbCreatedLE = 0;
    int nbRoiAttachedToLE = 0;
    int nbNoCreatedLE = 0;
    int nbROI = 0;
    int nbRoiNotAnalysed = 0;
    int roicounter = 0;

    unsigned char * ptr;

    // Loop pixel's subdivision.
    for(int i = 0; i < subdivision.rows; i++) {

        ptr = subdivision.ptr<unsigned char>(i);

        for(int j = 0; j < subdivision.cols; j++) {

            // Pixel is white.
            if((int)ptr[j] > 0) {

                // Check if we are not out of frame range when a ROI is defined at the current pixel location.
                if((subdivisionPos.y + i - mRoiSize[1]/2 > 0) &&
                   (subdivisionPos.y + i + mRoiSize[1]/2 < absDiffBinaryMap.rows) &&
                   (subdivisionPos.x + j - mRoiSize[0]/2 > 0) &&
                   (subdivisionPos.x + j + mRoiSize[0]/2 < absDiffBinaryMap.cols)) {

                    msg = msg
                        + "Analyse ROI ("
                        +  Conversion::intToString(subdivisionPos.x + j) + ";" + Conversion::intToString(subdivisionPos.y + i) + ")\n";

                    nbROI++;
                    roicounter++;
                    // Get colors in eventMap at the current ROI location.
                    vector<Scalar> listColorInRoi = getColorInEventMap(eventMap, Point(subdivisionPos.x + j, subdivisionPos.y + i));

                    if(listColorInRoi.size() == 0)  situation = 0;  // black color = create a new local event
                    if(listColorInRoi.size() == 1)  situation = 1;  // one color = add the current roi to an existing local event
                    if(listColorInRoi.size() >  1)  situation = 2;  // several colors = make a decision

                    switch(situation) {

                        case 0 :

                            {

                                if(listLE.size() < maxNbLE) {

                                    msg = msg
                                        + "->CREATE New Local EVENT\n"
                                        + "  - Initial position : ("
                                        +  Conversion::intToString(subdivisionPos.x + j) + ";" + Conversion::intToString(subdivisionPos.y + i) + ")\n"
                                        + "  - Color : (" + Conversion::intToString(mListColors.at(listLE.size())[0]) + ";"
                                        + Conversion::intToString(mListColors.at(listLE.size())[1]) + ";"
                                        + Conversion::intToString(mListColors.at(listLE.size())[2]) + ")\n";

                                    // Create new localEvent object.
                                    LocalEvent newLocalEvent(   mListColors.at(listLE.size()),
                                                                Point(subdivisionPos.x + j, subdivisionPos.y + i),
                                                                absDiffBinaryMap.rows,
                                                                absDiffBinaryMap.cols,
                                                                mRoiSize);

                                    // Extract white pixels in ROI.
                                    vector<Point> whitePixAbsDiff,whitePixPosDiff, whitePixNegDiff;
                                    Mat roiAbsDiff, roiPosDiff, roiNegDiff;

                                    absDiffBinaryMap(Rect(subdivisionPos.x + j - mRoiSize[0]/2, subdivisionPos.y + i - mRoiSize[1]/2, mRoiSize[0], mRoiSize[1])).copyTo(roiAbsDiff);
                                    posDiff(Rect(subdivisionPos.x + j - mRoiSize[0]/2, subdivisionPos.y + i - mRoiSize[1]/2, mRoiSize[0], mRoiSize[1])).copyTo(roiPosDiff);
                                    negDiff(Rect(subdivisionPos.x + j - mRoiSize[0]/2, subdivisionPos.y + i - mRoiSize[1]/2, mRoiSize[0], mRoiSize[1])).copyTo(roiNegDiff);

                                    if(roiPosDiff.type() == CV_16UC1 && roiNegDiff.type() == CV_16UC1) {

                                        unsigned char * ptrRoiAbsDiff;
                                        unsigned short * ptrRoiPosDiff;
                                        unsigned short * ptrRoiNegDiff;

		                                for(int a = 0; a < roiAbsDiff.rows; a++) {

			                                ptrRoiAbsDiff = roiAbsDiff.ptr<unsigned char>(a);
                                            ptrRoiPosDiff = roiPosDiff.ptr<unsigned short>(a);
                                            ptrRoiNegDiff = roiNegDiff.ptr<unsigned short>(a);

			                                for(int b = 0; b < roiAbsDiff.cols; b++){

				                                if(ptrRoiAbsDiff[b] > 0) whitePixAbsDiff.push_back(Point(subdivisionPos.x + j - mRoiSize[0]/2 + b, subdivisionPos.y + i - mRoiSize[1]/2 + a));
                                                if(ptrRoiPosDiff[b] > posDiffThreshold) whitePixPosDiff.push_back(Point(subdivisionPos.x + j - mRoiSize[0]/2 + b, subdivisionPos.y + i - mRoiSize[1]/2 + a));
                                                if(ptrRoiNegDiff[b] > negDiffThreshold) whitePixNegDiff.push_back(Point(subdivisionPos.x + j - mRoiSize[0]/2 + b, subdivisionPos.y + i - mRoiSize[1]/2 + a));

			                                }
		                                }

                                    }else if(roiPosDiff.type() == CV_8UC1 && roiNegDiff.type() == CV_8UC1) {

                                        unsigned char * ptrRoiAbsDiff;
                                        unsigned char * ptrRoiPosDiff;
                                        unsigned char * ptrRoiNegDiff;

		                                for(int a = 0; a < roiAbsDiff.rows; a++) {

			                                ptrRoiAbsDiff = roiAbsDiff.ptr<unsigned char>(a);
                                            ptrRoiPosDiff = roiPosDiff.ptr<unsigned char>(a);
                                            ptrRoiNegDiff = roiNegDiff.ptr<unsigned char>(a);

			                                for(int b = 0; b < roiAbsDiff.cols; b++){

				                                if(ptrRoiAbsDiff[b] > 0) whitePixAbsDiff.push_back(Point(subdivisionPos.x + j - mRoiSize[0]/2 + b, subdivisionPos.y + i - mRoiSize[1]/2 + a));
                                                if(ptrRoiPosDiff[b] > posDiffThreshold) whitePixPosDiff.push_back(Point(subdivisionPos.x + j - mRoiSize[0]/2 + b, subdivisionPos.y + i - mRoiSize[1]/2 + a));
                                                if(ptrRoiNegDiff[b] > negDiffThreshold) whitePixNegDiff.push_back(Point(subdivisionPos.x + j - mRoiSize[0]/2 + b, subdivisionPos.y + i - mRoiSize[1]/2 + a));

			                                }
		                                }
                                    }

                                    msg = msg + "Number white pix in abs diff : " + Conversion::intToString(whitePixAbsDiff.size()) + "\n";
                                    msg = msg + "Number white pix in pos diff : " + Conversion::intToString(whitePixPosDiff.size()) + "\n";
                                    msg = msg + "Number white pix in neg diff : " + Conversion::intToString(whitePixNegDiff.size()) + "\n";

                                    newLocalEvent.addAbs(whitePixAbsDiff);
                                    newLocalEvent.addPos(whitePixPosDiff);
                                    newLocalEvent.addNeg(whitePixNegDiff);

                                    // Update center of mass.
                                    newLocalEvent.computeMassCenter();
                                    msg = msg
                                        + "  - Center of mass abs pixels : ("
                                        +  Conversion::intToString(newLocalEvent.getMassCenter().x) + ";" + Conversion::intToString(newLocalEvent.getMassCenter().y) + ")\n";

                                    // Save the frame number where the local event has been created.
                                    newLocalEvent.setNumFrame(numFrame);
                                    // Add LE in the list of localEvent.
                                    listLE.push_back(newLocalEvent);
                                    // Update eventMap with the color of the new localEvent.
                                    Mat roi(mRoiSize[1], mRoiSize[0], CV_8UC3, mListColors.at(listLE.size()-1));
                                    roi.copyTo(eventMap(Rect(subdivisionPos.x + j-mRoiSize[0]/2, subdivisionPos.y + i-mRoiSize[1]/2, mRoiSize[0], mRoiSize[1])));
                                    // Color roi in black in the current region.
                                    colorRoiInBlack(Point(j,i), mRoiSize[1], mRoiSize[0], subdivision);

                                    colorRoiInBlack(Point(subdivisionPos.x +j,subdivisionPos.y +i), mRoiSize[1], mRoiSize[0], absDiffBinaryMap);
                                    colorRoiInBlack(Point(subdivisionPos.x +j,subdivisionPos.y +i), mRoiSize[1], mRoiSize[0], posDiff);
                                    colorRoiInBlack(Point(subdivisionPos.x +j,subdivisionPos.y +i), mRoiSize[1], mRoiSize[0], negDiff);

                                    nbCreatedLE++;

                                }else {

                                    nbNoCreatedLE++;

                                }
                            }

                            break;

                        case 1 :

                            {

                                vector<LocalEvent>::iterator it;
                                int index = 0;
                                for(it=listLE.begin(); it!=listLE.end(); ++it){

                                    // Try to find a local event which has the same color.
                                    if((*it).getColor() == listColorInRoi.at(0)){

                                        msg = msg
                                            + "->Attach ROI ("
                                            +  Conversion::intToString(subdivisionPos.x + j) + ";" + Conversion::intToString(subdivisionPos.y + i) + ") with LE " + Conversion::intToString(index) + "\n";

                                        // Extract white pixels in ROI.
                                        vector<Point> whitePixAbsDiff,whitePixPosDiff, whitePixNegDiff;
                                        Mat roiAbsDiff, roiPosDiff, roiNegDiff;

                                        absDiffBinaryMap(Rect(subdivisionPos.x + j - mRoiSize[0]/2, subdivisionPos.y + i - mRoiSize[1]/2, mRoiSize[0], mRoiSize[1])).copyTo(roiAbsDiff);
                                        posDiff(Rect(subdivisionPos.x + j - mRoiSize[0]/2, subdivisionPos.y + i - mRoiSize[1]/2, mRoiSize[0], mRoiSize[1])).copyTo(roiPosDiff);
                                        negDiff(Rect(subdivisionPos.x + j - mRoiSize[0]/2, subdivisionPos.y + i - mRoiSize[1]/2, mRoiSize[0], mRoiSize[1])).copyTo(roiNegDiff);

                                        if(roiPosDiff.type() == CV_16UC1 && roiNegDiff.type() == CV_16UC1) {

                                            unsigned char * ptrRoiAbsDiff;
                                            unsigned short * ptrRoiPosDiff;
                                            unsigned short * ptrRoiNegDiff;

		                                    for(int a = 0; a < roiAbsDiff.rows; a++) {

			                                    ptrRoiAbsDiff = roiAbsDiff.ptr<unsigned char>(a);
                                                ptrRoiPosDiff = roiPosDiff.ptr<unsigned short>(a);
                                                ptrRoiNegDiff = roiNegDiff.ptr<unsigned short>(a);

			                                    for(int b = 0; b < roiAbsDiff.cols; b++){

				                                    if(ptrRoiAbsDiff[b] > 0) whitePixAbsDiff.push_back(Point(subdivisionPos.x + j - mRoiSize[0]/2 + b, subdivisionPos.y + i - mRoiSize[1]/2 + a));
                                                    if(ptrRoiPosDiff[b] > posDiffThreshold) whitePixPosDiff.push_back(Point(subdivisionPos.x + j - mRoiSize[0]/2 + b, subdivisionPos.y + i - mRoiSize[1]/2 + a));
                                                    if(ptrRoiNegDiff[b] > negDiffThreshold) whitePixNegDiff.push_back(Point(subdivisionPos.x + j - mRoiSize[0]/2 + b, subdivisionPos.y + i - mRoiSize[1]/2 + a));

			                                    }
		                                    }

                                        }else if(roiPosDiff.type() == CV_8UC1 && roiNegDiff.type() == CV_8UC1) {

                                            unsigned char * ptrRoiAbsDiff;
                                            unsigned char * ptrRoiPosDiff;
                                            unsigned char * ptrRoiNegDiff;

		                                    for(int a = 0; a < roiAbsDiff.rows; a++) {

			                                    ptrRoiAbsDiff = roiAbsDiff.ptr<unsigned char>(a);
                                                ptrRoiPosDiff = roiPosDiff.ptr<unsigned char>(a);
                                                ptrRoiNegDiff = roiNegDiff.ptr<unsigned char>(a);

			                                    for(int b = 0; b < roiAbsDiff.cols; b++){

				                                    if(ptrRoiAbsDiff[b] > 0) whitePixAbsDiff.push_back(Point(subdivisionPos.x + j - mRoiSize[0]/2 + b, subdivisionPos.y + i - mRoiSize[1]/2 + a));
                                                    if(ptrRoiPosDiff[b] > posDiffThreshold) whitePixPosDiff.push_back(Point(subdivisionPos.x + j - mRoiSize[0]/2 + b, subdivisionPos.y + i - mRoiSize[1]/2 + a));
                                                    if(ptrRoiNegDiff[b] > negDiffThreshold) whitePixNegDiff.push_back(Point(subdivisionPos.x + j - mRoiSize[0]/2 + b, subdivisionPos.y + i - mRoiSize[1]/2 + a));

			                                    }
		                                    }
                                        }

                                        msg = msg + "Number white pix in abs diff : " + Conversion::intToString(whitePixAbsDiff.size()) + "\n";
                                        msg = msg + "Number white pix in pos diff : " + Conversion::intToString(whitePixPosDiff.size()) + "\n";
                                        msg = msg + "Number white pix in neg diff : " + Conversion::intToString(whitePixNegDiff.size()) + "\n";

                                        (*it).addAbs(whitePixAbsDiff);
                                        (*it).addPos(whitePixPosDiff);
                                        (*it).addNeg(whitePixNegDiff);

                                        // Add the current roi.
                                        (*it).mLeRoiList.push_back(Point(subdivisionPos.x + j, subdivisionPos.y + i));
                                        // Set local event 's map
                                        (*it).setMap(Point(subdivisionPos.x + j - mRoiSize[0]/2, subdivisionPos.y + i - mRoiSize[1]/2), mRoiSize[1], mRoiSize[0]);
                                        // Update center of mass
                                        (*it).computeMassCenter();
                                        msg = msg
                                        + "  - Update Center of mass abs pixels of LE " +  Conversion::intToString(index) + " : ("
                                        +  Conversion::intToString((*it).getMassCenter().x) + ";" + Conversion::intToString((*it).getMassCenter().y) + ")\n";

                                        // Update eventMap with the color of the new localEvent
                                        Mat roi(mRoiSize[1], mRoiSize[0], CV_8UC3, listColorInRoi.at(0));
                                        roi.copyTo(eventMap(Rect(subdivisionPos.x + j-mRoiSize[0]/2, subdivisionPos.y + i-mRoiSize[1]/2,mRoiSize[0],mRoiSize[1])));
                                        // Color roi in black in thresholded frame.
                                        Mat roiBlack(mRoiSize[1],mRoiSize[0],CV_8UC1,Scalar(0));
                                        roiBlack.copyTo(absDiffBinaryMap(Rect(subdivisionPos.x + j-mRoiSize[0]/2, subdivisionPos.y + i-mRoiSize[1]/2,mRoiSize[0],mRoiSize[1])));
                                        // Color roi in black in the current region.
                                        colorRoiInBlack(Point(j,i), mRoiSize[1], mRoiSize[0], subdivision);

                                        colorRoiInBlack(Point(subdivisionPos.x +j,subdivisionPos.y +i), mRoiSize[1], mRoiSize[0], absDiffBinaryMap);
                                        colorRoiInBlack(Point(subdivisionPos.x +j,subdivisionPos.y +i), mRoiSize[1], mRoiSize[0], posDiff);
                                        colorRoiInBlack(Point(subdivisionPos.x +j,subdivisionPos.y +i), mRoiSize[1], mRoiSize[0], negDiff);

                                        nbRoiAttachedToLE++;

                                        break;

                                    }

                                    index++;
                                }
                            }

                            break;

                        case 2 :

                            {
                                nbRoiNotAnalysed++;

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

    msg = msg
        + "--> RESUME REGION ANALYSE : \n"
        + "Number of analysed ROI : "
        + Conversion::intToString(nbROI) + "\n"
        + "Number of not analysed ROI : "
        + Conversion::intToString(nbRoiNotAnalysed) + "\n"
        + "Number of new LE : "
        + Conversion::intToString(nbCreatedLE) + "\n"
        + "Number of updated LE :"
        + Conversion::intToString(nbRoiAttachedToLE) + "\n";

}
