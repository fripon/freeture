/*
							CameraGigeSdkPylon.cpp

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
* \file    CameraGigeSdkPylon.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/07/2014
* \brief   Use Pylon library to pilot GigE Cameras.
*/

#include "CameraGigeSdkPylon.h"

#ifdef USE_PYLON

boost::log::sources::severity_logger< LogSeverityLevel >  CameraGigeSdkPylon::logger;
CameraGigeSdkPylon::_Init CameraGigeSdkPylon::_initializer;

CameraGigeSdkPylon::CameraGigeSdkPylon(){

	pTlFactory			= NULL;
	pCamera				= NULL;
	pDevice				= NULL;
	pEventGrabber		= NULL;
	pEventAdapter		= NULL;
	pStreamGrabber		= NULL;
	nbEventBuffers		= 20;

	connectionStatus	= false;

}

CameraGigeSdkPylon::~CameraGigeSdkPylon(void){

	if(pEventAdapter != NULL)	delete pEventAdapter;
	if(pEventGrabber != NULL)	delete pEventGrabber;
	if(pStreamGrabber != NULL)	delete pStreamGrabber;
	if(pDevice != NULL)			delete pDevice;
	if(pCamera != NULL)			delete pCamera;
	if(pTlFactory != NULL)		delete pTlFactory;

}

void CameraGigeSdkPylon::listGigeCameras(){

	try{

		if (!pTlFactory){

			cout << "TransportLayer creation succeed" << endl;
			pTlFactory=&CTlFactory::GetInstance();

		}

		// Exit the application if the specific transport layer is not available
		if (!pTlFactory){

			cout << "Failed to create TransportLayer" << endl;
			return;
		}

		if (!connectionStatus){

			devices.clear();

			if (pTlFactory){

				if (0 == pTlFactory->EnumerateDevices(devices)){

					cout << "No cameras detected..." << endl;

				}else{

					cout << "(ID) Cameras has been detected : " << endl;

				}

				int id=0;

				if ( ! devices.empty() && ! connectionStatus){

					DeviceInfoList_t::const_iterator it;

					for (it = devices.begin(); it != devices.end(); ++it ){

						if (!devices.empty()){

							if(devices[id].GetFullName().find_first_of("Basler")==0||devices[id].GetFullName().find_first_of("Prosilica")==0){

								cout<<"-->";
								cout<< "	ID :		"		<<id<<endl;
								cout<< "	S/N :		"		<<devices[id].GetSerialNumber().c_str()<<endl;
								cout<< "	Name :		"		<<devices[id].GetModelName().c_str()<<endl;

								list<string> ch;
								Conversion::stringTok(ch,devices[id].GetFullName().c_str(), "#");

								cout<< "	Adress :	"		<<ch.back()<<endl<<endl;

							}
						}

						id++;
					}
				}
			}
		}

	}catch (GenICam::GenericException &e){

		cout << "An exception occured : " << e.GetDescription() << endl;

	}
}

bool CameraGigeSdkPylon::createDevice(int id){

    bool chooseState  = false;

    try{

		if(!devices.empty()){

			if(!pDevice){

				if(id >= 0 && id < devices.size()){

					pDevice = pTlFactory->CreateDevice(devices[id]);
					cout << "success to create device : " << id<< endl;

				}else{

					cout << "No camera with id : " << id << endl;

				}
			}

			if(pDevice){

				if(!pDevice->IsOpen()) pDevice->Open();

				if(pDevice->IsOpen()){

						connectionStatus = true;
						chooseState = true;
						cout << "device opened" << endl;

				}
			}

			if (pDevice && ! pCamera){

				pCamera=new CBaslerGigECamera(pDevice,true);
				cout << "Basler GigE camera created" << endl;
			}

		}else{
			cout << "Devices list is empty" << endl;
		}

	}catch (GenICam::GenericException &e){

		cout << e.GetDescription() << endl;

    }

	if(!chooseState){

		cout << "Camera is not accesible. Can be already opened by an another process." << endl;

	}

	return chooseState;

}

bool CameraGigeSdkPylon::getDeviceNameById(int id, string &device){

	pTlFactory = &CTlFactory::GetInstance();

	devices.clear();

	if(pTlFactory){

		if (0 == pTlFactory->EnumerateDevices(devices)){

			cout <<"No cameras detected..." << endl;
			return false;

		}else{

			cout << " Camera (ID:" << id << ") detected " << endl;
			cout << " Name :		" << devices[id].GetModelName().c_str() << endl;
			return true;
		}
	}else
		return false;

}

