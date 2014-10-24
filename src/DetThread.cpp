/*
								DetThread.cpp

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
 * @file    DetThread.cpp
 * @author  Yoan Audureau
 * @version 1.0
 * @date    03/06/2014
 */

#include "DetThread.h"

DetThread::DetThread(Mat                        maskImg,
                     int                        mth,
                     int                        acqFormatPix,
                     Fifo<Frame>                *queue,
                     boost::mutex               *m_mutex_queue,
                     boost::condition_variable  *m_cond_queue_fill,
                     boost::condition_variable  *m_cond_queue_new_element,
                     boost::condition_variable  *newElem_listEventToRecord,
                     boost::mutex               *mutex_listEventToRecord,
                     vector<RecEvent>           *listEventToRecord,
                     int                        geAfterTime,
                     int                        geMax,
                     int                        geMaxTime,
                     string                     recPath,
                     string                     stationName,
                     bool                       detDebug,
                     string                     debugPath,
                     bool                       detMaskMoon,
                     bool                       saveMaskedMoon,
                     bool                       detDownsample,
                     Fits fitsHead){


    fitsHeader  = fitsHead;
    downsample                      =   detDownsample;
    maskMoonSave                    =   saveMaskedMoon;
    maskMoon                        =   detMaskMoon;
    debug                           =   detDebug;
    debugLocation                   =   debugPath;
    recordingPath                   =   recPath;
    station                         =   stationName;

	m_thread					        =	NULL;
	mustStop				        =	false;
	detMeth					        =	mth;
	framesQueue				        =	queue;

	imgFormat                       =   acqFormatPix;

    maskImg.copyTo(mask);
	mutexQueue				        =	m_mutex_queue;
	condQueueFill			        =	m_cond_queue_fill;
	condQueueNewElement		        =	m_cond_queue_new_element;
	condNewElemOn_ListEventToRec    =   newElem_listEventToRecord;
	mutex_listEvToRec               =   mutex_listEventToRecord;
	listEvToRec                     =   listEventToRecord;

    geMaxDuration                   =   geMaxTime;
    geMaxInstance                   =   geMax;
    geAfterDuration                 =   geAfterTime;
    //threadStopped                   =   false;

	nbDet                           =   0;

}

DetThread::~DetThread(void){

	if (m_thread!=NULL)

        delete m_thread;

}

void DetThread::join(){

	m_thread->join();

}

void DetThread::startDetectionThread(){

	m_thread = new boost::thread(boost::ref(*this));

}

void DetThread::stopDetectionThread(){

	// Signal the thread to stop (thread-safe)
	BOOST_LOG_SEV(log, logenum::notification) << "mustStop: " << mustStop;
	mustStopMutex.lock();
	mustStop=true;
	mustStopMutex.unlock();
	BOOST_LOG_SEV(log, logenum::notification) << "mustStop: " << mustStop;
    BOOST_LOG_SEV(log, logenum::notification) << "Wait join";
	// Wait for the thread to finish.
    if (m_thread!=NULL) m_thread->join();

    BOOST_LOG_SEV(log, logenum::notification) << "Thread stopped";

}

