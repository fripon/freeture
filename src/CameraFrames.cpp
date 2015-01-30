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
* \file    CameraFrames.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    02/09/2014
* \brief   Acquisition thread with fits individual frames in input.
*/

#include "CameraFrames.h"

CameraFrames::CameraFrames( string dir,
                            int sepPos,
                            string sep,
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

    separatorPosition = sepPos;
    separator = sep;

	imgH = 0;
	imgW = 0;

	newFrameDet = newFrameForDet;
    m_newFrameDet = m_newFrameForDet;
    c_newFrameDet = c_newFrameForDet;
    frameBuffer = cb;
    m_frameBuffer = m_cb;
    c_newElemFrameBuffer = c_newElemCb;
}

CameraFrames::~CameraFrames(void){}

void CameraFrames::startThread(){

	// Launch acquisition thread
	thread = new boost::thread(boost::ref(*this));

}

void CameraFrames::join(){

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

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "ACQ_THREAD");
	BOOST_LOG_SEV(log,notification) << "\n";
	BOOST_LOG_SEV(log,notification) << "==============================================";
	BOOST_LOG_SEV(log,notification) << "========== Start acquisition thread ==========";
	BOOST_LOG_SEV(log,notification) << "==============================================";

    bool endReadFrames = false;
    bool fileFound = false;

    int cpt = 0;

    namespace fs = boost::filesystem;

    path p(dirPath);

    if(fs::exists(p)){

        int firstFrame = -1, lastFrame = 0;
        string filename = "";

        /// Search first and last frames.
        for(directory_iterator file(p);file!= directory_iterator(); ++file){
            path curr(file->path());
            cout << file->path() << endl;

            if(is_regular_file(curr)){

                list<string> ch;
                stringtok(ch, curr.filename().c_str(), separator.c_str());
                std:list<string>::const_iterator lit(ch.begin()), lend(ch.end());

                int i = 0, number = 0;

                for(; lit != lend; ++lit){

                    if(i == separatorPosition && i != ch.size() - 1){
                        number = atoi((*lit).c_str()); break;
                    }

                    if(i == ch.size() - 1){

                        list<string> ch_;
                        cout << ch_.front() << endl;
                        stringtok(ch_, (*lit).c_str(), ".");
                        cout << ch_.front() << endl;
                        number = atoi(ch_.front().c_str());
                        break;

                    }

                    i++;

                }

                if(firstFrame == -1){

                    firstFrame = number;

                }else if(number < firstFrame){

                    firstFrame = number;

                }

                if(number > lastFrame){

                    lastFrame = number;

                }

                cout << "number : " << number << endl;
                cout << ">> First Frame : " << firstFrame << endl;
                cout << ">> Last Frame  : " << lastFrame  << endl;

            }
        }

        cout << ">> First Frame : " << firstFrame << endl;
        cout << ">> Last Frame  : " << lastFrame  << endl;

        do{

            filename = "";

            /// Search a frame in the directory.

            for(directory_iterator file(p);file!= directory_iterator(); ++file){

                path curr(file->path());

                if(is_regular_file(curr)){

                    list<string> ch;

                    stringtok(ch,curr.filename().c_str(), separator.c_str());

                    list<string>::const_iterator lit(ch.begin()), lend(ch.end());
                    int i = 0;
                    int number = 0;

                    for(; lit != lend; ++lit){

                        if(i == separatorPosition){

                            number = atoi((*lit).c_str()); break;
                        }

                        if(i == ch.size() - 1 && i != ch.size() - 1){

                        list<string> ch_;
                        stringtok(ch_, (*lit).c_str(), ".");
                        number = atoi(ch_.front().c_str());
                        break;

                    }

                        i++;

                    }

                    if(number == firstFrame){

                        firstFrame++;
                        fileFound = true;
                        cpt++;
                        cout << "FILE:" << file->path().c_str() << endl;

                        filename = file->path().c_str() ;

                        break;

                    }
                }
            }

            if(firstFrame > lastFrame){

                endReadFrames = true;

            }else if(!fileFound){

                endReadFrames = true;

            }

            /// Read the frame.

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

            Frame newFrame(resMat, 0, 0.0, TimeDate::localDateTime(second_clock::universal_time(),"%Y:%m:%d:%H:%M:%S"));
            boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
            string acqDateInMicrosec = to_iso_extended_string(time);
            newFrame.setAcqDateMicro(acqDateInMicrosec);
            newFrame.setNumFrame(cpt);
            newFrame.setFrameRemaining(lastFrame - cpt);
            newFrame.setFPS(1);
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

    }else{

        cout << "Path of frames " << dirPath << " doesn't exist." << endl;

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
