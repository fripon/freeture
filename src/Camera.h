/*
								Camera.h

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
*	Last modified:		21/01/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    Camera.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    13/06/2014
* \brief   
*/

#pragma once
#include "config.h"

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>

#include "ECamBitDepth.h"
#include "Frame.h"

using namespace cv;
using namespace std;

class Camera{

	public:

        Camera();

		~Camera();

		virtual void	listGigeCameras()							{};
		virtual bool	createDevice(int)							{return false;};
		virtual bool    getDeviceNameById(int id, string &device)   {return false;};
		virtual bool	getStopStatus()								{return false;};						
		virtual bool	grabStart()									{return false;};	// Prepare grabbing.
		virtual void	acqStart()									{};					// Launch grabbing.
		virtual void	acqStop()							        {};					// Stop grabbing.
		virtual void	grabStop()							        {};					// Free ressources and close device.

        virtual bool    grabImage(Frame &newFrame)                  {return false;};
		virtual bool	grabSingleImage(Frame &frame, int camID)	{return false;};

		virtual void	getExposureBounds(int &eMin, int &eMax)		{};
		virtual void	getGainBounds(int &gMin, int &gMax)			{};
		virtual	bool	getPixelFormat(CamBitDepth &format)		    {return false;};
		virtual	int		getWidth()									{return	0;};
		virtual	int		getHeight()									{return	0;};
		virtual	int		getFPS()									{return 0;};
		virtual	string	getModelName()							    {return "";};
		virtual int	    getGain()						            {return 0;};
		virtual int		getExposureTime()					        {return 0;};

		virtual bool	setExposureTime(int)					    {return false;};
		virtual bool	setGain(int)						        {return false;};
		virtual bool	setFPS(int)						            {return false;};
		virtual	bool	setPixelFormat(CamBitDepth)					{return false;};

};
