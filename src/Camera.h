/*
								Camera.h

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
 * @file    Camera.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    01/07/2014
 */

#pragma once

#include "includes.h"
#include "ECamBitDepth.h"

using namespace std;
using namespace cv;

//! Camera class
class Camera{

	public:

        //! Constructor
		Camera(void);

		//! Destructor
		virtual ~Camera(void);

        //! List connected cameras
		virtual void	getListCameras(void)                            {};

		//! Select a connected device
		virtual bool	setSelectedDevice(int, string)                  {return false;};

		virtual bool	setSelectedDevice(string)                  {return false;};

        //! Wait the end of the acquisition thread
		virtual void    join()                                          {};

		//! Stop the acquisition thread
		virtual void	stopThread()                                    {};

		//! Start the acquisition thread
		virtual void	startThread()                                   {};

		virtual bool    startGrab()                                     {};

		virtual void    stopGrab()                                      {};

		virtual bool    grabSingleFrame(Mat &frame, string &date)                      {};

        //! Get the minimum exposition value
		virtual double	getCameraExpoMin(void)						    {return -1;};

		//! Get the maximum exposition value
		virtual double	getCameraExpoMax(void)						    {return -1;};

		//! Get the minimum gain value
		virtual	int		getCameraGainMin(void)						    {return	-1;};

		//! Get the maximum exposition value
		virtual	int		getCameraGainMax(void)					    	{return	-1;};

		//! Get the pixel's format
		virtual	int		getCameraPixelFormat(void)						{return	-1;};

		//! Get the width
		virtual	int		getCameraWidth(void)						    {return	-1;};

		//! Get the height
		virtual	int		getCameraHeight(void)						    {return	-1;};

		//! Get the frame per second rate
		virtual	double	getCameraFPS(void)						        {return -1;};

		//! Get the camera's model name
		virtual	string	getCameraModelName()							{return "";};

        //! Set the exposure time
		virtual bool	setCameraExposureTime(double)					{};

        //! Get the exposure time
		virtual double	getCameraExposureTime()					        {};

		//! Set the gain
		virtual bool	setCameraGain(int)						        {};

		//! Set FPS.
		virtual bool    setCameraFPS(int)                               {};

		//! Get the gain
		virtual int	    getCameraGain()						            {return	-1;};

		//! Set the pixel format
		virtual	bool	setCameraPixelFormat(CamBitDepth depth)		{};

		virtual bool    getDeviceById(int id, string &device){return false;}

};

