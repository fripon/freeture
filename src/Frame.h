/*
								Frame.h

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
 * @file    Frame.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    19/06/2014
 */

#pragma once

#include "includes.h"
#include "Conversion.h"

using namespace std;
using namespace cv;

//!  This class provides informations about a grabbed frame
class Frame{

	private:

        //! Date of acquisition
		string   acqDate;

		//! Date of acquisition in a vector
        vector<int> date;

        //! Gain value of the frame
        int gain;

        //! Exposure value of the frame
        int exp;

        //! Path of the frame stocked on the hard disk
        string pathOnDisk;

        //! Mat frame
        Mat img;

        //! Name of the frame's file
        string fileName;

        map <string, bool> threadReadingStatus;

        int frameNumber;

        vector<string> dateString;

        int frameRemaining;

	public:

        //! Constructor without Mat
        Frame(vector<int>  acquisitionDate, int g, int e);

        //! Constructor with Mat
        Frame(Mat capImg, int g, int e, string acquisitionDate);

        Frame(string  acquisitionDate, int g, int e);

        //! Simple constructor
        Frame();

        //! Destructor
        ~Frame(void);

        bool copyFrame(Frame *& frameToCopy);

        bool copyFrame(Frame*& frameToCopy, Mat mask);

		//! Getter on the path where the frame data are stocked
		string getPath();

		//! Getter on the name of the file of the frame
		string getFileName();

		//! Getter on the date of acquisition
		string getAcqDate();

		vector<string> getDateString();


		//! Getter on the date of acquisition
		vector <int> getDate();

		int getNumFrame();

		void setNumFrame(int n);


		int getFrameRemaining();

		void setFrameRemaining(int val);

		//! Getter on the exposure value
		int getExposure();

        //! Getter on the gain value
		int getGain();

        //! Getter on the Mat
		Mat getImg();



        //! Setter on the Mat
        void setImg(Mat i);

		//! Setter on the path where the frame data are stocked
		void setPath (string p);

		//! Setter on the name of the file of the frame
		void setFileName (string val);

		//! Setter on the date of acquisition
		void setAcqDate (string val);

		//! Setter on the date of acquisition
		void setDate(vector<int> val);

		//! Setter on the exposure value
		void setExposure(int val);

        //! Setter on the gain value
		void setGain(int val);

		void setThreadReadingStatus(string threadName, bool status);

        bool getThreadReadingStatus (string threadName);

};
