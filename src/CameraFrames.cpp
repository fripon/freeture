/*
								CameraFrames.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau
*                               FRIPON-GEOPS-UPSUD-CNRS
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
 * @file    CameraFrames.cpp
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    02/09/2014
 */

#include "CameraFrames.h"


CameraFrames::CameraFrames( string dir,
                            int frameStart,
                            int frameStop,
                            Fits fitsHead,
                            int bitdepth,
                            boost::circular_buffer<Frame> *cb,
                            boost::mutex *m_cb,
                            boost::condition_variable *c_newElemCb,
                            bool *newFrameForDet,
                            boost::mutex *m_newFrameForDet,
                            boost::condition_variable *c_newFrameForDet){

    bitpix              = bitdepth;
    fitsHeader          = fitsHead;
	dirPath             = dir;
	thread              = NULL;

	imgH = 0;
	imgW = 0;
	frameStart_   = frameStart;
	frameStop_    = frameStop;

	newFrameDet = newFrameForDet;
    m_newFrameDet = m_newFrameForDet;
    c_newFrameDet = c_newFrameForDet;
    frameBuffer = cb;
    m_frameBuffer = m_cb;
    c_newElemFrameBuffer = c_newElemCb;
}

CameraFrames::~CameraFrames(void){

}

void CameraFrames::startThread(){

	// Launch acquisition thread
	thread=new boost::thread(boost::ref(*this));

}

void CameraFrames::join(){

    cout << "waiting"<<endl;
	thread->join();

}

template <typename Container> void stringtok(Container &container, string const &in, const char * const delimiters = "_"){


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

void CameraFrames::operator () (){

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "acqThread");

	BOOST_LOG_SEV(log, critical) << "\n";

	BOOST_LOG_SEV(log,notification) << "Acquisition thread started.";

    string dir = dirPath;

    vector<Mat> listF;

    bool endReadFrames = false;
    bool fileFound = false;

    int numFrame = frameStart_;

    path p(dir);

    int cpt = 0;

    namespace fs = boost::filesystem;

    if(fs::exists(p)){

        do{

            string filename = "";

            /// === Search a frame in the directory. ===

            for(directory_iterator file(p);file!= directory_iterator(); ++file){

                path curr(file->path());

                if(is_regular_file(curr)){

                    list<string> ch;

                    stringtok(ch,curr.filename().c_str(),"_");

                    std:list<string>::const_iterator lit(ch.begin()), lend(ch.end());
                    int i = 0;
                    int number = 0;

                    for(;lit!=lend;++lit){

                        if(i==1){

                            number = atoi((*lit).c_str());
                            break;
                        }

                        i++;

                    }

                    if(number == numFrame){

                        numFrame++;
                        fileFound = true;
                        cpt++;
                        cout << "FILE:" << file->path().c_str() << endl;

                        filename = file->path().c_str() ;

                        break;

                    }
                }
            }

            if(numFrame > frameStop_){

                endReadFrames = true;

            }else if(!fileFound){

                endReadFrames = true;

            }

            /// === Read the frame. ===

            Mat resMat;
            Fits2D newFits;

            switch(bitpix){

                case 8 :

                    newFits.readFits8UC(resMat, filename);

                    break;

                case 16 :

                    newFits.readFits16S(resMat, filename);

                    break;

            }

            //Timestamping

            string acquisitionDate = TimeDate::localDateTime(second_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");

            Frame newFrame( resMat, 0, 0, acquisitionDate );
            newFrame.setNumFrame(cpt);
            newFrame.setFrameRemaining(frameStop_ - cpt);

            /*boost::mutex::scoped_lock lock(*mutexQueue);
            frameQueue->pushInFifo(newFrameRAM);
            lock.unlock();

            if(frameQueue->getFifoIsFull()) condQueueFill->notify_all();
            frameQueue->setThreadRead("imgCap", true);
            frameQueue->setThreadRead("astCap", true);
            frameQueue->setThreadRead("det", true);
            condQueueNewElement->notify_all();*/


            boost::mutex::scoped_lock lock(*m_frameBuffer);
            frameBuffer->push_back(newFrame);
            lock.unlock();

            boost::mutex::scoped_lock lock2(*m_newFrameDet);
            *newFrameDet = true;
            c_newFrameDet->notify_one();
            lock2.unlock();

            fileFound = false;

            waitKey(100) ;

        }while(!endReadFrames);

        endReadFrames = false;
        cpt = 0;
        numFrame = frameStart_;

    }else{

        cout << "Path of frames " << dirPath << "doesn't exist." << endl;

    }

	BOOST_LOG_SEV(log,notification) << "Acquisition thread terminated.";
	cout << "Acquisition thread terminated." << endl;

}

int CameraFrames::getHeight(){

	return imgH;

}

int CameraFrames::getWidth(){

	return imgW;

}
