/*
                    DetectionTemporal.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*   This file is part of:   freeture
*
*   Copyright:      (C) 2014-2015 Yoan Audureau
*                               FRIPON-GEOPS-UPSUD-CNRS
*
*   License:        GNU General Public License
*
*   FreeTure is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*   FreeTure is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*   You should have received a copy of the GNU General Public License
*   along with FreeTure. If not, see <http://www.gnu.org/licenses/>.
*
*   Last modified:      20/07/2015
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

DetectionTemporal::DetectionTemporal(double timeBefore, string cfgPath):
mDownsampleEnabled(false), mSaveGeMap(false), mSavePos(false), mSaveDirMap(false),
mSaveGeInfos(false),mImgNum(0), mDebugUpdateMask(false),
mSubdivisionStatus(false), mDebugEnabled(false), mDebugVideo(false),
mDataSetCounter(0), mTimeBeforeEvent(timeBefore) {

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

    Configuration cfg;
    cfg.Load(cfgPath);

    //********************* DEBUG OPTION.******************************

    if(!cfg.Get("DET_DEBUG", mDebugEnabled)) {
        mDebugEnabled = false;
        BOOST_LOG_SEV(logger, warning) << "Fail to load DET_DEBUG from configuration file. Set to false.";
    }

    //********************* DOWNSAMPLE OPTION *************************

    if(!cfg.Get("DET_DOWNSAMPLE_ENABLED", mDownsampleEnabled)) {
        mDownsampleEnabled = true;
        BOOST_LOG_SEV(logger, warning) << "Fail to load DET_DOWNSAMPLE_ENABLED from configuration file. Set to true.";
    }

    //********************* GEMAP OPTION ******************************

    if(!cfg.Get("DET_SAVE_GEMAP", mSaveGeMap)) {
        mSaveGeMap = false;
        BOOST_LOG_SEV(logger, warning) << "Fail to load DET_SAVE_GEMAP from configuration file. Set to false.";
    }

    //********************* DIRMAP OPTION *****************************

    if(!cfg.Get("DET_SAVE_DIRMAP", mSaveDirMap)) {
        mSaveDirMap = false;
        BOOST_LOG_SEV(logger, warning) << "Fail to load DET_SAVE_DIRMAP from configuration file. Set to false.";
    }

    //********************* SAVE POSITION OPTION **********************

    if(!cfg.Get("DET_SAVE_POS", mSavePos)) {
        mSavePos = true;
        BOOST_LOG_SEV(logger, warning) << "Fail to load DET_SAVE_POS from configuration file. Set to true.";
    }

    //********************* SAVE GE OPTION ****************************

    if(!cfg.Get("DET_SAVE_GE_INFOS", mSaveGeInfos)) {
        mSaveGeInfos = false;
        BOOST_LOG_SEV(logger, warning) << "Fail to load DET_SAVE_GE_INFOS from configuration file. Set to false.";
    }

    //********************* USE MASK OPTION ***************************
    bool maskEnabled = false;
    if(!cfg.Get("ACQ_MASK_ENABLED", maskEnabled)) {
        BOOST_LOG_SEV(logger, warning) << "Fail to load ACQ_MASK_ENABLED from configuration file. Set to false.";
    }

    //********************* UPDATE MASK *******************************
    int updateMaskFrequency = 10;
    if(!cfg.Get("DET_UPDATE_MASK_FREQUENCY", updateMaskFrequency)) {
        BOOST_LOG_SEV(logger, warning) << "Fail to load DET_UPDATE_MASK_FREQUENCY from configuration file. Set to 10.";
    }


    //********************* UPDATE MASK *******************************
    bool updateMask = false;
    if(!cfg.Get("DET_UPDATE_MASK", updateMask)) {
        BOOST_LOG_SEV(logger, warning) << "Fail to load DET_UPDATE_MASK from configuration file. Set to false.";
    }


    //********************* DEBUG UPDATE MASK *************************

    if(!cfg.Get("DET_DEBUG_UPDATE_MASK", mDebugUpdateMask)) {
        mDebugUpdateMask = false;
        BOOST_LOG_SEV(logger, warning) << "Fail to load DET_DEBUG_UPDATE_MASK from configuration file. Set to false.";
    }

    //********************* MASK PATH *****************************
    string maskPath;
    if(!cfg.Get("ACQ_MASK_PATH", maskPath)) {
        throw "Fail to load ACQ_MASK_PATH from configuration file.";
    }

    string acqBitDepth;
    cfg.Get("ACQ_BIT_DEPTH", acqBitDepth);
    EParser<CamBitDepth> camBitDepth;

    mMaskManager = new Mask(updateMaskFrequency, maskEnabled, maskPath, mDownsampleEnabled, camBitDepth.parseEnum("ACQ_BIT_DEPTH", acqBitDepth), updateMask);

    // Create local mask to eliminate single white pixels.
    Mat maskTemp(3,3,CV_8UC1,Scalar(255));
    maskTemp.at<uchar>(1, 1) = 0;
    maskTemp.copyTo(mLocalMask);

    //********************* DEBUG PATH ********************************

    if(!cfg.Get("DET_DEBUG_PATH", mDebugPath)) {
        mDebugEnabled = false;
        mDebugPath = "";
        BOOST_LOG_SEV(logger, warning) << "Error about DET_DEBUG_PATH from configuration file. DET_DEBUG option disabled.";
    }

    mDebugPath = mDebugPath + "/";

    mDebugCurrentPath = mDebugPath;

    //********************* DEBUG VIDEO OPTION ***********************

    if(!cfg.Get("DET_DEBUG_VIDEO", mDebugVideo)) {
        mDebugVideo = false;
        BOOST_LOG_SEV(logger, warning) << "Fail to load DET_DEBUG_VIDEO from configuration file. Set to false.";
    }

    // Create directories for debugging method.
    if(mDebugEnabled)
        createDebugDirectories(true);

    // Create debug video.
    if(mDebugVideo)
        mVideoDebug = VideoWriter(mDebugCurrentPath + "debug-video.avi", CV_FOURCC('M', 'J', 'P', 'G'), 5, Size(static_cast<int>(1280), static_cast<int>(960)), true);

}

DetectionTemporal::~DetectionTemporal() {
    if(mMaskManager != NULL)
        delete mMaskManager;
}

void DetectionTemporal::resetDetection(bool loadNewDataSet){

    BOOST_LOG_SEV(logger, notification) << "Clear global events list.";
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

    mMaskManager->resetMask();

}

void DetectionTemporal::createDebugDirectories(bool cleanDebugDirectory){

    mDebugCurrentPath = mDebugPath + "debug_" + Conversion::intToString(mDataSetCounter) + "/" ;

    if(cleanDebugDirectory) {

        const boost::filesystem::path p0 = path(mDebugPath);

        if(boost::filesystem::exists(p0)) {
            boost::filesystem::remove_all(p0);
        }else {
            boost::filesystem::create_directories(p0);
        }

    }

    const boost::filesystem::path p1 = path(mDebugCurrentPath);

    if(!boost::filesystem::exists(p1))
        boost::filesystem::create_directories(p1);

    vector<string> debugSubDir = {  "original",
                                    "absolute_difference",
                                    "absolute_difference_thresholded",
                                    "event_map_initial",
                                    "event_map_filtered",
                                    "absolute_difference_dilated",
                                    "neg_difference_thresholded",
                                    "pos_difference_thresholded",
                                    "neg_difference",
                                    "pos_difference"};

    for(int i = 0; i< debugSubDir.size(); i++){

        const boost::filesystem::path path(mDebugCurrentPath + debugSubDir.at(i));

        if(!boost::filesystem::exists(path)) {
            boost::filesystem::create_directories(path);
        }

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

    // ################# Save approximate positions. #################

    if(mSavePos){

        ofstream posFile;
        string posFilePath = p + "positions.txt";
        posFile.open(posFilePath.c_str());

        // Number of the first frame associated to the event.
        int numFirstFrame = -1;

        vector<LocalEvent>::iterator itLe;
        for(itLe = (*mGeToSave).LEList.begin(); itLe!=(*mGeToSave).LEList.end(); ++itLe) {

            if(numFirstFrame == -1)
                numFirstFrame = (*itLe).getNumFrame() - mTimeBeforeEvent;

            Point pos = (*itLe).getMassCenter();

            int positionY = 0;
            if(mDownsampleEnabled) {
                pos*=2;
                positionY = mPrevFrame.rows*2 - pos.y;
            }else {
                positionY = mPrevFrame.rows - pos.y;
            }

            // NUM_FRAME    POSITIONX     POSITIONY (inversed)
            string line = Conversion::intToString((*itLe).getNumFrame() - numFirstFrame) + "               (" + Conversion::intToString(pos.x)  + ";" + Conversion::intToString(positionY) + ")                 " + TimeDate::getIsoExtendedFormatDate((*itLe).mFrameAcqDate)+ "\n";
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

    if(!mSubdivisionStatus) {

        mSubdivisionPos.clear();

        int h = c.mImg.rows;
        int w = c.mImg.cols;

        if(mDownsampleEnabled) {
            h /= 2;
            w /= 2;
        }

        subdivideFrame(mSubdivisionPos, 8, h, w);
        mSubdivisionStatus = true;

        if(mDebugEnabled) {

            Mat s = Mat(h, w,CV_8UC1,Scalar(0));

            for(int i = 0; i < 8; i++) {
               line(s, Point(0, i * (h/8)), Point(w - 1, i * (h/8)), Scalar(255), 1);
               line(s, Point(i * (w/8), 0), Point(i * (w/8), h-1), Scalar(255), 1);
            }

            SaveImg::saveBMP(s, mDebugCurrentPath + "subdivisions_map");

        }

    }else {

        double tDownsample = 0;
        double tAbsDiff = 0;
        double tPosDiff = 0;
        double tNegDiff = 0;
        double tDilate = 0;
        double tThreshold = 0;
        double tStep1 = 0;
        double tStep2 = 0;
        double tStep3 = 0;
        double tStep4 = 0;
        double tTotal = (double)getTickCount();
        ofstream dlog;

        if(mDebugEnabled) {

            const boost::filesystem::path logDir = path(mDebugCurrentPath + "/logs");

            if(!boost::filesystem::exists(logDir))
                boost::filesystem::create_directories(logDir);

            string pathLogFileFrame = mDebugCurrentPath + "/logs/cframe_" + Conversion::intToString(c.mFrameNumber);
            dlog.open(pathLogFileFrame.c_str());

        }

        /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        /// %%%%%%%%%%%%%%%%%%%%%%%%%%% STEP 1 : FILETRING / THRESHOLDING %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        dlog << "###################### STEP 1 ######################\n";
        double tstep1 = (double)getTickCount();
        Mat currImg;

        // ------------------------------
        //    Downsample current frame.
        // -------------------------------

        if(mDownsampleEnabled) {

            tDownsample = (double)getTickCount();
            pyrDown(c.mImg, currImg, Size(c.mImg.cols / 2, c.mImg.rows / 2));
            tDownsample = ((double)getTickCount() - tDownsample);

        }else {

            c.mImg.copyTo(currImg);

        }

        // Apply mask on currImg.
        // If true is returned, it means that the mask has been updated and applied on currImg. Detection process can't continue.
        // If false is returned, it means that the mask has not been updated. Detection process can continue.
        if(!mMaskManager->applyMask(currImg)) {

            //SaveImg::saveJPEG(Conversion::convertTo8UC1(currImg), mDebugPath + "/original/orig_" + Conversion::intToString(c.mFrameNumber));

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
            tAbsDiff = (double)getTickCount();
            cv::absdiff(currImg, mPrevFrame, absdiffImg);
            tAbsDiff = (double)getTickCount() - tAbsDiff;

            // Positive difference.
            tPosDiff = (double)getTickCount();
            cv::subtract(currImg,mPrevFrame,posDiffImg,mMaskManager->mCurrentMask);
            tPosDiff = (double)getTickCount() - tPosDiff;

            // Negative difference.
            tNegDiff = (double)getTickCount();
            cv::subtract(mPrevFrame,currImg,negDiffImg,mMaskManager->mCurrentMask);
            tNegDiff = (double)getTickCount() - tNegDiff;

            // ---------------------------------
            //  Dilatate absolute difference.
            // ---------------------------------

            tDilate = (double)getTickCount();
            int dilation_size = 2;
            Mat element = getStructuringElement(MORPH_RECT, Size(2*dilation_size + 1, 2*dilation_size+1), Point(dilation_size, dilation_size));
            cv::dilate(absdiffImg, absdiffImg, element);
            tDilate = (double)getTickCount() - tDilate;

            if(mDebugEnabled) {

                SaveImg::saveBMP(absdiffImg, mDebugCurrentPath + "/absolute_difference_dilated/frame_" + Conversion::intToString(c.mFrameNumber));

            }

            // ------------------------------------------------------------------------------
            //   Threshold absolute difference / positive difference / negative difference
            // ------------------------------------------------------------------------------

            tThreshold = (double)getTickCount();
            Mat absDiffBinaryMap = ImgProcessing::thresholding(absdiffImg, mMaskManager->mCurrentMask, 3, Thresh::MEAN);
            tThreshold = (double)getTickCount() - tThreshold;

            Scalar meanPosDiff, stddevPosDiff, meanNegDiff, stddevNegDiff;
            meanStdDev(posDiffImg, meanPosDiff, stddevPosDiff, mMaskManager->mCurrentMask);
            meanStdDev(negDiffImg, meanNegDiff, stddevNegDiff, mMaskManager->mCurrentMask);
            int posThreshold = stddevPosDiff[0] * 5 + 10;
            int negThreshold = stddevNegDiff[0] * 5 + 10;

            if(mDebugEnabled) {

                Mat posBinaryMap = ImgProcessing::thresholding(posDiffImg, mMaskManager->mCurrentMask, 5, Thresh::STDEV);
                Mat negBinaryMap = ImgProcessing::thresholding(negDiffImg, mMaskManager->mCurrentMask, 5, Thresh::STDEV);

                SaveImg::saveBMP(Conversion::convertTo8UC1(currImg), mDebugCurrentPath + "/original/frame_" + Conversion::intToString(c.mFrameNumber));
                SaveImg::saveBMP(posBinaryMap, mDebugCurrentPath + "/pos_difference_thresholded/frame_" + Conversion::intToString(c.mFrameNumber));
                SaveImg::saveBMP(negBinaryMap, mDebugCurrentPath + "/neg_difference_thresholded/frame_" + Conversion::intToString(c.mFrameNumber));
                SaveImg::saveBMP(absDiffBinaryMap, mDebugCurrentPath + "/absolute_difference_thresholded/frame_" + Conversion::intToString(c.mFrameNumber));
                SaveImg::saveBMP(absdiffImg, mDebugCurrentPath + "/absolute_difference/frame_"+Conversion::intToString(c.mFrameNumber));
                SaveImg::saveBMP(Conversion::convertTo8UC1(posDiffImg), mDebugCurrentPath + "/pos_difference/frame_" + Conversion::intToString(c.mFrameNumber));
                SaveImg::saveBMP(Conversion::convertTo8UC1(negDiffImg), mDebugCurrentPath + "/neg_difference/frame_" + Conversion::intToString(c.mFrameNumber));

            }

            // Current frame is stored as the previous frame.
            currImg.copyTo(mPrevFrame);

            tStep1 = (double)getTickCount() - tStep1;

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

            vector <LocalEvent> listLocalEvents;
            vector<LocalEvent>::iterator itLE;
            tStep2 = (double)getTickCount();

            // Event map for the current frame.
            Mat eventMap = Mat(currImg.rows,currImg.cols, CV_8UC3,Scalar(0,0,0));

            // ----------------------------------
            //        Search local events.
            // ----------------------------------

            // Iterator on list of sub-regions.
            vector<Point>::iterator itR;

            // Analyse subdivisions.
            if(mDebugEnabled) dlog << "#################### ANALYSE SUBDIVISIONS ####################\n";

            for(itR = mSubdivisionPos.begin(); itR != mSubdivisionPos.end(); ++itR) {

                if(mDebugEnabled) dlog << "----> REGION (" << (*itR).x << ";" << (*itR).y << ")\n";

                // Extract subdivision from binary map.
                Mat subdivision = absDiffBinaryMap(Rect((*itR).x, (*itR).y, absDiffBinaryMap.cols/8, absDiffBinaryMap.rows/8));

                if(mDebugEnabled) {

                    dlog << "LE NUMBER : " << listLocalEvents.size() << "\n";
                    dlog << "---------------------------------------------------\n";
                    for(int i = 0; i < listLocalEvents.size(); i++)
                        dlog << ">> LE " << i << " : " << listLocalEvents.at(i).getMassCenter() << "\n";
                    dlog << "---------------------------------------------------\n";

                }

                // Check if there is white pixels.
                if(countNonZero(subdivision) > 0){

                    if(mDebugEnabled) dlog << "NOT EMPTY REGION\n" ;

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
                                    debugMsg,
                                    c.mDate);

                    if(mDebugEnabled) dlog << debugMsg;

                }else {

                    if(mDebugEnabled) dlog << "EMPTY REGION\n" ;

                }

            }

            for(int i = 0; i < listLocalEvents.size(); i++)
                listLocalEvents.at(i).setLeIndex(i);

            if(mDebugEnabled)
                SaveImg::saveBMP(eventMap, mDebugCurrentPath + "/event_map_initial/frame_" + Conversion::intToString(c.mFrameNumber));

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

                    if(mDebugEnabled)dlog << "LE " << (*itLE).getLeIndex() << " has no negative cluster.\n";
                    itLePos.push_back(itLE);

                }else if(!(*itLE).getPosClusterStatus() && (*itLE).getNegClusterStatus()){

                    if(mDebugEnabled)dlog << "LE " << (*itLE).getLeIndex() << " has no negative cluster.\n";
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

                    if(mDebugEnabled)dlog << "MERGE LE " << (*itLePos.at(i)).getLeIndex() << " with LE " << (*itChoose).getLeIndex() << " \n";
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

                    if(mDebugEnabled)dlog << "Remove LE "  << (*itLE).getLeIndex() << " \n";
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

                        dlog << "Remove LE "  << (*itLE).getLeIndex() << " \n";
                        itLE = listLocalEvents.erase(itLE);

                    }

                }else {
                    ++itLE;
                }
            }

            if(mDebugEnabled) {

                Mat eventMapFiltered = Mat(currImg.rows,currImg.cols, CV_8UC3,Scalar(0,0,0));

                for(int i = 0; i < listLocalEvents.size(); i++) {

                    Mat roiF(10, 10, CV_8UC3, listLocalEvents.at(i).getColor());

                    for(int j = 0; j < listLocalEvents.at(i).mLeRoiList.size();j++) {
                        if( listLocalEvents.at(i).mLeRoiList.at(j).x-5 > 0 &&
                            listLocalEvents.at(i).mLeRoiList.at(j).x+5 < eventMapFiltered.cols &&
                            listLocalEvents.at(i).mLeRoiList.at(j).y-5 > 0 &&
                            listLocalEvents.at(i).mLeRoiList.at(j).y+5 < eventMapFiltered.rows) {
                            roiF.copyTo(eventMapFiltered(Rect(listLocalEvents.at(i).mLeRoiList.at(j).x - 5, listLocalEvents.at(i).mLeRoiList.at(j).y - 5, 10, 10)));
                        }
                    }

                }

                SaveImg::saveBMP(eventMapFiltered, mDebugCurrentPath + "/event_map_filtered/frame_" + Conversion::intToString(c.mFrameNumber));

            }

            tStep2 = (double)getTickCount() - tStep2;

            /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            /// %%%%%%%%%%%%%%%%%%%%%%%%%% STEP 3 : ATTACH LE TO GE OR CREATE NEW ONE %%%%%%%%%%%%%%%%%%%%%%%%%
            /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

            // SUMMARY :
            // Loop list of local events.
            // Create a new global event initialized with the current Local event or attach it to an existing global event.
            // If attached, check the positive-negative couple of the global event.

            // Iterator on list of global event.
            vector<GlobalEvent>::iterator itGE;

            tStep3 = (double)getTickCount();

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

            tStep3 = (double)getTickCount() - tStep3;

            /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%% STEP 4 : MANAGE LIST GLOBAL EVENT %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

            if(mDebugEnabled) {
                dlog << " ########## STEP 4 : MANAGE LIST GLOBAL EVENT ##########\n";
                dlog << "Size list GE : " << mListGlobalEvents.size() << "\n\n";
            }

            tStep4 = (double)getTickCount();

            itGE = mListGlobalEvents.begin();

            bool saveSignal = false;

            int nGe = 0;

            while(itGE != mListGlobalEvents.end()){

                if(mDebugEnabled) dlog << "----------- GE " << nGe << " -----------\n";

                // Increment age.
                (*itGE).setAge((*itGE).getAge() + 1);
                if(mDebugEnabled) dlog << "-> AGE :  " << (*itGE).getAge() << "\n";

                // The current global event has not received a new local event.
                if(!(*itGE).getNewLEStatus()){

                    // Increment age without any new local event.
                    (*itGE).setAgeLastElem((*itGE).getAgeLastElem()+1);

                }else{

                    (*itGE).setNumLastFrame(c.mFrameNumber);
                    (*itGE).setNewLEStatus(false);

                }

                if(mDebugEnabled) dlog << "-> AGE LAST ELEM :  " << (*itGE).getAgeLastElem() << "\n";
                if(mDebugEnabled) dlog << "-> NUM LAST IMG  :  " << (*itGE).getNumLastFrame() << "\n";

                string msgGe = "";
                // CASE 1 : Finished event.
                if((*itGE).getAgeLastElem() > 5){

                    //cout << "Finished event." << endl;
                    if(mDebugEnabled) dlog << "-> Finished event.\n";
                    dlog << "Vec LE : " << (*itGE).leDir << "\n";
                    dlog << "Vec GE : " << (*itGE).geDir << "\n";
                    dlog << "NB LE : " << (*itGE).LEList.size() << "\n";
                    // Linear profil ? Minimum duration respected ?
                    if((*itGE).LEList.size() >= 5 && (*itGE).continuousGoodPos(4, msgGe) && (*itGE).ratioFramesDist(msgGe) && (*itGE).negPosClusterFilter(msgGe)){

                        if(mDebugEnabled) dlog <<msgGe;
                        mGeToSave = itGE;
                        if(mDebugEnabled) dlog << "-> SAVE THIS GE  !  \n";
                        saveSignal = true;

                        break;


                    }else{

                        if(mDebugEnabled) dlog <<msgGe;
                        if(mDebugEnabled) dlog << "-> REMOVE THIS GE   \n";
                        // Delete the event.
                        itGE = mListGlobalEvents.erase(itGE);

                    }





                //CASE 2 : Not finished event.
                }else{
                    if(mDebugEnabled) dlog << "-> Not Finished event.\n";
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

            tStep4 = (double)getTickCount() - tStep4;

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

            tTotal = (double)getTickCount() - tTotal;

            if(mDebugEnabled) {

                dlog << "mDownsampleTime : " << (tDownsample/getTickFrequency())*1000 << " ms";
                dlog << "absdiffTime : " << (tAbsDiff/getTickFrequency())*1000 << " ms";
                dlog << "posdiffTime : " << (tPosDiff/getTickFrequency())*1000 << " ms";
                dlog << "negdiffTime : " << (tNegDiff/getTickFrequency())*1000 << " ms";
                dlog << "dilateTime : " << (tDilate/getTickFrequency())*1000 << " ms";
                dlog << "thresholdTime : " << (tThreshold/getTickFrequency())*1000 << " ms";
                dlog << "timeStep1 : " << (tStep1/getTickFrequency())*1000 << " ms";
                dlog << "timeStep2 : " << (tStep2/getTickFrequency())*1000 << " ms";
                dlog << "timeStep3 : " << (tStep3/getTickFrequency())*1000 << " ms";
                dlog << "timeStep4 : " << (tStep4/getTickFrequency())*1000 << " ms";
                dlog << "timeTotal : " << (tTotal/getTickFrequency())*1000 << " ms";
                dlog.close();

            }

            return saveSignal;

        }else{

            if(mDebugUpdateMask) {

                const boost::filesystem::path p = path(mDebugPath + "/mask/");

                if(!boost::filesystem::exists(p))
                    boost::filesystem::create_directories(p);

                SaveImg::saveJPEG(Conversion::convertTo8UC1(currImg), mDebugPath + "/mask/updateMask_" + Conversion::intToString(c.mFrameNumber));

            }

            currImg.copyTo(mPrevFrame);

        }
    }

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

    int posX = p.x - w;
    int posY = p.y - h;

    if(p.x - w < 0) {

        w = p.x + w/2;
        posX = 0;

    }else if(p.x + w/2 > region.cols) {

        w = region.cols - p.x + w/2;

    }

    if(p.y - h < 0) {

        h = p.y + h/2;
        posY = 0;


    }else if(p.y + h/2 > region.rows) {

        h = region.rows - p.y + h/2;

    }

    // Color roi in black in the current region.
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
                                    string &msg,
                                    TimeDate::Date cFrameDate){

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
                                // Save acquisition date of the frame.
                                newLocalEvent.mFrameAcqDate = cFrameDate;
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
