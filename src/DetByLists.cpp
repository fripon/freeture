/*
								DetByLists.cpp

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
* \file    DetByLists.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    13/06/2014
* \brief   Temporal meteor detection method.
*/

#include "DetByLists.h"

Point roiPositionToFramePosition(Point roiPix, Point newOrigine, Point framePix){

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

Mat buildMeteorTrail( GlobalEvent &ev, int h, int w, bool originalIntensity){

   /* Mat event = Mat::zeros(h,w, CV_8UC1);

    vector<LocalEvent>::iterator it;

    vector<LocalEvent> *p = ev.getListLocalEvent();

    for (it=p->begin(); it!=p->end(); ++it){

        LocalEvent &e = *it;

        vector< vector<PixelEvent> >::iterator it1;

        for (it1=e.listRoiPix.begin(); it1!=e.listRoiPix.end(); ++it1){

            vector<PixelEvent> &vpe = *it1;

            vector<PixelEvent>::iterator it2;

            for (it2=vpe.begin(); it2!=vpe.end(); ++it2){

                PixelEvent &pe = *it2;

               // cout << "val: "<< pe.intensity<<endl;

                if(originalIntensity)
                    event.at<uchar>(pe.position.y, pe.position.x) = (unsigned char)pe.intensity;
                else
                    event.at<uchar>(pe.position.y, pe.position.x) = 255;
            }
        }
    }

    return event;*/

}

vector<PixelEvent> createListPixSupThreshold(Mat &frame, const int *roiSize,Point framePixPos){

    src::severity_logger< LogSeverityLevel > log;


    Mat roi;
     vector<PixelEvent> listPix;

   // if(pixelFormat == 8){

        unsigned char * ptr;

        frame(Rect(framePixPos.x-roiSize[0]/2,framePixPos.y-roiSize[1]/2,roiSize[0],roiSize[1])).copyTo(roi);

        // loop roi
        for(int i = 0; i < roi.rows; i++){

            ptr = roi.ptr<unsigned char>(i);


            for(int j = 0; j < roi.cols; j++){

                if(ptr[j] > 0){


                        PixelEvent *pix = new PixelEvent(roiPositionToFramePosition(Point(j, i), Point(roiSize[0]/2, roiSize[1]/2), framePixPos), ptr[j]);

                        listPix.push_back(*pix);

                }
            }
        }

      //  cout << "Nb Pix > seuil in ROI : "<<listPixSupThresh.size()<<endl;

  /*  }else{


        unsigned short * ptr;

        frame(Rect(framePixPos.x-roiSize[0]/2,framePixPos.y-roiSize[1]/2,roiSize[0],roiSize[1])).copyTo(roi);

        // loop roi
        for(int i = 0; i < roi.rows; i++){

            ptr = roi.ptr<unsigned short>(i);


            for(int j = 0; j < roi.cols; j++){

                if(ptr[j] > 0){

                    PixelEvent *pix = new PixelEvent(roiPositionToFramePosition(Point(j, i), Point(roiSize[0]/2, roiSize[1]/2), framePixPos), ptr[j]);

                    listPix.push_back(*pix);

                }
            }
        }

    }
*/





    return listPix;
}

vector<Scalar> getColorInEventMap(Mat &eventMap, Point roiCenter, const int *roiSize){

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
/*
void buildRecEvent(GlobalEvent &gEvent, string recPath, vector<RecEvent> &listRecEvent, boost::mutex &m_listRecEvent){

  //GlobalEvent newGlobalEvent((*itLE),(*itLE).getMap(),date);
    RecEvent rec = gEvent.extractEventRecInfos();

    rec.setPath(recPath);

    boost::mutex::scoped_lock lock_listRecEvent(m_listRecEvent);

    listRecEvent.push_back(rec);

    lock_listRecEvent.unlock();

}*/

template <typename Container> void stringtok_(Container &container, string const &in, const char * const delimiters = "_"){

    const string::size_type len = in.length();
    string::size_type i = 0;

    while (i < len){

        // Eat leading whitespace
        i = in.find_first_not_of(delimiters, i);

        if (i == string::npos)

            return;   // Nothing left but white space

        // Find the end of the token
        string::size_type j = in.find_first_of(delimiters, i);

        // Push token
        if (j == string::npos){

            container.push_back(in.substr(i));

            return;

        }else

            container.push_back(in.substr(i, j-i));

        // Set up for next loop
        i = j + 1;

    }
}



int defineThreshold(Mat i, Mat &mask){

    Scalar imgMean;
    Scalar imgStddev;
    Mat newMask;
    mask.convertTo(newMask, CV_16UC1);

    meanStdDev(i, imgMean, imgStddev, mask);

    cout << "stdev : " << 5*(int)(cvRound(imgStddev.val[0])+1) << endl;

    return 5*(int)(cvRound(imgStddev.val[0])+1);

}

void extractGERegion(vector<GlobalEvent> &listGE, vector<Point> &areaAroundGE, int areaSize, int imgW, int imgH){

    vector<GlobalEvent>::iterator itGE;
    src::severity_logger< LogSeverityLevel > log;

    // Create ROI around existing GE
    for (itGE = listGE.begin(); itGE!=listGE.end(); ++itGE){

        Point pos = (*itGE).LEList.back().getMassCenter();
        BOOST_LOG_SEV(log,notification) << " GE position : " << pos;

        int x = pos.x - areaSize/2, y = pos.y - areaSize/2;
        int w = areaSize, h = areaSize;

        //BOOST_LOG_SEV(log,notification) << " x : " << x << " y : " << y;
        //BOOST_LOG_SEV(log,notification) << " w : " << w << " h : " << h;

        if(x < 0){

            x = 0;
            w = pos.x + areaSize/2;

        }

        if(y < 0){

            y = 0;
            h = pos.y + areaSize/2;

        }

        if((pos.x + areaSize/2 ) > imgW){

            w = imgW - pos.x + areaSize/2;

        }

        if((pos.y + areaSize/2) > imgH){

            h = imgH - pos.y + areaSize/2;

        }

       // BOOST_LOG_SEV(log,notification) << " x : " << x << " y : " << y;
        //BOOST_LOG_SEV(log,notification) << " w : " << w << " h : " << h;

        areaAroundGE.push_back(Point(x,y));
        areaAroundGE.push_back(Point(w,h));

    }

}



void colorInBlack(int const *roiSize, int j, int i, int areaPosX, int areaPosY, Point areaPosition, Mat &area, Mat &frame){

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

void searchROI( Mat &area,                      // One frame's region
                Mat &frame,                     // Thresholded frame
                Mat &eventMap,
                vector<Scalar> listColors,
                vector<LocalEvent> &listLE,     // List of local events
                int imgH,                       // Frame height
                int imgW,                       // Frame width
                int const *roiSize,             // ROI size
                int areaPosX,                   // Size of region where to search ROI
                int areaPosY,                   // Size of region where to search ROI
                Point areaPosition,
                int maxNbLE){            // Position of the area in the frame (up top corner)

    src::severity_logger< LogSeverityLevel > log;

    int situation;

    unsigned char * ptr;

    for(int i = 0; i < area.rows; i++){

        ptr = area.ptr<unsigned char>(i);

        for(int j = 0; j < area.cols; j++){

            if((int)ptr[j] > 0){

                // Check if ROI is not out of range in the frame
                if((areaPosition.y + i - roiSize[1]/2 > 0) && (areaPosition.y + i + roiSize[1]/2 < imgH) && ( areaPosition.x + j-roiSize[0]/2 > 0) && (areaPosition.x + j + roiSize[0]/2 < imgW)){

                    // Get colors in eventMap at the current ROI location.
                    vector<Scalar> listColorInRoi = getColorInEventMap(eventMap, Point(areaPosition.x + j, areaPosition.y + i), roiSize);

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

                                    colorInBlack(roiSize, j, i, areaPosX, areaPosY, areaPosition, area, frame);

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

                                        colorInBlack(roiSize, j, i, areaPosX, areaPosY, areaPosition, area, frame);

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

                                            colorInBlack(roiSize, j, i, areaPosX, areaPosY, areaPosition, area, frame);

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

void DetByLists::buildListSubdivisionOriginPoints(vector<Point> &listSubdivPosition, int nbSubdivOnAxis, int imgH, int imgW){


    int subW = imgW/nbSubdivOnAxis;
    int subH = imgH/nbSubdivOnAxis;

    Point first = Point((nbSubdivOnAxis/2 - 1) * subW, (nbSubdivOnAxis/2)*subH);
    Point last = Point(imgW - subW, imgH - subH);

    listSubdivPosition.push_back(first);

    int x = first.x, y = first.y;
    int nbdep = 0,
        nbdepLimit = 1,
        dep = 1; // 1 up
                 // 2 right
                 // 3 down
                 // 4 left

    for(int i = 1; i < nbSubdivOnAxis * nbSubdivOnAxis; i++){

        if(dep == 1){

            y = y - subH;
            listSubdivPosition.push_back(Point(x,y));
            nbdep ++;
            if(nbdep == nbdepLimit){
                nbdep = 0;
                dep ++;
            }

        }else if(dep == 2){

            x = x + subW;
            listSubdivPosition.push_back(Point(x,y));
            nbdep ++;
            if(nbdep == nbdepLimit){
                nbdep = 0;
                nbdepLimit++;
                dep ++;
            }

        }else if(dep == 3){

            y = y + subH;
            listSubdivPosition.push_back(Point(x,y));
            nbdep ++;
            if(nbdep == nbdepLimit){
                nbdep = 0;
                dep ++;
            }

        }else if(dep == 4){

            x = x - subW;
            listSubdivPosition.push_back(Point(x,y));
            nbdep ++;
            if(nbdep == nbdepLimit){
                nbdep = 0;
                nbdepLimit++;
                dep = 1;
            }

        }

    }
}

bool DetByLists::detectionMethodByListManagement(   Frame currentFrame,
                                                    Frame previousFrame,
                                                    int const *roiSize,
                                                    vector<GlobalEvent> &listGlobalEvents,
                                                    vector<Scalar> &listColors,
                                                    Mat mask,
                                                    int timeMax,
                                                    int nbGE,
                                                    int timeAfter,
                                                    int pixelFormat,
                                                    Mat localMask,
                                                    bool debug,
                                                    vector<Point> regionsPos,
                                                    bool downsample,
                                                    Mat &prevthresh,
                                                    int &nbDet,
                                                    vector<GlobalEvent>::iterator &itGEToSave){

    src::severity_logger< LogSeverityLevel > log;

    bool rec = false;

    bool saveDiff       = false;
    bool saveThresh     = false;
    bool saveThresh2    = false;
    bool saveEventmap   = false;
    bool saveCurr       = false;
    bool saveRes        = false;

    // Height of the frame.
    int imgH = currentFrame.getImg().rows;

    // Width of the frame.
    int imgW = currentFrame.getImg().cols;

    Mat currImg, prevImg;

    Mat tempCurr, tempPrev;
    currentFrame.getImg().copyTo(tempCurr, mask);
    previousFrame.getImg().copyTo(tempPrev, mask);


    // List of localEvent objects.
    vector <LocalEvent> listLocalEvents;

    /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    /// %%%%%%%%%%%%%%%%%%%%%%%%%%% STEP 1 : FILETRING / THRESHOLDING %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    //cout << "### Starting step 1. ###" << endl;

    // Execution time.
    double tStep1 = (double)getTickCount();

    //cout << "> Downsample : " << downsample << endl;

    double tdownsample = (double)getTickCount();

    // According to DET_DOWNSAMPLE_ENABLED's parameter in configuration file.
    if(downsample){

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
        pyrDown( mask, mask, Size( mask.cols/2, mask.rows/2 ) );
        tdownmask = (((double)getTickCount() - tdownmask)/getTickFrequency())*1000;
        //cout << "> tdownmask Time : " << tdownmask << endl;

    }else{

        tempCurr.copyTo(currImg);
        tempPrev.copyTo(prevImg);

    }

    if(saveCurr)
        SaveImg::saveBMP(currentFrame.getImg(), "/home/fripon/debug/curr/curr_"+Conversion::intToString(currentFrame.getNumFrame()));

    tdownsample = (((double)getTickCount() - tdownsample)/getTickFrequency())*1000;
    //cout << "> Downsample Time : " << tdownsample << endl;

    //cout << "> Compute difference." << endl;

    double tdiff = (double)getTickCount();

    // Difference between current and previous frame.
    Mat diff;
    absdiff(currImg, prevImg, diff);

    if(pixelFormat == MONO_12){

        Conversion::convertTo8UC1(diff).copyTo(diff);
        Conversion::convertTo8UC1(localMask).copyTo(localMask);

    }

    tdiff = (((double)getTickCount() - tdiff)/getTickFrequency())*1000;
    cout << "> Difference Time : " << tdiff << endl;

    if(saveDiff)
        SaveImg::saveBMP(diff, "/home/fripon/debug/diff/diff_"+Conversion::intToString(currentFrame.getNumFrame()));

    //cout << "> Thresholding" << endl;

    double tthresh = (double)getTickCount();

    // Threshold map.
    Mat mapThreshold = Mat(imgH,imgW, CV_8UC1,Scalar(0));

    // Thresholding diff.
    threshold(diff, mapThreshold, defineThreshold(diff, mask), 255, THRESH_BINARY);

    tthresh = (((double)getTickCount() - tthresh)/getTickFrequency())*1000;
    cout << "> Threshold Time : " << tthresh << endl;

    if(saveThresh)
        SaveImg::saveBMP(mapThreshold, "/home/fripon/debug/thresh/thresh_"+Conversion::intToString(currentFrame.getNumFrame()));

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
            SaveImg::saveBMP(threshANDprev, "/home/fripon/debug/resET_"+Conversion::intToString(currentFrame.getNumFrame()));

    }

    mapThreshold.copyTo(prevthresh);

    tStep1 = (((double)getTickCount() - tStep1)/getTickFrequency())*1000;
    cout << "> Time 1) : " << tStep1 << endl << endl;

    if(saveThresh2)
        SaveImg::saveBMP(mapThreshold, "/home/fripon/debug/thresh2/thresh_"+Conversion::intToString(currentFrame.getNumFrame()));

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
        for(itR = regionsPos.begin(); itR != regionsPos.end(); ++itR){

             Mat subdivision = threshANDprev(Rect((*itR).x, (*itR).y, imgW/8, imgH/8));

             searchROI( subdivision, threshANDprev, eventMap, listColors, listLocalEvents, imgH, imgW, roiSize, imgW/8, imgH/8, (*itR), 20);

        }
    }

    cout << "> LE number : " << listLocalEvents.size() << endl;

    tStep2 = (((double)getTickCount() - tStep2)/getTickFrequency())*1000;
    cout << "> Time 2) : " << tStep2 << endl << endl;

    if(saveEventmap)
        SaveImg::saveBMP(eventMap, "/home/fripon/debug/evMap/event"+Conversion::intToString(currentFrame.getNumFrame()));

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

        (*itLE).setNumFrame(currentFrame.getNumFrame());

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

            GlobalEvent newGE(currentFrame.getDateString(), currentFrame.getNumFrame(), currImg.rows, currImg.cols);
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
            (*itGE).setNumLastFrame(currentFrame.getNumFrame());

        }

        (*itGE).setNewLEStatus(false);

    }

    tStep3 = (((double)getTickCount() - tStep3)/getTickFrequency())*1000;
    cout << "> Time 3) : " << tStep3 << endl;

    /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%% STEP 4 : MANAGE LIST GLOBAL EVENT %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    /// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    cout << "### Starting step 4. ###" <<endl;

    double  tStep4      = (double)getTickCount();

    cout << "> GE number before management : " << listGlobalEvents.size() << endl;

    itGE = listGlobalEvents.begin();

    while (itGE != listGlobalEvents.end()){

        // CASE 1 : Finished event.
        if((*itGE).getAgeLastElem() > 5){

            if( ((*itGE).LEList.size() >= 10 && (*itGE).getLinearStatus() && !(*itGE).getGeStatic()) ||
                ((*itGE).LEList.size() >= 10 && !(*itGE).getLinearStatus() && (*itGE).continuousGoodPos(10) && !(*itGE).getGeStatic())){

                nbDet++;
                itGEToSave = itGE;
                return true;

            }else{

                // Delete the event.
                itGE = listGlobalEvents.erase(itGE);

            }

        //CASE 2 : Not finished event.
        }else{

            if((*itGE).getAge() > 400){

                // Delete the event.
                itGE = listGlobalEvents.erase(itGE);

            }else if((currentFrame.getFrameRemaining()< 10 && currentFrame.getFrameRemaining() != 0)){      // No more frames soon (in video or frames)

                 if((*itGE).LEList.size() >= 10 && (*itGE).getLinearStatus() && !(*itGE).getGeStatic()){

                    nbDet++;
                    itGEToSave = itGE;
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

    return false;

//    if(debug){
//
//        Mat VIDEO_finalFrame;
//        Mat VIDEO_originalFrame;
//        Mat VIDEO_diffFrame;
//        Mat VIDEO_threshMapFrame;
//        Mat VIDEO_eventMapFrame;
//        Mat VIDEO_eventFrame;
//        Mat VIDEO_geMapFrame;
//
//        /*if(f.getNumFrame() < 200){
//
//            int totalDigit = 4;
//
//            int cpt = 0;
//
//            int number = f.getNumFrame();
//
//            int nbZeroToAdd = 0;
//
//            string ch = "";
//
//            if(number<10){
//
//                nbZeroToAdd = totalDigit - 1;
//
//                for(int i = 0; i < nbZeroToAdd; i++){
//
//                    ch += "0";
//
//                }
//
//                SaveImg::saveBMP(currentFrame,"/home/fripon/data2/currentFrame/currentFrame" + ch + Conversion::intToString(f.getNumFrame()));
//                SaveImg::saveBMP(copyMapThreshold,"/home/fripon/data2/mapThreshold/mapThreshold_" + ch + Conversion::intToString(f.getNumFrame()));
//                SaveImg::saveBMP(eventMap,"/home/fripon/data2/eventMap/evMap_" + ch + Conversion::intToString(f.getNumFrame()));
//                SaveImg::saveBMP(geMap,"/home/fripon/data2/ge/geMap_" + Conversion::intToString(f.getNumFrame()));
//
//            }else{
//
//                while(number > 0){
//
//                    number/=10;
//                    cpt ++;
//
//                }
//
//                nbZeroToAdd = totalDigit - cpt;
//
//                for(int i = 0; i < nbZeroToAdd; i++){
//
//                    ch += "0";
//
//                }
//                SaveImg::saveBMP(currentFrame,"/home/fripon/data2/currentFrame/currentFrame" + ch + Conversion::intToString(f.getNumFrame()));
//                SaveImg::saveBMP(copyMapThreshold,"/home/fripon/data2/mapThreshold/mapThreshold_" + ch + Conversion::intToString(f.getNumFrame()));
//                SaveImg::saveBMP(eventMap,"/home/fripon/data2/eventMap/evMap_" + ch + Conversion::intToString(f.getNumFrame()));
//                SaveImg::saveBMP(geMap,"/home/fripon/data2/ge/geMap_" + Conversion::intToString(f.getNumFrame()));
//
//            }
//        }*/
//
//        VIDEO_finalFrame        = Mat(960,1280, CV_8UC3,Scalar(255,255,255));
//
//        VIDEO_originalFrame     = Mat(470,630, CV_8UC3,Scalar(0,0,0));
//        VIDEO_threshMapFrame    = Mat(470,630, CV_8UC3,Scalar(0,0,0));
//        VIDEO_eventMapFrame     = Mat(470,630, CV_8UC3,Scalar(0,0,0));
//        VIDEO_eventFrame        = Mat(470,630, CV_8UC3,Scalar(0,0,0));
//
//        cvtColor(copyCurrWithoutMask, copyCurrWithoutMask, CV_GRAY2BGR);
//        resize(copyCurrWithoutMask, VIDEO_originalFrame, Size(630,470), 0, 0, INTER_LINEAR );
//        cvtColor(copyMapThreshold, copyMapThreshold, CV_GRAY2BGR);
//        resize(copyMapThreshold, VIDEO_threshMapFrame, Size(630,470), 0, 0, INTER_LINEAR );
//        resize(eventMap, VIDEO_eventMapFrame, Size(630,470), 0, 0, INTER_LINEAR );
//
//        cvtColor(geMap, geMap, CV_GRAY2BGR);
//
//        resize(geMap, VIDEO_eventFrame, Size(630,470), 0, 0, INTER_LINEAR );
//
//        copyMakeBorder(VIDEO_originalFrame, VIDEO_originalFrame, 5, 5, 5, 5, BORDER_CONSTANT, Scalar(255,255,255) );
//        copyMakeBorder(VIDEO_threshMapFrame, VIDEO_threshMapFrame, 5, 5, 5, 5, BORDER_CONSTANT, Scalar(255,255,255) );
//        copyMakeBorder(VIDEO_eventMapFrame, VIDEO_eventMapFrame, 5, 5, 5, 5, BORDER_CONSTANT, Scalar(255,255,255) );cout << "here"<< endl;
//        copyMakeBorder(VIDEO_eventFrame, VIDEO_eventFrame, 5, 5, 5, 5, BORDER_CONSTANT, Scalar(255,255,255) );
//
//        putText(VIDEO_originalFrame, "Original", cvPoint(300,450),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,0,255), 1, CV_AA);
//        putText(VIDEO_threshMapFrame, "Filtering", cvPoint(300,450),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,0,255), 1, CV_AA);
//        putText(VIDEO_eventMapFrame, "Local Event Map", cvPoint(300,450),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,0,255), 1, CV_AA);
//        putText(VIDEO_eventFrame, "Global Event Map", cvPoint(300,450),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,0,255), 1, CV_AA);
//
//        VIDEO_originalFrame.copyTo(VIDEO_finalFrame(Rect(0, 0, 640, 480)));
//        VIDEO_threshMapFrame.copyTo(VIDEO_finalFrame(Rect(640, 0, 640, 480)));
//        VIDEO_eventMapFrame.copyTo(VIDEO_finalFrame(Rect(0, 480, 640, 480)));
//        VIDEO_eventFrame.copyTo(VIDEO_finalFrame(Rect(640, 480, 640, 480)));
//
//        string printFrameNum = Conversion::intToString(f.getNumFrame());
//        const char * c_printFrameNum;
//        c_printFrameNum = printFrameNum.c_str();
//        putText(VIDEO_finalFrame, c_printFrameNum, cvPoint(30,50),FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(0,255,0), 2, CV_AA);
//
//        if(videoDebug.isOpened()){
//
//            videoDebug<<VIDEO_finalFrame;
//
//        }
//
//    }

    //return rec;

}
