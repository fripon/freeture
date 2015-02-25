/*
								CameraSDK.h

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
* \file    CameraSDK.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    21/01/2015
* \brief   Parent class of Camera's SDK.
*/

#pragma once


#include "Frame.h"
#include "ECamBitDepth.h"
#include "EAcquisitionMode.h"

using namespace std;

class CameraSDK{

	public:

		CameraSDK(void);

		virtual ~CameraSDK(void);

        //! Print a list of available cameras.
		virtual void	listCameras(void)                           {};

		//! Select a camera by its name.
		virtual bool	chooseDevice(string)                        {return false;};

		virtual bool	chooseDevice(int, string)                   {return false;};

		//! Prepare the grabbing of frames.
		virtual bool	grabStart()							        {return false;};

		virtual bool	grabStart(int camFps)						{return false;};

		//! Start acquisition.
		virtual void	acqStart(CamAcqMode mode)					{};

		//! Stop acquisition and clean ressources.
		virtual void	acqStop()							        {};

		//! Stop grabbing frames and clean ressources.
		virtual void	grabStop()							        {};

        //! Grab a frame.
        virtual bool    grabImage(Frame &newFrame)                  {return false;};

        virtual bool	grabImage(Frame *&newFrame, Mat newImage)   {return false;};

		//! Restart grabbing images
		virtual void	grabRestart()				                {};

        //! Get the minimum available exposition value.
		virtual double	getExpoMin(void)						    {return -1;};

		//! Get the maximum available exposition value.
		virtual double	getExpoMax(void)						    {return -1;};

		//! Get the minimum available gain value.
		virtual	int		getGainMin(void)						    {return	-1;};

		//! Get the maximum available exposition value.
		virtual	int		getGainMax(void)						    {return	-1;};

		//! Get frame's bit depth.
		virtual	bool	getPixelFormat(CamBitDepth &format)		    {return false;};

		//! Get frame's width.
		virtual	int		getWidth(void)						        {return	-1;};

		//! Get frame's height.
		virtual	int		getHeight(void)						        {return	-1;};

		//! Get camera's fps.
		virtual	double	getFPS(void)						        {return -1;};

		//! Get camera's model name.
		virtual	string	getModelName()							    {return "";};

        //! Set camera's exposure time.
		virtual bool	setExposureTime(double)					    {return false;};

		//! Get camera's exposure time.
		virtual double	getExposureTime()					        {return -1.0;};

		//! Set camera's gain.
		virtual bool	setGain(int)						        {return false;};

        //! Set camera's fps.
		virtual bool	setFPS(int)						            {return false;};

		//! Get camera's gain.
		virtual int	    getGain()						            {return -1;};

		//! Set frame's bit depth.
		virtual	bool	setPixelFormat(CamBitDepth depth)	        {return false;};

        //! Get camera's name by its id.
		virtual bool    getDeviceById(int id, string &device)       {return false;}

		virtual bool	grabSingleImage(Frame &frame, int camID)							{return false;};

};
