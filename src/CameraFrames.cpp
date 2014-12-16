/*
								CameraFrames.cpp

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
 * @file    CameraFrames.cpp
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    02/09/2014
 */

#include "CameraFrames.h"


CameraFrames::CameraFrames( string dir,
                            int frameStart,
                            int frameStop,
                            Fifo<Frame> *queue,
                            boost::mutex *m_mutex_queue,
                            boost::condition_variable *m_cond_queue_fill,
                            boost::condition_variable *m_cond_queue_new_element,
                            Fits fitsHead){
    fitsHeader = fitsHead;
	dirPath             = dir;
	thread              = NULL;
	frameQueue          = queue;
	mutexQueue			= m_mutex_queue;
	condQueueFill		= m_cond_queue_fill;
	condQueueNewElement	= m_cond_queue_new_element;
	imgH = 0;
	imgW = 0;
	frameStart_   = frameStart;
	frameStop_    = frameStop;
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

        std::cout << "Directory " << p.string() << " exists." << '\n';

        do{

            string filename = "";

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

            Mat resMat;

            Fits2D newFits(filename,fitsHeader);
            newFits.readFits16S(resMat, filename);

            //Timestamping

            string acquisitionDate = TimeDate::localDateTime(second_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");

            Frame newFrameRAM( resMat, 0, 0, acquisitionDate );
            newFrameRAM.setNumFrame(cpt);
            newFrameRAM.setFrameRemaining(frameStop_ - cpt);

            boost::mutex::scoped_lock lock(*mutexQueue);
            frameQueue->pushInFifo(newFrameRAM);
            lock.unlock();

            if(frameQueue->getFifoIsFull()) condQueueFill->notify_all();
            frameQueue->setThreadRead("imgCap", true);
            frameQueue->setThreadRead("astCap", true);
            frameQueue->setThreadRead("det", true);
            condQueueNewElement->notify_all();

            fileFound = false;

            waitKey(100) ;

        }while(!endReadFrames);

        endReadFrames = false;
        cpt = 0;
        numFrame = frameStart_;

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
