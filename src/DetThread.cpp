/*
								DetThread.cpp

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
 * @file    DetThread.cpp
 * @author  Yoan Audureau
 * @version 1.0
 * @date    03/06/2014
 */

#include "DetThread.h"

DetThread::DetThread(Mat                            maskImg,
                     int                            mth,
                     CamBitDepth                    acqFormatPix,
                     DetMeth                        detMthd,
                     int                            geAfterTime,
                     int                            bufferSize,
                     int                            geMax,
                     int                            geMaxTime,
                     string                         recPath,
                     string                         stationName,
                     bool                           detDebug,
                     string                         debugPath,
                     bool                           detDownsample,
                     Fits                           fitsHead,
                     boost::circular_buffer<Frame>  *cb,
                     boost::mutex                   *m_cb,
                     boost::condition_variable      *c_newElemCb,
                     bool                           *newFrameForDet,
                     boost::mutex                   *m_newFrameForDet,
                     boost::condition_variable      *c_newFrameForDet,
                     RecEvent                       *recEvent
                     ){

    mthd = detMthd;

    newFrameDet = newFrameForDet;
    m_newFrameDet = m_newFrameForDet;
    c_newFrameDet = c_newFrameForDet;

    fitsHeader  = fitsHead;
    downsample                      =   detDownsample;
    debug                           =   detDebug;
    debugLocation                   =   debugPath;
    recordingPath                   =   recPath;
    station                         =   stationName;

	m_thread					        =	NULL;
	mustStop				        =	false;
	detMeth					        =	mth;

	imgFormat                       =   acqFormatPix;

    maskImg.copyTo(mask);

	frameBuffer = cb;
    m_frameBuffer = m_cb;
    c_newElemFrameBuffer = c_newElemCb;

    timeMax                         =   geMaxTime;
    nbGE                            =   geMax;
    timeAfter                       =   geAfterTime;

	nbDet                           =   0;
	eventToRec = recEvent;
	frameBufferMaxSize      = bufferSize;

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
	BOOST_LOG_SEV(log, notification) << "mustStop: " << mustStop;
	mustStopMutex.lock();
	mustStop=true;
	mustStopMutex.unlock();
	BOOST_LOG_SEV(log, notification) << "mustStop: " << mustStop;
    BOOST_LOG_SEV(log, notification) << "Wait join";
	// Wait for the thread to finish.
    if (m_thread!=NULL) m_thread->join();

    BOOST_LOG_SEV(log, notification) << "Thread stopped";

}

