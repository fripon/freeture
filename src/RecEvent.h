/*
				RecEvent.h

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
 * @file    RecEvent.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    12/06/2014
 */

#pragma once

#include "includes.h"
#include "Frame.h"
#include "SaveImg.h"

using namespace cv;
using namespace std;

class RecEvent{

    public:

        RecEvent();
        ~RecEvent();
        string getPath();
        void setPath(string newPath);
        void setPathOfFrames(string newPath);
        vector<int>  getPositionInBuffer();
        vector<Point> getListMetPos();
        vector<Frame> getBufferFileName();
        string getPathOfFrames();
        void setBufferFileName(vector<Frame> l);

        void setListMetPos(vector<Point> l);

        void setPositionInBuffer(vector<int> l);

        bool copyFromRecEvent(RecEvent ev);

        void setPrevFrames(vector<Frame> prev);
        void setFramesDisk(vector<int> f);
        void setFrameBufferLocation(string path);

        vector<Frame> getPrevFrames();
        vector<int> getFramesDisk();
        string getFrameBufferLocation();

        void setMapEvent(Mat mapE);

        void setDateEvent(vector<string> date);
        vector<string> getDateEvent();

        Mat getMapEvent();

        void setBuffer(vector<Mat> b);
        vector<Mat> getBuffer();
        void setDirMap(Mat dirMap);

        Mat getDirMap();

        void setPrevBuffer(vector<Mat> eventPrevBuffer);
        vector<Mat> getPrevBuffer();

    private:

        vector<Mat> prevBuffer;
        Mat mapDir;
        vector<Mat> buffer;
        vector<string> dateEv;
        Mat mapEvent;
        vector<Point> meteorPos;
        vector<int> posInBuffer;
        vector<Frame> bufferFileName;
        string path;
        string pathOfFrames;
        vector<Frame> previousFrames;
        vector<int> diskFrames;
        string bufferPath;

};