void DetThread::operator ()(){

	bool stop = false;

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "detThread");
	BOOST_LOG_SEV(log,logenum::notification) << "\n Detection thread started.";

	bool detFlag = false;

	bool computeRegion = false;
	vector<Point> listSubdivPosition;

	vector<Mat> moonList;
	vector<Point> listROI;
	Mat tempROI;
	Mat lastMoonCap;
	Point moonPos = Point(0,0);



	int roiSize[2] = {10, 10};

	int cptNoMoon = 0;

    //%%%%%%%%%%% STORE THE POSITION OF N LAST DETECTED EVENTS  %%%%%%%%%%%

	vector<Point> lastDetectionPosition;

    //%%%%%%%%%%%%%%%%%%%%%%%%%% CREATE MASK %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    // Mask of 3x3                          | | | |
    // Black pixel in the middle            | |X| |
    //                                      | | | |

	Mat maskNeighborhood;

	if(imgFormat == 8){

        Mat maskTemp(3,3,CV_8UC1,Scalar(255));
        maskTemp.at<uchar>(1, 1) = 0;
        maskTemp.copyTo(maskNeighborhood);

    }else if(imgFormat == 12){

        Mat maskTemp(3,3,CV_16UC1,Scalar(4095));
        maskTemp.at<ushort>(1, 1) = 0;
        maskTemp.copyTo(maskNeighborhood);

    }

    //%%%%%%%%%%%%%%%%% USED TO DEBUG DETECTION ON VIDEOS %%%%%%%%%%%%%%%%%

    VideoWriter videoDebug;

    if(debug){

        Size frameSize(static_cast<int>(1280), static_cast<int>(960));

        videoDebug = VideoWriter(debugLocation + "debug.avi", CV_FOURCC('M  ', 'J', 'P', 'G'), 5, frameSize, true); //initialize the VideoWriter object

    }

    //%%%%%%%%%%%%%%% START MAIN LOOP OF DETECTION THREAD %%%%%%%%%%%%%%%%%

	do{

        try{

            //%%%%%%%%%%%%%%%%%%%%%% GET NEW FRAME %%%%%%%%%%%%%%%%%%%%%%%%

            BOOST_LOG_SEV(log, logenum::notification) << "\n \n";

            boost::mutex::scoped_lock lock(*mutexQueue);
            BOOST_LOG_SEV(log,logenum::notification) << "Wait framesQueue full";
            while (!framesQueue->getFifoIsFull()) condQueueFill->wait(lock);
            BOOST_LOG_SEV(log,logenum::notification) << "FramesQueue full";
            BOOST_LOG_SEV(log,logenum::notification) << "Wait new frame";
            while (!framesQueue->getThreadReadStatus("det")) condQueueNewElement->wait(lock);
            BOOST_LOG_SEV(log,logenum::notification) << "New frame received";
            framesQueue->setThreadRead("det",false);

            double t = (double)getTickCount();

            Mat currentFrame, previousFrame;

            BOOST_LOG_SEV(log,logenum::notification) << "Get current frame from framesQueue";
            Frame f = framesQueue->getFifoElementAt(0);
            f.getImg().copyTo(currentFrame);


            BOOST_LOG_SEV(log,logenum::notification) << "Get previous frame from framesQueue";
            framesQueue->getFifoElementAt(1).getImg().copyTo(previousFrame);


            BOOST_LOG_SEV(log,logenum::notification) << "Get current frame's date";

            vector<string> date = framesQueue->getFifoElementAt(0).getDateString();

            BOOST_LOG_SEV(log,logenum::notification) << "****************************************************";
            BOOST_LOG_SEV(log,logenum::notification) << "                   Frame " << f.getNumFrame();
            BOOST_LOG_SEV(log,logenum::notification) << "****************************************************";


            //%%%%%%%%%%%%%%%% MOON %%%%%%%%%%%%%%%%%%%%%%%%%

            // Segmentation toutes les 30 secondes pour localiser la position de la lune
            if(f.getNumFrame()%400 == 0 && maskMoon){

                vector <LocalEvent> listLEMoon;

                Mat moon_res, temp;
                currentFrame.copyTo(temp,mask);
                threshold(temp, moon_res, 250, 255, THRESH_BINARY);
                //SaveImg::saveBMP(moon_res,"/home/fripon/data2/moon/moon_pos" + Conversion::intToString(f.getNumFrame()));

                if(!lastMoonCap.data){

                    moon_res.copyTo(lastMoonCap);

                }else{

                    Mat res = lastMoonCap & moon_res;

                    //SaveImg::saveBMP(res,"/home/fripon/data2/moon/moon_res" + Conversion::intToString(f.getNumFrame()));

                    if(countNonZero(res)>0){

                        cout << "non zero" <<endl;

                        unsigned char * ptr;

                        //height
                        for(int i = 0; i < res.rows; i++){

                            ptr = res.ptr<unsigned char>(i);

                            //width
                            for(int j = 0; j < res.cols; j++){

                                 if((int)ptr[j] > 0){

                                    if((i - 5 > 0) && ( i + 5 < res.rows) && (  j- 5 > 0) && (j + 5 < res.cols)){

                                        vector<LocalEvent>::iterator it;

                                        bool added = false;

                                        Mat temp(res.rows,res.cols,CV_8UC1,Scalar(0));
                                        Mat roiTemp(10,10,CV_8UC1,Scalar(255));
                                        roiTemp.copyTo(temp(Rect( j - 5, i - 5, 10, 10)));

                                        if(listLEMoon.size()>0){

                                            for (it=listLEMoon.begin(); it!=listLEMoon.end(); ++it){

                                                Mat resTest = (*it).getMap() & temp;

                                                if( countNonZero(resTest)>0){

                                                    Mat tempMat = (*it).getMap();
                                                    roiTemp.copyTo(tempMat(Rect( j - 5, i - 5, 10, 10)));
                                                    (*it).setMap(tempMat);
                                                    (*it).listRoiCenter.push_back(Point(j,i));
                                                    (*it).computeCenterOfMass(true);
                                                    Mat roi(roiSize[1],roiSize[0],CV_8UC1,Scalar(0));
                                                    roi.copyTo(res(Rect( j-roiSize[0]/2, i-roiSize[1]/2,roiSize[0],roiSize[1])));
                                                    break;

                                                }else{

                                                    vector<PixelEvent> listPixInRoi;
                                                    LocalEvent newLocalEvent(Scalar(0,0,0), Point(j, i), listPixInRoi, res.rows, res.cols, roiSize);
                                                    newLocalEvent.computeCenterOfMass(true);
                                                    listLEMoon.push_back(newLocalEvent);
                                                    Mat roi(roiSize[1],roiSize[0],CV_8UC1,Scalar(0));
                                                    roi.copyTo(res(Rect( j-roiSize[0]/2, i-roiSize[1]/2,roiSize[0],roiSize[1])));
                                                    break;
                                                }
                                            }

                                        }else{

                                            vector<PixelEvent> listPixInRoi;
                                            LocalEvent newLocalEvent(Scalar(0,0,0), Point(j, i), listPixInRoi, res.rows, res.cols, roiSize);
                                            newLocalEvent.computeCenterOfMass(true);
                                            listLEMoon.push_back(newLocalEvent);
                                            Mat roi(roiSize[1],roiSize[0],CV_8UC1,Scalar(0));
                                            roi.copyTo(res(Rect( j-roiSize[0]/2, i-roiSize[1]/2,roiSize[0],roiSize[1])));

                                        }
                                    }
                                }
                            }
                        }

                    }

                    if(listLEMoon.size()>0){

                        vector<LocalEvent>::iterator it;
                        vector<LocalEvent>::iterator tempIt;
                        bool first = true;

                        for (it=listLEMoon.begin(); it!=listLEMoon.end(); ++it){

                            if(first){

                                tempIt = it;
                                first = false;

                            }else{

                                if((*tempIt).listRoiCenter.size() < (*it).listRoiCenter.size())
                                    tempIt = it;

                            }

                        }


                        if(sqrt(pow(((*tempIt).centerOfMass.x - moonPos.x),2) + pow(((*tempIt).centerOfMass.y - moonPos.y),2)) < 100 ){

                            moonPos = (*tempIt).centerOfMass;

                        }

                        cout << "MOON POSITION = " << moonPos << endl;

                        if(maskMoonSave)
                            SaveImg::saveBMP(currentFrame, debugLocation + "moon_original_" + Conversion::intToString(f.getNumFrame()));

                        circle(currentFrame, moonPos, 60, Scalar(0), CV_FILLED, 8, 0);

                        if(maskMoonSave)
                            SaveImg::saveBMP(currentFrame, debugLocation + "moon_final_" + Conversion::intToString(f.getNumFrame()));

                        cptNoMoon = 0;

                    }else{

                        cptNoMoon ++;

                    }

                    if(cptNoMoon >= 5 )
                        moonPos = Point(0,0);

                    moon_res.copyTo(lastMoonCap);


                }
            }

            //%%%%%%%%%%%%%%%% COMPUTE REGION %%%%%%%%%%%%%%%
            if(!computeRegion){

                DetByLists::buildListSubdivisionOriginPoints(listSubdivPosition, 8, currentFrame.rows, currentFrame.cols);
                computeRegion = true;

            }

            //%%%%%%%%%%%%%%%% IF NO MASK, CREATE WHITE ONE %%%%%%%%%%%%%%%

            if(!mask.data){

                Mat tempMat(currentFrame.rows,currentFrame.cols,CV_8UC1,Scalar(255));
                tempMat.copyTo(mask);

            }

            //%%%%%%%%%%%%%%%%%%% MEAN N PREVIOUS FRAMES %%%%%%%%%%%%%%%%%%

            /*double tMean = (double)getTickCount();

            Mat meanFrames;
            Mat res = Mat::zeros(currentFrame.rows,currentFrame.cols,CV_32FC1);
            Mat img = Mat::zeros(currentFrame.rows,currentFrame.cols,CV_32FC1);

            int nbImgToMean = 3;

            if(imgFormat == 8){

                if(framesQueue->getSizeQueue() > nbImgToMean ){

                    for(int i = 1; i <= nbImgToMean ; i++){

                        framesQueue->getFifoElementAt(i).getImg().convertTo(img, CV_32FC1);
                        accumulate(img,res);

                    }

                    res = res/nbImgToMean;

                    meanFrames = Conversion::convertTo8UC1(res);

                }else{

                    previousFrame.copyTo(meanFrames);

                }

            }else if(imgFormat == 12){

                if(framesQueue->getSizeQueue() > nbImgToMean ){

                    for(int i = 1; i <= nbImgToMean ; i++){

                        framesQueue->getFifoElementAt(i).getImg().convertTo(img, CV_32FC1);
                        accumulate(img,res);

                    }

                    res = res/nbImgToMean;

                    res.convertTo(meanFrames, CV_16UC1);

                }else{

                    previousFrame.copyTo(meanFrames);

                }

            }else{

                throw "Image format unknown";

            }

            tMean = (((double)getTickCount() - tMean)/getTickFrequency())*1000;
            cout << "tMean: " << tMean << endl;
*/
            lock.unlock();


            //%%%%%%%%%%%%%%%%% CHOOSE A DETECTION METHOD %%%%%%%%%%%%%%%%%




          //   if(moonList.size() == 5){

                // Meteor detection by pixel tresholding and by lists management
                detFlag = DetByLists::detectionMethodByListManagement(  f,
                                                                        date,
                                                                        currentFrame,
                                                                        previousFrame,
                                                                        previousFrame,
                                                                        roiSize,
                                                                        listGlobalEvents,
                                                                        mask,
                                                                        *mutex_listEvToRec,
                                                                        *mutexQueue,
                                                                        *listEvToRec,
                                                                        *framesQueue,
                                                                        recordingPath,
                                                                        station,
                                                                        nbDet,
                                                                        geMaxDuration,
                                                                        geMaxInstance,
                                                                        geAfterDuration,
                                                                        imgFormat,
                                                                        lastDetectionPosition,
                                                                        maskNeighborhood,
                                                                        videoDebug,
                                                                        debug,
                                                                        listSubdivPosition,
                                                                        maskMoon,
                                                                        moonPos,
                                                                        downsample);

           // }

            if(detFlag){

                BOOST_LOG_SEV(log,logenum::notification) << "!!!!!!!!!! - DETECTION - !!!!!!!!!!";

                //Send notification to the record thread
                condNewElemOn_ListEventToRec->notify_one();
                detFlag = false;

            }

            t = (((double)getTickCount() - t)/getTickFrequency())*1000;
            cout    << " [-DETECTION-]    Time: "
                    << std::setprecision(3)
                    << std::fixed
                    << t
                    << " ms "
                    << endl;

            cout    << " DET STATS ---> "
                    << nbDet
                    << endl;

            BOOST_LOG_SEV(log,logenum::notification) << " [-DETECTION-]    Time: "
                                            << std::setprecision(3)
                                            << std::fixed
                                            << t
                                            << " ms  -->> DETECTION STATS -->> "
                                            << nbDet;

            // Get the "must stop" state (thread-safe)
            mustStopMutex.lock();
            stop = mustStop;
            mustStopMutex.unlock();

		}catch(const boost::thread_interrupted&){

            cout << "Detection thread INTERRUPTED" <<endl;
            break;

        }catch(const char * msg){

            cout << msg << endl;

        }

	}while (stop==false);

	//threadStopped = true;

	BOOST_LOG_SEV(log, logenum::notification) << "Exit detection thread loop ";

}