void DetThread::operator ()(){

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "detThread");
	BOOST_LOG_SEV(log,notification) << "\n";
    BOOST_LOG_SEV(log,notification) << "==============================================";
	BOOST_LOG_SEV(log,notification) << "=========== Start detection thread ===========";
	BOOST_LOG_SEV(log,notification) << "==============================================";

	bool detectionStatus    = false;
    bool stopThread         = false;
	bool computeRegion      = false;


	vector<Point> regionsPos;

	int roiSize[2] = {10, 10};

	vector<Point> lastDetectionPosition;

	vector<Scalar> listColors; // B, G, R
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

    /// Create local mask to eliminate single white pixels.

	Mat localMask;

	if(imgFormat == MONO_8){

        Mat maskTemp(3,3,CV_8UC1,Scalar(255));
        maskTemp.at<uchar>(1, 1) = 0;
        maskTemp.copyTo(localMask);

    }else if(imgFormat == MONO_12){

        Mat maskTemp(3,3,CV_16UC1,Scalar(4095));
        maskTemp.at<ushort>(1, 1) = 0;
        maskTemp.copyTo(localMask);

    }

    /// Create a video to debug.

    /*VideoWriter videoDebug;

    if(debug){

        Size frameSize(static_cast<int>(1280), static_cast<int>(960));
        videoDebug = VideoWriter(debugLocation + "debug.avi", CV_FOURCC('M  ', 'J', 'P', 'G'), 5, frameSize, true); //initialize the VideoWriter object

    }*/

    vector<GlobalEvent>::iterator itGEToSave;
    bool breakAnalyse = false;
    int downtime = 0;

    /// Thread loop.

	do{

        try{



            /// Wait new frame.

            boost::mutex::scoped_lock lock(*m_newFrameDet);
            while (!(*newFrameDet)) c_newFrameDet->wait(lock);
            *newFrameDet = false;
            lock.unlock();

            /// Get last frames.

            Frame currentFrame, previousFrame;

            // Get access on frame buffer.
            boost::mutex::scoped_lock lock2(*m_frameBuffer);

            // Uptime
            double t = (double)getTickCount();

            // At least two frames are in the frame buffer.
            if(frameBuffer->size() > 2){

                currentFrame    = frameBuffer->back();
                previousFrame   = frameBuffer->at(frameBuffer->size()-2);

            }

            //Release frame buffer.
            lock2.unlock();

            if(currentFrame.getImg().data && previousFrame.getImg().data){

                /// Compute regions.

                if(!computeRegion){

                    if(downsample)
                        DetByLists::buildListSubdivisionOriginPoints(regionsPos, 8, currentFrame.getImg().rows/2, currentFrame.getImg().cols/2);
                    else
                        DetByLists::buildListSubdivisionOriginPoints(regionsPos, 8, currentFrame.getImg().rows, currentFrame.getImg().cols);

                    computeRegion = true;

                }

                /// Mask.

                if(!mask.data){

                    Mat tempMat(currentFrame.getImg().rows,currentFrame.getImg().cols,CV_8UC1,Scalar(255));
                    tempMat.copyTo(mask);

                }

                /// Meteor detection.

                switch(mthd){

                    case TEMPORAL_MTHD :

                        {

                            if(!breakAnalyse)

                            detectionStatus = DetByLists::detectionMethodByListManagement(  currentFrame,           // Last grabbed frame
                                                                                            previousFrame,          // Previous grabbed frame
                                                                                            roiSize,                // Size of a region of interest
                                                                                            listGlobalEvents,       // Global events to analyze
                                                                                            listColors,
                                                                                            mask,                   // Frame Mask
                                                                                            timeMax,                // Maximum duration for an event
                                                                                            nbGE,                   // Maximum of allowed global event
                                                                                            timeAfter,              // Maximum time after an event
                                                                                            imgFormat,
                                                                                            localMask,
                                                                                            debug,
                                                                                            regionsPos,
                                                                                            downsample,
                                                                                            prevthresh,
                                                                                            nbDet,
                                                                                            itGEToSave);

                        }

                        break;

                    case HOUGH_MTHD :

                        {

                        }

                        break;

                }

                /// Save datas if a "meteor" has been detected.

                if(detectionStatus){

                    breakAnalyse = true;

                    (*itGEToSave).setAgeLastElem((*itGEToSave).getAgeLastElem() + 1);
                    (*itGEToSave).setAge((*itGEToSave).getAge() + 1);

                    if((*itGEToSave).getAgeLastElem() > timeAfter ||
                       (currentFrame.getFrameRemaining() < 10 && currentFrame.getFrameRemaining()!= 0)){

                        eventToRec->buildEventLocation((*itGEToSave).getDate());

                        eventToRec->saveGE(listGlobalEvents, itGEToSave);

                        listGlobalEvents.clear();

                        detectionStatus = false;
                        breakAnalyse = false;

                    }
                }

                /// Infos.

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

            }

            mustStopMutex.lock();
            stopThread = mustStop;
            mustStopMutex.unlock();

		}catch(const boost::thread_interrupted&){

            cout << "Detection thread INTERRUPTED" <<endl;
            break;

        }catch(const char * msg){

            cout << msg << endl;

        }

	}while(stopThread == false);

	BOOST_LOG_SEV(log, notification) << "Exit detection thread loop ";

}
