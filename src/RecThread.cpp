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
                        bool positionFile,
                        bool bmp,
                        bool trail,
                        bool shape,
                        bool mapGE,
                        Fits fitsHead  ){

    fitsHeader  = fitsHead;
    pixelFormat             = pixFormat;
    recPath                 = recpath;
    listEventToRec          = sharedList;
    mutex_listEventToRec    = sharedListMutex;
    condNewElem             = condition;
    recAvi                  = avi;
    recFits3D               = fits3D;
    recFits2D               = fits2D;
	recPositionFile         = positionFile;
	recBmp                  = bmp;
	recTrail                = trail;
	recShape                = shape;
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

                lock.lock();

                BOOST_LOG_SEV(log,notification) << "Number of event to record : " << listEventToRec->size();

                RecEvent *r = new RecEvent();

                r->copyFromRecEvent(&listEventToRec->at(listEventToRec->size()-1));

                listEventToRec->pop_back();

                lock.unlock();

                vector<Mat> listFrames;



                listFrames = r->getBuffer();
                BOOST_LOG_SEV(log,notification) << "size : "<<r->getBuffer().size();
                BOOST_LOG_SEV(log,notification) << listFrames.size();

                  BOOST_LOG_SEV(log,notification) << r->getBuffer().at(0).rows;
                 BOOST_LOG_SEV(log,notification) << r->getBuffer().at(0).cols;

                     BOOST_LOG_SEV(log,notification) << listFrames.at(0).rows;
                 BOOST_LOG_SEV(log,notification) << listFrames.at(0).cols;



                /// POSITIONS FILE ///

                BOOST_LOG_SEV(log,notification) << "Build meteor's positions file";

                //Get the list of meteor positions
                BOOST_LOG_SEV(log,notification) << "Get the list of meteor positions";
                vector<Point> posEv = r->getListMetPos();

                //Get the list of positions in buffer
                BOOST_LOG_SEV(log,notification) << "Get the list of positions in buffer";
                vector <int> posEvBuff = r->getPositionInBuffer();

                ofstream posFile;
                string posFilePath = r->getPath() + "eventPositions.txt";
                BOOST_LOG_SEV(log,notification) << "File location : " << posFilePath;

                posFile.open(posFilePath.c_str());
                posFile << "num_frame (x;y)\n";

                for(int i =0; i<posEv.size(); i++){

                    posFile << posEvBuff.at(posEv.size()- 1 - i) << "         (" << posEv.at(i).x << ";" << posEv.at(i).y<<")\n";

                }

                BOOST_LOG_SEV(log,notification) << "Close positions file";

                posFile.close();

                vector<string> eventDate = r->getDateEvent();

                string evDate =   eventDate.at(0)
                                + eventDate.at(1)
                                + eventDate.at(2) + "_"
                                + eventDate.at(3)
                                + eventDate.at(4)
                                + eventDate.at(5);

                BOOST_LOG_SEV(log,notification) << "Load event date : " << evDate;

                vector<Mat>::iterator it2;

                /// AVI ///

                if(recAvi){

                    BOOST_LOG_SEV(log,notification) << "START to record AVI ...";

                    int h = listFrames.at(0).rows;
                    int w = listFrames.at(0).cols;

                    Size frameSize(static_cast<int>(w), static_cast<int>(h));

                    VideoWriter oVideoWriter(r->getPath() + "video_" +".avi", CV_FOURCC('D', 'I', 'V', 'X'), 30, frameSize, false); //initialize the VideoWriter object

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

                            imwrite(r->getPath() + bmpName + Conversion::intToString(n) + ".bmp", temp1);

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

                            imwrite(r->getPath() + bmpName + Conversion::intToString(n) + ".bmp", temp1);

                            n++;
                        }
                    }
                }

                /// Fits3D ///

                if(recFits3D){

                    BOOST_LOG_SEV(log,notification) << "START to record fits3D ...";
                    BOOST_LOG_SEV(log,notification) << listFrames.at(0).rows;
                    BOOST_LOG_SEV(log,notification) << listFrames.at(0).cols;


                    cout << "save fits3S" <<endl;


                    Fits3D fitsFile(listFrames.size(), listFrames.at(0).rows, listFrames.at(0).cols, &listFrames);

                    if(pixelFormat == 8){
                        BOOST_LOG_SEV(log,notification) << "fits3D in 8 bits";
                        fitsFile.writeFits3D_UC(r->getPath() + "fits3D_" + ".fits");
                    }else{
                        BOOST_LOG_SEV(log,notification) << "fits3D in 12 bits";
                        BOOST_LOG_SEV(log,notification) << "fits3D mat type : " << Conversion::matTypeToString(r->getBuffer().at(0).type());
                        fitsFile.writeFits3D_US(r->getPath() + "fits3D_" + ".fits");

                    BOOST_LOG_SEV(log,notification) << "END to record fits3D ...";
                    }
                }

                /// GEMap ///

                if(recMapGE){

                    BOOST_LOG_SEV(log,notification) << "START to record GEMap ...";

                    Mat temp;

                    r->getMapEvent().copyTo(temp);

                    /*string path = r->getPath() + evDate + "_";

                    string pathFinal;

                    int v = 0;

                    do{

                        pathFinal = path + Conversion::intToString(v);
                        v++;

                    }while(boost::filesystem::exists( pathFinal + ".bmp" ) );

                    SaveImg::saveBMP(temp, pathFinal);*/

                    SaveImg::saveBMP(temp, r->getPath() + "GEMap_" + evDate);

                    BOOST_LOG_SEV(log,notification) << "END to record fits3D ...";

                }

                /// Sum frames ///

                bool recSum = true;


                if(recSum){

                    BOOST_LOG_SEV(log,notification) << "START to record sum ...";

                    Mat resImg = Mat::zeros(listFrames.at(0).rows,listFrames.at(0).cols, CV_32FC1);
                    Mat img = Mat::zeros(listFrames.at(0).rows,listFrames.at(0).cols,CV_32FC1);
                     BOOST_LOG_SEV(log,notification) << "start sum...rows: " << listFrames.at(0).rows << " cols : "<< listFrames.at(0).cols;
                    for (it2=listFrames.begin(); it2!=listFrames.end(); ++it2){

                        //(*it2).convertTo(img, CV_32FC1);
                        accumulate((*it2),resImg);

                    }

                    BOOST_LOG_SEV(log,notification) << "start to save...";

                    Fits2D newFits(r->getPath() + "sum",fitsHeader);

                    newFits.writeimage(resImg, 32, "0", true );


                   /* Fits2D fit(r->getPath() + "sum", listFrames.size(), "", 0, 30, 255, 33333.0, 400, 0.0 );
                    fit.loadKeywordsFromConfigFile("/home/fripon/friponProject/friponCapture/configuration.cfg");
                    fit.writeimage(resImg, 32, "0" , true);*/

                    /*Mat final8UCsum;
                    resImg.convertTo(final8UCsum, CV_8UC1, 255, 0);
                    SaveImg::saveBMP(final8UCsum,r->getPath()+"sum");*/

                    BOOST_LOG_SEV(log,notification) << "END to record sum ...";

                }



                if(r!=NULL)
                    delete r;

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

