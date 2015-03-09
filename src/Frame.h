/*
								Frame.h

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
* \file    Frame.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    19/06/2014
* \brief   Frame grabbed from a camera or other input with an image.
*/

#pragma once

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <boost/tokenizer.hpp>

#include "Conversion.h"
#include "SaveImg.h"
#include "ECamBitDepth.h"

using namespace std;
using namespace cv;

class Frame{

	private:

        //! Image format.
		CamBitDepth bitdepth;

        //! Acquisition date YYYY-MM-DDTHH:MM:SS
		string acqDate;

		//! Acquisition date in vector.
        vector<int> date;

        //! Acquisition date YYYY-MM-DDTHH:MM:SS,ff.
        string acqDateInMicrosec;

        //! Acquisition date YYYY:MM:DD:HH:MM:SS
        string rawDate;

        vector<string> dateString;

        float dateSeconds;

        //! Camera's gain value used to grab the frame.
        int gain;

        //! Camera's exposure value used to grab the frame.
        int exp;

        //! Frame's image data.
        Mat img;

        //! Frame's name.
        string fileName;

        //! Each frame is identified by a number corresponding to the acquisition order.
        int frameNumber;

        //! Define the number of remaining frames if the input source is a video or a set of single frames.
        int frameRemaining;

        //! Max pixel value in the image.
        double saturatedValue;

        //! Camera's fps parameter.
        int fps;

	public:

        Frame(Mat capImg, int g, int e, string acquisitionDate);

        Frame();

        ~Frame();

		vector<string>  getDateString       ()                      {return dateString;};

		CamBitDepth		getFrameBitDepth	()						{return bitdepth;};

		int             getNumFrame         ()                      {return frameNumber;};
		void            setNumFrame         (int n)                 {frameNumber = n;};

        float           getDateSeconds      ()                      {return dateSeconds;};

        void            setAcqDateMicro     (string date);
		string          getAcqDateMicro     ()                      {return acqDateInMicrosec;};

		CamBitDepth     getBitDepth         ()                      {return bitdepth;};
		void            setBitDepth         (CamBitDepth depth)     {bitdepth = depth;};

        int             getFrameRemaining   ()                      {return frameRemaining;};
        void            setFrameRemaining   (int val)               {frameRemaining = val;};

        string          getRawDate          ()                      {return rawDate;};
        void            setRawDate          (string d)              {rawDate = d;};

		double          getSaturatedValue   ()                      {return saturatedValue;};
        void            setSaturatedValue   (double val)            {saturatedValue = val;};

		int             getFPS              ()                      {return fps;};
		void            setFPS              (int f)                 {fps = f;};

		int             getExposure         ()                      {return exp;};
        void            setExposure         (int val)               {exp = val;};

		int             getGain             ()                      {return gain;};
        void            setGain             (int val)               {gain = val;};

		Mat             getImg              ()                      {return img;};
        void            setImg              (Mat i)                 {i.copyTo(img);};

		string          getFileName         ()                      {return fileName;};
		void            setFileName         (string val)            {fileName = val;};

		string          getAcqDate          ()                      {return acqDate;};
		void            setAcqDate          (string val)            {acqDate = val;};

		vector<int>     getDate             ()                      {return date;};
		void            setDate             (vector<int> val)       {date = val;};

};