bool CameraGigeSdkPylon::grabInitialization(){

	cout << " Starting basler's grab initialization... " << endl;

	if(pCamera){

		if(pCamera->IsOpen()){

			try{

				// Check if the device supports events.
				if (!GenApi::IsAvailable( pCamera->EventSelector)){

					throw RUNTIME_EXCEPTION( "The device doesn't support events.");

				}

				// Create the event grabber
				pEventGrabber = new (CBaslerGigECamera::EventGrabber_t) (pCamera->GetEventGrabber());

				// parametrize and open it
				pEventGrabber->NumBuffer.SetValue(nbEventBuffers);

				// Enable resending of event messages when lossed messages are detected:
				// Loss of messages is detected by sending acknowledges for every event messages.
				// When the camera doesn't receive the acknowledge it will resend the message up to
				// 'RetryCount' times.
				pEventGrabber->RetryCount = 3;

				pEventGrabber->Open();

				// Create an event adaptater
				pEventAdapter = pCamera->CreateEventAdapter();

				//Disable acquisition start trigger if available
				{
					GenApi::IEnumEntry* acquisitionStart = pCamera->TriggerSelector.GetEntry( TriggerSelector_AcquisitionStart);

					if ( acquisitionStart && GenApi::IsAvailable( acquisitionStart)){

						pCamera->TriggerSelector.SetValue( TriggerSelector_AcquisitionStart);
						pCamera->TriggerMode.SetValue( TriggerMode_Off);

					}
				}

				//Disable frame start trigger if available
				{
					GenApi::IEnumEntry* frameStart = pCamera->TriggerSelector.GetEntry( TriggerSelector_FrameStart);

					if ( frameStart && GenApi::IsAvailable( frameStart)){

						pCamera->TriggerSelector.SetValue( TriggerSelector_FrameStart);
						pCamera->TriggerMode.SetValue( TriggerMode_Off);

					}
				}

				//Set acquisition mode
				cout << " Set acquisition mode to CONTINUOUS " << endl;
				pCamera->AcquisitionMode.SetValue(AcquisitionMode_Continuous);

				//Set exposure settings
				pCamera->ExposureMode.SetValue(ExposureMode_Timed);

				if (!pStreamGrabber){

					pStreamGrabber = new (CBaslerGigECamera::StreamGrabber_t)(pCamera->GetStreamGrabber(0));

				}

				pStreamGrabber->Open();

				// Get the image buffer size
				cout <<"Get image size : " << pCamera->PayloadSize.GetValue() << endl;
				const size_t ImageSize = (size_t)(pCamera->PayloadSize.GetValue());

				// We won't use image buffers greater than ImageSize
				pStreamGrabber->MaxBufferSize.SetValue(ImageSize);

				// We won't queue more than nbBuffers image buffers at a time
				pStreamGrabber->MaxNumBuffer.SetValue(nbBuffers);

				pStreamGrabber->PrepareGrab();

				for (int i = 0; i < nbBuffers; ++i){

					//ppBuffers[i] = new unsigned char[ImageSize];
					if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono8){

						ppBuffersUC[i] = new uint8_t[ImageSize];
						handles[i] = pStreamGrabber->RegisterBuffer(ppBuffersUC[i], ImageSize);

					}

					if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono12){

						ppBuffersUS[i] = new uint16_t[ImageSize];
						handles[i] = pStreamGrabber->RegisterBuffer(ppBuffersUS[i], ImageSize);

					}

					pStreamGrabber->QueueBuffer(handles[i], NULL);
				}

				// Launch acquisition thread
				cout << "Grab initilisation terminated." << endl;

			}catch (GenICam::GenericException &e){

				// Error handling.
				cout << "An exception occurred." << e.GetDescription() << endl;

			}
		}
	}

	return true;

}

