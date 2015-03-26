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
                    BOOST_LOG_SEV(logger, normal) << "INPUT : BASLER_GIGE -> Use Pylon";
                    cam = new CameraGigeSdkPylon();
                #else
                    #ifdef LINUX
                        BOOST_LOG_SEV(logger, normal) << "INPUT : BASLER_GIGE -> Use Aravis";
                        cam = new CameraGigeSdkAravis();
                    #endif
                #endif
            }

            break;

        case DMK_GIGE:

            {

                #ifdef WINDOWS
                    BOOST_LOG_SEV(logger, normal) << "INPUT : DMK_GIGE -> Use Imaging Source";
                    cam = new CameraGigeSdkIc();
                #else
                    #ifdef LINUX
                        BOOST_LOG_SEV(logger, normal) << "INPUT : DMK_GIGE -> Use Aravis";
                        cam = new CameraGigeSdkAravis(true);
                    #endif
                #endif

            }

            break;

        default :

            cam = NULL;

    }
}

Device::~Device(){

    if(cam != NULL) delete cam;
}

bool Device::prepareDevice(CamType type, string cfgFile){

	try{

		Configuration cfg;
		cfg.Load(cfgFile);

		switch(type){

			case FRAMES :

				{

                    // Get frames location.
					string INPUT_DATA_PATH; cfg.Get("INPUT_DATA_PATH", INPUT_DATA_PATH);
					BOOST_LOG_SEV(logger, normal) << "Read INPUT_DATA_PATH from configuration file : " << INPUT_DATA_PATH;

					// Get separator position in frame's name.
					int FRAMES_SEPARATOR_POSITION; cfg.Get("FRAMES_SEPARATOR_POSITION", FRAMES_SEPARATOR_POSITION);
					BOOST_LOG_SEV(logger, normal) << "Read FRAMES_SEPARATOR_POSITION from configuration file : " << FRAMES_SEPARATOR_POSITION;

                    // Create camera using pre-recorded fits2D in input.
					cam = new CameraFrames(INPUT_DATA_PATH, FRAMES_SEPARATOR_POSITION);
					cam->grabStart();

				}

				break;

			case VIDEO:

				{
				    // Get frames location.
					string	INPUT_DATA_PATH; cfg.Get("INPUT_DATA_PATH", INPUT_DATA_PATH);

                    // Create camera using pre-recorded video in input.
					cam = new CameraVideo(INPUT_DATA_PATH);
				}

				break;

			default :

                // Get camera id.
				int CAMERA_ID;
				cfg.Get("CAMERA_ID", CAMERA_ID);

                // Get camera format.
				string acq_bit_depth;
				cfg.Get("ACQ_BIT_DEPTH", acq_bit_depth);
				EParser<CamBitDepth> cam_bit_depth;
				CamBitDepth ACQ_BIT_DEPTH = cam_bit_depth.parseEnum("ACQ_BIT_DEPTH", acq_bit_depth);

                // Get camera exposure.
				int ACQ_EXPOSURE;
				cfg.Get("ACQ_EXPOSURE", ACQ_EXPOSURE);

                // Get camera gain.
				int ACQ_GAIN;
				cfg.Get("ACQ_GAIN", ACQ_GAIN);

                // Get camera fps.
				int ACQ_FPS;
				cfg.Get("ACQ_FPS", ACQ_FPS);

                // List Gige Camera to check the ID.
				cam->listGigeCameras();

                // Create camera according to its ID.
				if(!cam->createDevice(CAMERA_ID)) throw "Fail to create device.";
				// Set camera format.
				if(!cam->setPixelFormat(ACQ_BIT_DEPTH)) throw "Fail to set Format.";
				// Set camera exposure time.
				if(!cam->setExposureTime(ACQ_EXPOSURE)) throw "Fail to set Exposure.";
				// Set camera gain.
				if(!cam->setGain(ACQ_GAIN)) throw "Fail to set Gain.";
				// Set camera fps.
				if(!cam->setFPS(ACQ_FPS)) throw "Fail to set Fps.";
				// Prepare grabbing.
				if(!cam->grabStart()) throw "Fail to start grab.";
				// Start acquisition.
				cam->acqStart();

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

void Device::listGigeCameras(){
	cam->listGigeCameras();
}

void Device::grabStop(){
	cam->grabStop();
}

bool Device::getDeviceStopStatus(){
	return cam->getStopStatus();
}

void Device::acqStop(){
	cam->acqStop();
}

bool Device::grabImage(Frame& newFrame){
	return cam->grabImage(newFrame);
}

bool Device::grabSingleImage(Frame &frame, int camID){
	return cam->grabSingleImage(frame, camID);
}

void Device::getExposureBounds(int &gMin, int &gMax){
	cam->getExposureBounds(gMin, gMax);
}

void Device::getGainBounds(int &eMin, int &eMax){
	cam->getGainBounds(eMin, eMax);
}

bool Device::getPixelFormat(CamBitDepth &format){
	return cam->getPixelFormat(format);
}

int Device::getWidth(){
	return cam->getWidth();
}

int Device::getHeight(){
	return cam->getHeight();
}

int Device::getFPS(){
    return cam->getFPS();
}

string Device::getModelName(){
	return cam->getModelName();
}

bool Device::setExposureTime(int exp){
	return cam->setExposureTime(exp);
}

bool Device::setGain(int gain){
	return cam->setGain(gain);
}

bool Device::setFPS(int fps){
	return cam->setFPS(fps);
}

bool Device::setPixelFormat(CamBitDepth depth){
	return cam->setPixelFormat(depth);
}
