/*
								RecThread.cpp

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
 * @file    RecThread.cpp
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    12/06/2014
 */

 #include "RecThread.h"

//! Constructor
/*!
    \param recPath location of the captured images
    \param sharedList pointer on the shared list which contains the events to record
    \param sharedListMutex pointer on a mutex used for the shared list
    \param sharedListMutex pointer on a condition used to notify when the shared list has a new element
*/
RecThread::RecThread(   string recpath,
                        vector<RecEvent> *sharedList,
                        boost::mutex *sharedListMutex,
                        boost::condition_variable *condition,
                        int  pixFormat,
                        bool avi,
                        bool fits3D,
                        bool fits2D,
                        bool sum,
                        bool positionFile,
                        bool bmp,
                        bool trail,
                        bool mapGE,
                        Fits fitsHead  ){

    fitsHeader              = fitsHead;
    pixelFormat             = pixFormat;
    recPath                 = recpath;
    listEventToRec          = sharedList;
    mutex_listEventToRec    = sharedListMutex;
    condNewElem             = condition;
    recAvi                  = avi;
    recFits3D               = fits3D;
    recFits2D               = fits2D;
	recPos                  = positionFile;
	recSum                  = sum;
	recBmp                  = bmp;
	recTrail                = trail;
	recMapGE                = mapGE;
    mustStop				        =	false;
	threadStopped           = false;

}

RecThread::~RecThread(void){

	if (recThread!=NULL)
		delete recThread;

		  BOOST_LOG_SEV(log,notification) << "rec thread destructor";

}

void RecThread::start(){

    recThread=new boost::thread(boost::ref(*this));

}

void RecThread::stop(){

	// Signal the thread to stop (thread-safe)
	mustStopMutex.lock();
	mustStop=true;
	mustStopMutex.unlock();

    while(recThread->timed_join(boost::posix_time::seconds(5)) == false){

        BOOST_LOG_SEV(log, notification) << "Thread not stopped, interrupt it now";
        recThread->interrupt();
        BOOST_LOG_SEV(log, notification) << "Interrupt Request sent";

    }

    BOOST_LOG_SEV(log, notification) << "Thread stopped";

}