void CameraGigeSdkPylon::grabCleanse(){

	if(pCamera){

		if(pCamera->IsOpen()){

			try{

				//if (grabStatus){

					// Flush the input queue, grabbing may have failed
					cout << "Flush the input queue" << endl;
					pStreamGrabber->CancelGrab();

					// Consume all items from the output queue
					GrabResult Result;

					while (pStreamGrabber->GetWaitObject().Wait(0)){

						pStreamGrabber->RetrieveResult(Result);

						if (Result.Status() == Canceled)
							cout << "Got canceled buffer" << endl;

					}

					// Deregister and free buffers
					for(int i = 0; i < nbBuffers; ++i){

						pStreamGrabber->DeregisterBuffer(handles[i]);

						cout << "Deregister and free buffer n° "<< i << endl;

						if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono8){

							delete [] ppBuffersUC[i];

						}else if (pCamera->PixelFormat.GetValue() == PixelFormat_Mono12){

							delete [] ppBuffersUS[i];

						}
					}

					//grabStatus = false;

					// Free all resources used for grabbing
					pStreamGrabber->FinishGrab();
					pStreamGrabber->Close();
					pEventGrabber->Close();

				//}

			}catch (GenICam::GenericException &e){

				// Error handling.
				cout << "An exception occurred." << e.GetDescription() << endl;

			}
		}

		pCamera->Close();
	}
}

void CameraGigeSdkPylon::acqStart(){

	pCamera->AcquisitionStart.Execute();

}

void CameraGigeSdkPylon::acqStop(){

    pCamera->AcquisitionStop.Execute();

}

bool CameraGigeSdkPylon::grabImage(Frame &newFrame){

    bool res = true;

 //  pCamera->TemperatureSelector.SetValue(TemperatureSelector_Sensorboard);

   // double temperature = pCamera->TemperatureAbs.GetValue();

          /* INodeMap* nodemap = pCamera->GetNodeMap();
           cout << "Vendor           : "
             << CStringPtr( nodemap->GetNode( "DeviceInformation") )->GetValue() << endl;

          // CFloatPtr tempe( nodemap->GetNode( "TemperatureAbs"));
//cout << "TEMPERATURE : " <<  tempe  <<endl;

           cout << "TEMPERATURE : " <<  temperature  <<endl;
*/

    if(pStreamGrabber->GetWaitObject().Wait(3000)){

        // Get an item from the grabber's output queue
        if(!pStreamGrabber->RetrieveResult(result)){

            res = false;
            cout << "Failed to retrieve an item from the output queue" << endl;

        }

        CGrabResultPtr ptrGrabResult;

        if(result.Succeeded()){
			cout << "success to grab a frame "<<endl;

			//Timestamping.
			string acquisitionDate = TimeDate::localDateTime(microsec_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");
			// Convert to form YYYY-MM-DDTHH:MM:SS,fffffffff where T is the date-time separator
			boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
			string acqDateInMicrosec = to_iso_extended_string(time);

			Mat newImg;

			if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono8){

				newImg = Mat(pCamera->Height.GetValue(), pCamera->Width.GetValue(), CV_8UC1, Scalar(0));

			}else if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono12){

				newImg = Mat(pCamera->Height.GetValue(), pCamera->Width.GetValue(), CV_16UC1, Scalar(0));

			}

            memcpy(newImg.ptr(), result.Buffer(), pCamera->PayloadSize.GetValue());

            newFrame = Frame(	newImg,
								pCamera->GainRaw.GetValue(),
								(double)pCamera->ExposureTimeAbs.GetValue(),
								acquisitionDate);

			newFrame.setAcqDateMicro(acqDateInMicrosec);
			newFrame.setFPS(pCamera->AcquisitionFrameRateAbs.GetValue());

			if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono8){

                newFrame.setBitDepth(MONO_8);
                newFrame.setSaturatedValue(255);

			}else if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono12){

                newFrame.setBitDepth(MONO_12);
                newFrame.setSaturatedValue(4095);

			}

        }else{

            res = false;
            cout << "Fail to grab a frame : " << result.GetErrorDescription() << endl;

        }

        // Requeue the buffer
        pStreamGrabber->QueueBuffer( result.Handle(), result.Context() );

    }else{

        cout <<"Fail to grab a frame (timeout) : " << result.GetErrorDescription() << endl;
        res = false;
    }

    return res;

}

