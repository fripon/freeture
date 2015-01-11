/*
				CameraSDK.h

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
 * @file    CameraSDK.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    30/06/2014
 */

#pragma once

#include "includes.h"
#include "Frame.h"
#include "ECamBitDepth.h"

using namespace std;

//! Camera's SDK class
class CameraSDK{

	public:

        //! Constructor
		CameraSDK(void);

		//! Destructor
		~CameraSDK(void);

        //! List connected cameras
		virtual void	listCameras(void)                           {};

		//! Select a connected device
		virtual bool	chooseDevice(string)                   {return false;};

		virtual bool	chooseDevice(int, string)                   {return false;};

		//! Start grabbing images
		virtual bool		grabStart()							        {return -1;};

		//! Start acquisition
		virtual void	acqStart()							        {};

		//! Stop acquisition
		virtual void	acqStop()							        {};

		//! Stop grabbing images
		virtual void	grabStop()							        {};

        //! Grab an image
        virtual bool    grabImage(Frame*& newFrame, Mat newImage)   {return false;};

		//! Restart grabbing images
		virtual void	grabRestart()				                {};

        //! Get the minimum exposition value
		virtual double	getExpoMin(void)						    {return -1;};

		//! Get the maximum exposition value
		virtual double	getExpoMax(void)						    {return -1;};

		//! Get the minimum gain value
		virtual	int		getGainMin(void)						    {return	-1;};

		//! Get the maximum exposition value
		virtual	int		getGainMax(void)						    {return	-1;};

		//! Get the pixel's format
		virtual	CamBitDepth		getPixelFormat(void)					    {return	DEPTH_ERROR;};

		//! Get the width
		virtual	int		getWidth(void)						        {return	-1;};

		//! Get the height
		virtual	int		getHeight(void)						        {return	-1;};

		//! Get the frame per second rate
		virtual	double	getFPS(void)						        {return -1;};

		//! Get the camera's model name
		virtual	string	getModelName()							    {return "";};

        //! Set the exposure time
        /*!
          \param exp exposure time
        */
		virtual bool	setExposureTime(double)					    {return false;};

		//! Get the exposure time
        /*!
          \param exp exposure time
        */
		virtual double	getExposureTime()					        {return -1.0;};

		//! Set the gain
        /*!
          \param gain
        */
		virtual bool	setGain(int)						        {return false;};

		virtual bool	setFPS(int)						            {return false;};

		//! Get the gain
        /*!
          \param gain
        */
		virtual int	getGain()						                {return -1;};

		//! Set the pixel format
        /*!
          \param format 12 or 8 bits
        */
		virtual	bool	setPixelFormat(CamBitDepth depth)	    {return false;};

		virtual bool    getDeviceById(int id, string &device){return false;}

};

