/*								CameraSDKAravis.h

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
 * \file    CameraSDKAravis.h
 * \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * \version 1.0
 * \date    21/01/2015
 * \brief   Use Aravis library to pilot GigE Cameras.
 *          https://wiki.gnome.org/action/show/Projects/Aravis?action=show&redirect=Aravis
 */

#pragma once

#include "iostream"
#include "CameraSDK.h"
#include "Frame.h"
#include "TimeDate.h"
#include "arv.h"
#include "arvinterface.h"
#include "ECamBitDepth.h"
#include "ELogSeverityLevel.h"

using namespace cv;
using namespace std;

class CameraSDKAravis: public CameraSDK{

    private:

        ArvCamera       *camera;
        ArvPixelFormat  pixFormat;
        ArvStream       *stream;

        int             width;
        int             height;
        double          fps;
        double          gainMin,
                        gainMax;
        unsigned int    payload;

        const char      *pixel_format_string;
        double          exposureMin,
                        exposureMax;
        const char      *caps_string;
        int             gain;
        double          exp;

        guint64         n_completed_buffers;
        guint64         n_failures;
        guint64         n_underruns;

    public:

		CameraSDKAravis();

		~CameraSDKAravis();

		void	listCameras(void);

		bool	chooseDevice(string);

		bool	grabStart();

		void	grabStop();

        bool    grabImage(Frame& newFrame);

        bool	grabImage(Frame *&newFrame, Mat newImage){return true;};

		void	grabRestart();

		double	getExpoMin(void);

		double	getExpoMax(void);

		int		getGainMin(void);

		int		getGainMax(void);

		bool	getPixelFormat(CamBitDepth &format);

		int		getWidth(void);

		int		getHeight(void);

		double	getFPS(void);

		string	getModelName();

		bool    getDeviceById(int id, string &device);

		bool	setExposureTime(double exposition);

		bool	setGain(int gain);

		bool    setFPS(int fps);

		bool	setPixelFormat(CamBitDepth depth);

		void    acqStart(bool continuousAcquisition);

        void    acqStop();

};