bool CameraGigeSdkPylon::grabSingleImage(Frame &frame, int camID){

	try{

		pTlFactory = &CTlFactory::GetInstance();

		devices.clear();

		if(pTlFactory){

			if (0 == pTlFactory->EnumerateDevices(devices)){

				cout << "Camera (ID:" << camID << ") not found. " << endl;
				return false;

			}else{

				cout << "Camera (ID:" << camID << ") found ! " << endl;

			}
		}

		// Create an instant camera object with the correct id camera device.
		CInstantCamera camera( CTlFactory::GetInstance().CreateDevice(devices[camID].GetFullName()));

		// Print infos about the camera.
		cout << "Device selected :          " << camera.GetDeviceInfo().GetModelName() << endl;

		INodeMap& nodemap = camera.GetNodeMap();

        // Open the camera for accessing the parameters.
        camera.Open();

		CIntegerPtr width( nodemap.GetNode("Width"));
        CIntegerPtr height( nodemap.GetNode("Height"));

		// Set width and height to the maximum sensor's size.
        width->SetValue(width->GetMax());
        height->SetValue(height->GetMax());

		// Set pixel format.
		// Access the PixelFormat enumeration type node.
        CEnumerationPtr pixelFormat( nodemap.GetNode("PixelFormat"));

        // Remember the current pixel format.
        String_t oldPixelFormat = pixelFormat->ToString();
        cout << "Old PixelFormat :          " << oldPixelFormat << endl;

		if(frame.getBitDepth() == MONO_8){

			// Set the pixel format to Mono8 if available.
			if(IsAvailable(pixelFormat->GetEntryByName("Mono8"))){
				pixelFormat->FromString("Mono8");
				cout << "New PixelFormat :          " << pixelFormat->ToString() << endl;
			}else{
				cout << "Fail to set pixel format to MONO_8" << endl;
				return false;
			}

		}else if(frame.getBitDepth() == MONO_12){

			// Set the pixel format to Mono8 if available.
			if(IsAvailable(pixelFormat->GetEntryByName("Mono12"))){
				pixelFormat->FromString("Mono12");
				cout << "New PixelFormat :          " << pixelFormat->ToString() << endl;
			}else{
				cout << "Fail to set pixel format to MONO_12" << endl;
				return false;
			}

		}else{

			cout << "No depth specified for the frame container." << endl;
			return false;
		}

		// Set exposure.

		CIntegerPtr ExposureTimeRaw( nodemap.GetNode("ExposureTimeRaw"));
        if(ExposureTimeRaw.IsValid()){

			//cout << "Min exp : " << ExposureTimeRaw->GetMin() << endl;
			//cout << "Max exp : " << ExposureTimeRaw->GetMax() << endl;

			if(frame.getExposure() >= ExposureTimeRaw->GetMin() && frame.getExposure() <= ExposureTimeRaw->GetMax()){
				ExposureTimeRaw->SetValue(frame.getExposure());
				cout << "New exposure value :       " << frame.getExposure() << endl;
			}else{
				ExposureTimeRaw->SetValue(ExposureTimeRaw->GetMin());
				cout << endl << "WARNING : exposure has been setted with the minimum available value."
					 <<	" The wanted value is not in range [min = " << ExposureTimeRaw->GetMin() << ";" << "max= " << ExposureTimeRaw->GetMax() << "] (us)" << endl << endl;
			}
        }else{
			cout << "Fail to set exposure value" << endl;
			return false;
		}

		// Set gain.
		// Access the GainRaw integer type node. This node is available for Firewire and GigE Devices.
        CIntegerPtr gainRaw( nodemap.GetNode("GainRaw"));
        if(gainRaw.IsValid())
        {
			//cout << "Min gain : " << gainRaw->GetMin() << endl;
			//cout << "Max gain : " << gainRaw->GetMax() << endl;

			if(frame.getGain() >= gainRaw->GetMin() && frame.getGain() <= gainRaw->GetMax()){
				gainRaw->SetValue(frame.getGain());
				cout << "New gain value :           " << frame.getGain() << endl;
			}else{
				gainRaw->SetValue(gainRaw->GetMin());
				cout << endl << "WARNING : gain has been setted with the minimum available value."
					 <<	" The wanted value is not in range [min = " << gainRaw->GetMin() << ";" << "max= " << gainRaw->GetMax() << "]" << endl << endl;
			}
        }

		camera.Close();

		// This smart pointer will receive the grab result data.
		CGrabResultPtr ptrGrabResult;

		camera.GrabOne(500, ptrGrabResult);

		Mat newImg;

		// Image grabbed successfully?
		if(ptrGrabResult->GrabSucceeded()){

			// Access the image data.
			//cout << "SizeX: " << ptrGrabResult->GetWidth() << endl;
			//cout << "SizeY: " << ptrGrabResult->GetHeight() << endl;

			if(ptrGrabResult->GetPixelType()== PixelType_Mono8){
				//cout << "Pixel Format : Mono8" << endl;
				newImg = Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC1, Scalar(0));

			}else if(ptrGrabResult->GetPixelType()== PixelType_Mono12){
				//cout << "Pixel Format : Mono12" << endl;
				newImg = Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_16UC1, Scalar(0));

			}

			memcpy(newImg.ptr(), ptrGrabResult->GetBuffer(), ptrGrabResult->GetPayloadSize());

			frame.setImg(newImg);

			return true;

		}

		return false;

	}catch (GenICam::GenericException &e){
        // Error handling.
        cerr << "An exception occurred." << endl
			 << e.GetDescription() << endl;

    }
}