// Thread function
void RecThread::operator () (){

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "recThread");
	BOOST_LOG_SEV(log, critical) << "\n";
	BOOST_LOG_SEV(log,notification) << "Recording Thread Started";

    bool stop = false;

    namespace fs = boost::filesystem;

    do{

        try{

            boost::mutex::scoped_lock lock(*mutex_listEventToRec);

            // When there is no data, wait till someone fills it.
            // Lock is automatically released in the wait and obtained again after the wait
            while (listEventToRec->size()==0) condNewElem->wait(lock);

            lock.unlock();

            BOOST_LOG_SEV(log,notification) << "New event sent to the recording thread";

            bool recStatus = true;

            do{

                BOOST_LOG_SEV(log,notification) << "Number of GE to save : " << listEventToRec->size();

                RecEvent r;
                vector<Mat> listFrames;
                vector<Mat> listPrevFrames;
                vector<Mat>::iterator it2;

                lock.lock();
                r.copyFromRecEvent(listEventToRec->back());
                listEventToRec->pop_back();
                lock.unlock();

                listFrames = r.getBuffer();
                listPrevFrames = r.getPrevBuffer();


                vector<string> eventDate = r.getDateEvent();

                string evDate =   eventDate.at(0)
                                + eventDate.at(1)
                                + eventDate.at(2) + "_"
                                + eventDate.at(3)
                                + eventDate.at(4)
                                + eventDate.at(5);

                /// %%%%%%%%%%%%%%%% SAVE POSITIONS FILE %%%%%%%%%%%%%%%% ///

                if(recPos){

                    BOOST_LOG_SEV(log,notification) << "Build meteor's positions file";

                    //Get the list of meteor positions
                    BOOST_LOG_SEV(log,notification) << "Get the list of meteor positions";
                    vector<Point> posEv = r.getListMetPos();

                    //Get the list of positions in buffer
                    BOOST_LOG_SEV(log,notification) << "Get the list of positions in buffer";
                    vector <int> posEvBuff = r.getPositionInBuffer();

                    ofstream posFile;
                    string posFilePath = r.getPath() + "eventPositions.txt";
                    BOOST_LOG_SEV(log,notification) << "File location : " << posFilePath;

                    posFile.open(posFilePath.c_str());
                    posFile << "num_frame (x;y)\n";

                    for(int i =0; i<posEv.size(); i++){

                        posFile << posEvBuff.at(posEv.size()- 1 - i) << "         (" << posEv.at(i).x << ";" << posEv.at(i).y<<")\n";

                    }

                    BOOST_LOG_SEV(log,notification) << "Close positions file";

                    posFile.close();

                }

                /// %%%%%%%%%%%%%%%% SAVE AVI AND/OR BMP %%%%%%%%%%%%%%%% ///

                if(recAvi){

                    BOOST_LOG_SEV(log,notification) << "START to record AVI ...";

                    int h = listFrames.at(0).rows;
                    int w = listFrames.at(0).cols;

                    Size frameSize(static_cast<int>(w), static_cast<int>(h));

                    VideoWriter oVideoWriter(r.getPath() + "video_" +".avi", CV_FOURCC('D', 'I', 'V', 'X'), 30, frameSize, false); //initialize the VideoWriter object

                    Conversion c;

                    int n = 0;

                    for (it2 = listFrames.begin(); it2 != listFrames.end(); ++it2){

                        Mat temp1;
                        (*it2).copyTo(temp1);

                        if(pixelFormat != 8)
                            temp1.convertTo(temp1, CV_8UC1, 255, 0);

                        if(oVideoWriter.isOpened()){

                            oVideoWriter << temp1;

                        }

                        if(recBmp){

                            string bmpName = "frame_";

                            imwrite(r.getPath() + bmpName + Conversion::intToString(n) + ".bmp", temp1);

                        }

                        n++;
                    }

                    BOOST_LOG_SEV(log,notification) << "END to record AVI ...";

                }else{

                    if(recBmp){

                        int n = 0;

                        for (it2=listFrames.begin(); it2!=listFrames.end(); ++it2){

                            Mat temp1;
                            (*it2).copyTo(temp1);

                            if(pixelFormat != 8)
                                temp1.convertTo(temp1, CV_8UC1, 255, 0);

                            string bmpName = "frame_";

                            imwrite(r.getPath() + bmpName + Conversion::intToString(n) + ".bmp", temp1);

                            n++;
                        }
                    }
                }

                /// %%%%%%%%%%%%%%%% SAVE Fits2D %%%%%%%%%%%%%%%% ///

                if(recFits2D){

                    BOOST_LOG_SEV(log,notification) << "START to save fits2D ...";

                    string fits2DPath = r.getPath() + "fits2D/";

                    if(ManageFiles::createDirectory(fits2DPath)){

                        int num = 0;

                        int digits = 0;
                        int maxx = listPrevFrames.size() + listFrames.size();
                        cout << "size : " << maxx << endl;
                        while (maxx) {
                            maxx/= 10;
                            digits++;
                        }

                        for (it2=listPrevFrames.begin(); it2!=listPrevFrames.end(); ++it2){

                            if(pixelFormat == 8){

                                Fits2D newFits(fits2DPath + "f_"+ Conversion::numbering(digits, num) + Conversion::intToString(num) + "_",fitsHeader);
                                newFits.writeFits((*it2), UC8, 0, true );

                            }else{

                                Fits2D newFits(fits2DPath + "f_"+ Conversion::numbering(digits, num) + Conversion::intToString(num) + "_",fitsHeader);
                                newFits.writeFits((*it2), US16, 0, true );
                            }

                            num++;
                        }


                        for (it2=listFrames.begin(); it2!=listFrames.end(); ++it2){

                            if(pixelFormat == 8){

                                Fits2D newFits(fits2DPath + "f_"+ Conversion::numbering(digits, num) + Conversion::intToString(num) + "_",fitsHeader);
                                newFits.writeFits((*it2), UC8, 0, true );

                            }else{

                                Fits2D newFits(fits2DPath + "f_"+ Conversion::numbering(digits, num) + Conversion::intToString(num) + "_",fitsHeader);
                                newFits.writeFits((*it2), US16, 0, true );
                            }

                            num++;
                        }

                    }else{

                        BOOST_LOG_SEV(log,notification) << "Can't create fits2D directory in " << r.getPath();

                    }

                    BOOST_LOG_SEV(log,notification) << "END to save fits2D ...";

                }

                /// %%%%%%%%%%%%%%%% SAVE Fits3D %%%%%%%%%%%%%%%% ///

                if(recFits3D){

                    BOOST_LOG_SEV(log,notification) << "START to save fits3D ...";

                    Fits3D fitsFile(&listFrames);

                    if(pixelFormat == 8){

                        fitsFile.writeFits3d8uc(r.getPath() + "fits3D_" + ".fits");

                    }else{

                        fitsFile.writeFits3d16us(r.getPath() + "fits3D_" + ".fits");

                    }

                    BOOST_LOG_SEV(log,notification) << "END to save fits3D ...";
                }

                /// %%%%%%%%%%%%%%%% Save GEMap %%%%%%%%%%%%%%%%///

                if(recMapGE){

                    BOOST_LOG_SEV(log,notification) << "START to save GEMap ...";

                    Mat temp;

                    r.getMapEvent().copyTo(temp);

                    SaveImg::saveBMP(temp, r.getPath() + "GEMap_" + evDate);

                    BOOST_LOG_SEV(log,notification) << "END to save fits3D ...";

                }

                /// %%%%%%%%%%%%%%%% Save the sum of the event's frames %%%%%%%%%%%%%%%% ///

                if(recSum){

                    BOOST_LOG_SEV(log,notification) << "START recSum ...";

                    Mat resImg = Mat::zeros(listFrames.at(0).rows,listFrames.at(0).cols, CV_32FC1);
                    Mat img = Mat::zeros(listFrames.at(0).rows,listFrames.at(0).cols,CV_32FC1);

                    for (it2=listFrames.begin(); it2!=listFrames.end(); ++it2){

                        accumulate((*it2),resImg);

                    }

                    Fits2D newFits(r.getPath() + "sum",fitsHeader);
                    newFits.writeFits(resImg, F32, 0, true );

                    BOOST_LOG_SEV(log,notification) << "END recSum ...";

                }

                imwrite(r.getPath() + "dirMap.bmp", r.getDirMap());

                lock.lock();

                if(listEventToRec->size() != 0)
                    recStatus = true;
                else
                    recStatus = false;

                lock.unlock();

            }while(recStatus);

            //Get the "must stop" state (thread-safe)
            mustStopMutex.lock();
            stop = mustStop;
            mustStopMutex.unlock();

        }catch(const boost::thread_interrupted&){

            cout<< "Record thread INTERRUPTED" <<endl;
            break;

        }

    }while(!stop);

    threadStopped = true;

	BOOST_LOG_SEV(log,notification) << "Record thread terminated";

}

