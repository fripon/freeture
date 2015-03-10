/*
								Device.cpp

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
* \file    Device.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    02/09/2014
* \brief
*/

#include "Device.h"

boost::log::sources::severity_logger< LogSeverityLevel >  Device::logger;
Device::_Init Device::_initializer;

	Device::Device(CamType type){

		switch(type){

			case BASLER_GIGE :

				{
					#ifdef USE_PYLON
						cam = new CameraGigeSdkPylon();
					#else
						#ifdef LINUX
                            cout << "use aravis sdk" << endl;
							cam = new CameraGigeSdkAravis();
						#endif
					#endif
				}

				break;

			case DMK_GIGE:

				{

					#ifdef WINDOWS
						//cam = new CameraGigeSdkIc();
					#else
						#ifdef LINUX
							cam = new CameraGigeSdkAravis();
						#endif
					#endif

				}

				break;

			default :

				cam = NULL;

		}
}

bool Device::prepareDevice(CamType type, string cfgFile){

	try{

		Configuration cfg;
		cfg.Load(cfgFile);

		switch(type){

			case FRAMES :

				{

					string	INPUT_DATA_PATH; cfg.Get("INPUT_DATA_PATH", INPUT_DATA_PATH);
					int		FRAMES_SEPARATOR_POSITION; cfg.Get("FRAMES_SEPARATOR_POSITION", FRAMES_SEPARATOR_POSITION);

					cam = new CameraFrames(INPUT_DATA_PATH, FRAMES_SEPARATOR_POSITION);
					cam->grabStart();

				}

				break;

			case VIDEO:

				{
					string	INPUT_DATA_PATH; cfg.Get("INPUT_DATA_PATH", INPUT_DATA_PATH);

					cam = new CameraVideo(INPUT_DATA_PATH);
				}

				break;

			default :

				int CAMERA_ID;
				cfg.Get("CAMERA_ID", CAMERA_ID);

				string acq_bit_depth;
				cfg.Get("ACQ_BIT_DEPTH", acq_bit_depth);
				EParser<CamBitDepth> cam_bit_depth;
				CamBitDepth ACQ_BIT_DEPTH = cam_bit_depth.parseEnum("ACQ_BIT_DEPTH", acq_bit_depth);

				int ACQ_EXPOSURE;
				cfg.Get("ACQ_EXPOSURE", ACQ_EXPOSURE);

				int ACQ_GAIN;
				cfg.Get("ACQ_GAIN", ACQ_GAIN);

				int ACQ_FPS;
				cfg.Get("ACQ_FPS", ACQ_FPS);

                cout << "List cameras" << endl;
				cam->listGigeCameras();
				cout << "CreateDevice" << endl;
				if(!cam->createDevice(CAMERA_ID))
                    throw "Fail to create device.";
				cam->setPixelFormat(ACQ_BIT_DEPTH);
				cam->setExposureTime(ACQ_EXPOSURE);
				cam->setGain(ACQ_GAIN);
				cam->setFPS(ACQ_FPS);
				cam->grabStart();
				cam->acqStart();
				getchar();

		}

	}catch(exception& e){

		cout << e.what() << endl;
		return false;

	}catch(const char * msg){

		cout << msg << endl;
		return false;

	}

	return true;
}

		Device::~Device(){

			if(cam != NULL) delete cam;
}

void	Device::listGigeCameras(){
	cam->listGigeCameras();
}

void	Device::grabStop(){
	cam->grabStop();
}

bool	Device::getDeviceStopStatus(){
	return cam->getStopStatus();
}

void    Device::acqStop(){
	cam->acqStop();
}

bool    Device::grabImage(Frame& newFrame){
	return cam->grabImage(newFrame);
}

bool	Device::grabSingleImage(Frame &frame, int camID){
	return cam->grabSingleImage(frame, camID);
}

void	Device::getExposureBounds(int &gMin, int &gMax){
	cam->getExposureBounds(gMin, gMax);
}

void	Device::getGainBounds(int &eMin, int &eMax){
	cam->getGainBounds(eMin, eMax);
}

bool	Device::getPixelFormat(CamBitDepth &format){
	return cam->getPixelFormat(format);
}

int		Device::getWidth(){
	return cam->getWidth();
}

int		Device::getHeight(){
	return cam->getHeight();
}

int		Device::getFPS(){
	return cam->getFPS();
}

string	Device::getModelName(){
	return cam->getModelName();
}

bool	Device::setExposureTime(int exp){
	return cam->setExposureTime(exp);
}

bool	Device::setGain(int gain){
	return cam->setGain(gain);
}

bool    Device::setFPS(int fps){
	return cam->setFPS(fps);
}

bool	Device::setPixelFormat(CamBitDepth depth){
	return cam->setPixelFormat(depth);
}