void CameraGigeSdkPylon::getExposureBounds(int &eMin, int &eMax){

}

void CameraGigeSdkPylon::getGainBounds(int &gMin, int &gMax){

}

bool CameraGigeSdkPylon::getPixelFormat(CamBitDepth &format){

    if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono8){

		format = MONO_8;
		return true;

    }else if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono12){

        format = MONO_12;

		return true;

    }

    return false;

}

int CameraGigeSdkPylon::getFrameWidth(){

	int imgW = 0;

	if(pCamera){

		try{

			if (pCamera->IsAttached() && pCamera->IsOpen()){

				imgW = pCamera->Width.GetValue();

			}else{

				cout << "Can't access width image. Camera not opened or not attached." << endl;

			}

		}catch (GenICam::GenericException &e){

			// Error handling
			cout << "An exception occurred."<<e.GetDescription() << endl;

		}
	}

	return imgW;

}

int CameraGigeSdkPylon::getHeight(){

	int imgH = 0;

	if(pCamera){

		try{

			if(pCamera->IsAttached() && pCamera->IsOpen()){

				imgH = pCamera->Height.GetValue();

			}else{

				cout << "Can't access height image. Camera not opened or not attached." << endl;

			}

		}catch(GenICam::GenericException &e){

			// Error handling
			cout << "An exception occurred." << e.GetDescription() << endl;

		}
	}

	return imgH;

}

int	CameraGigeSdkPylon::getFPS(){
	return 0;
}

string	CameraGigeSdkPylon::getModelName(){
	return "";
}

bool CameraGigeSdkPylon::setExposureTime(int exposition){

    if(pCamera){

        try{
            if ( pCamera->IsAttached() && pCamera->IsOpen() ){

                pCamera->ExposureTimeAbs = (double)exposition;

            }else{

				 std::cout << "Camera not opened or not attached" << endl;
            }

            return true;

        }catch (GenICam::GenericException &e){

            // Error handling
            cout << "An exception occurred." << e.GetDescription() << endl;
            return false;
        }
    }

    return false;

}

bool CameraGigeSdkPylon::setGain(int gain){

	if(pCamera){
		try{

			if( pCamera->IsAttached() && pCamera->IsOpen() ){

				pCamera->GainRaw = gain;

			}else{

				std::cout << "Camera not opened or not attached" << endl;
			}

			return true;

		}catch (GenICam::GenericException &e){

			// Error handling
			cout << "An exception occurred." << e.GetDescription() << endl;
			return false;
		}
	}else
		std::cout << "pCamera null"<< endl;
    return false;

}

bool CameraGigeSdkPylon::setFPS(int fps){
	return false;
}

bool CameraGigeSdkPylon::setPixelFormat(CamBitDepth format){

    Basler_GigECamera::PixelFormatEnums fpix;

	if(depth == MONO_8 ){

		fpix = PixelFormat_Mono8;

	}
	else if (depth == MONO_12 ){

		fpix = PixelFormat_Mono12;


	}

	if (pCamera){

		try{
			if(pCamera->IsAttached() && pCamera->IsOpen()){
				cout << "PayloadSize before : " << pCamera->PayloadSize.ToString().c_str() << endl;

				pCamera->PixelFormat.SetValue(fpix);

				cout << "PayloadSize after : " << pCamera->PayloadSize.ToString().c_str() << endl;

			}else{
				cout <<"Camera not opened or not attached" << endl;
			}
		}
		catch (GenICam::GenericException &e){
			// Error handling
			cout << "An exception occurred." << e.GetDescription() << endl;
		}

		return true;
	}

	return false;

}








/*


double  CameraSDKPylon::getExposureTime(){


     return (double)pCamera->ExposureTimeAbs.GetValue();

}

int     CameraSDKPylon::getGain(){

    return (int)((double)pCamera->GainRaw.GetValue());

}
*/


#endif
