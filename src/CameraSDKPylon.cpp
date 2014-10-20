/*
								CameraSDKPylon.cpp

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
 * @file    CameraSDKPylon.cpp
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    03/07/2014
 */

#include "CameraSDKPylon.h"

#ifdef USE_PYLON

CameraSDKPylon::CameraSDKPylon(){

	pTlFactory			= NULL;
	pCamera				= NULL;
	pDevice				= NULL;
	pEventGrabber		= NULL;
	pEventAdapter		= NULL;
	pStreamGrabber		= NULL;
	nbEventBuffers		= 20;

	connectionStatus = false;

}

CameraSDKPylon::~CameraSDKPylon(void){


}

bool	CameraSDKPylon::chooseDevice(int id, string name){

    bool chooseState  = false;

    try{

		if (!devices.empty()){

			if (!pDevice){

					pDevice = pTlFactory->CreateDevice(devices[id]);
					BOOST_LOG_SEV(log, normal) << "Device n° "<<id<<" created";

			}

			if(pDevice){

				if(!pDevice->IsOpen()) pDevice->Open();

				if(pDevice->IsOpen()){

						connectionStatus = true;
						chooseState = true;

					}
				}

				if (pDevice && ! pCamera){

					pCamera=new CBaslerGigECamera(pDevice,true);
					BOOST_LOG_SEV(log, normal) << "Basler GigE camera created";

				}

		}else{

			BOOST_LOG_SEV(log, fail) << "Devices list is empty";

		}

	}catch (GenICam::GenericException &e){

		BOOST_LOG_SEV(log, critical) << e.GetDescription();

    }

	if(!chooseState){

		BOOST_LOG_SEV(log, fail) << "Camera is not accesible. Can be already opened by an another process.";
		cout <<"Camera is not accesible. Can be already opened by an another process."<<endl;

	}

	return chooseState;

}

void	CameraSDKPylon::grabRestart(){

}

double	CameraSDKPylon::getExpoMin(void){

}

double	CameraSDKPylon::getExpoMax(void){

}

int		CameraSDKPylon::getGainMin(void){

}

int		CameraSDKPylon::getGainMax(void){

}

int		CameraSDKPylon::getPixelFormat(void){

    if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono8){

        return 8;

    }else if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono12){

        return 12;

    }

    return -1;

}

double	CameraSDKPylon::getFPS(void){

}

string	CameraSDKPylon::getModelName(){

}

bool	CameraSDKPylon::setExposureTime(double exposition){

    if(pCamera){

        try{
            if ( pCamera->IsAttached() && pCamera->IsOpen() ){

                pCamera->ExposureTimeAbs=exposition;

            }else{

                BOOST_LOG_SEV(log, fail) <<"Camera not opened or not attached";

            }

            return true;

        }catch (GenICam::GenericException &e){

            // Error handling
            BOOST_LOG_SEV(log, critical) <<"An exception occurred."<<e.GetDescription();
             return false;
        }
    }

    return false;

}

bool	CameraSDKPylon::setGain(int gain){

    BOOST_LOG_SEV(log, normal) << "Set gain";

	try{

		if( pCamera->IsAttached() && pCamera->IsOpen() ){

			pCamera->GainRaw = gain;

		}else{

			BOOST_LOG_SEV(log, fail) <<"Camera not opened or not attached";

		}

		return true;

	}catch (GenICam::GenericException &e){

        // Error handling
		BOOST_LOG_SEV(log, critical) <<"An exception occurred."<<e.GetDescription();
        return false;
    }

    return false;

}

bool	CameraSDKPylon::setPixelFormat(int depth){

    Basler_GigECamera::PixelFormatEnums fpix;

	if (depth == 8 ){

		fpix = PixelFormat_Mono8;

	}
	else if (depth == 12 ){

		fpix = PixelFormat_Mono12;


	}else{

		fpix = PixelFormat_Mono8;

	}

	if (pCamera){

		try{
			if( pCamera->IsAttached() && pCamera->IsOpen() ){
				BOOST_LOG_SEV(log, normal) <<"PayloadSize before : "<<pCamera->PayloadSize.ToString().c_str();

				pCamera->PixelFormat.SetValue(fpix);

				BOOST_LOG_SEV(log, normal) <<"PayloadSize after : "<<pCamera->PayloadSize.ToString().c_str();

			} else 	{
				BOOST_LOG_SEV(log, fail) <<"Camera not opened or not attached";
			}
		}
		catch (GenICam::GenericException &e){
			// Error handling
			BOOST_LOG_SEV(log, critical) <<"An exception occurred."<<e.GetDescription();
		}

		return true;
	}

	return false;

}

template <typename Container> void stringtok(Container &container, string const &in, const char * const delimiters = "#"){

    const string::size_type len = in.length();
    string::size_type i = 0;

    while (i < len){

        // Eat leading whitespace
        i = in.find_first_not_of(delimiters, i);

        if (i == string::npos)
            return;   // Nothing left but white space

        // Find the end of the token
        string::size_type j = in.find_first_of(delimiters, i);

        // Push token
        if (j == string::npos){

            container.push_back(in.substr(i));
            return;

        }else

            container.push_back(in.substr(i, j-i));

        // Set up for next loop
        i = j + 1;

    }
}

void    CameraSDKPylon::listCameras(){

	try{

		if (!pTlFactory){

			BOOST_LOG_SEV(log, notification) << "TransportLayer creation succeed";
			pTlFactory=&CTlFactory::GetInstance ();

		}

		// Exit the application if the specific transport layer is not available
		if (!pTlFactory){

			BOOST_LOG_SEV(log, notification) << "Failed to create TransportLayer";

		}

		if (!connectionStatus){

			devices.clear ();

			if (pTlFactory){

				if (0 == pTlFactory->EnumerateDevices(devices)){

					BOOST_LOG_SEV(log, notification) <<"No cameras detected...";
					cout <<"No cameras detected..."<<endl<<endl;

				}else {

					BOOST_LOG_SEV(log, notification) <<"Cameras detected...";
					cout <<"(ID) Cameras has been detected : "<<endl<<endl;

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
								stringtok(ch,devices[id].GetFullName().c_str());

								cout<< "	Adress :	"		<<ch.back()<<endl<<endl;

								BOOST_LOG_SEV(log, notification)
									<<"ID : "		<<id
									<<"   S/N : "	<<devices[id].GetSerialNumber().c_str()
									<<"   Name : "	<<devices[id].GetModelName().c_str()
									<<"   Adress : "<<ch.back();
							}
						}

						id++;
					}
				}
			}
		}

	}catch (GenICam::GenericException &e){

		BOOST_LOG_SEV(log, critical) <<"An exception occured : "<<e.GetDescription();

	}
}

int	    CameraSDKPylon::grabStart(){

	BOOST_LOG_SEV(log, normal) <<" Starting basler's grab initialization... ";

	if(pCamera){

		if(pCamera->IsOpen()){

			try{

				// Check if the device supports events.
				if (!GenApi::IsAvailable( pCamera->EventSelector)){

					throw RUNTIME_EXCEPTION( "The device doesn't support events.");

				}

				// Create the event grabber
				pEventGrabber = new 	(CBaslerGigECamera::EventGrabber_t) (pCamera->GetEventGrabber( ));

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
				BOOST_LOG_SEV(log, normal) <<" Set acquisition mode to CONTINUOUS ";
				pCamera->AcquisitionMode.SetValue(AcquisitionMode_Continuous);

				//Set exposure settings
				pCamera->ExposureMode.SetValue(ExposureMode_Timed);

				if (!pStreamGrabber){

					pStreamGrabber = new (CBaslerGigECamera::StreamGrabber_t)(pCamera->GetStreamGrabber(0));

				}

				pStreamGrabber->Open();

				// Get the image buffer size
				BOOST_LOG_SEV(log, normal) <<"Get image size : " << pCamera->PayloadSize.GetValue();
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
				BOOST_LOG_SEV(log, normal) <<"Grab initilisation terminated.";


			}catch (GenICam::GenericException &e){

				// Error handling.
				BOOST_LOG_SEV(log, critical) <<"An exception occurred."<<e.GetDescription();

			}

		}
	}

	return 0;

}

void	    CameraSDKPylon::acqStart(){

    // Start image acquisition
	pCamera->AcquisitionStart.Execute();

}

void	CameraSDKPylon::acqStop(){

    pCamera->AcquisitionStop.Execute();

}

void    CameraSDKPylon::grabStop(){

	if (pCamera){

		if (pCamera->IsOpen()){

			try{

				//if (grabStatus){

					// Flush the input queue, grabbing may have failed
					BOOST_LOG_SEV(log, normal) <<"Flush the input queue";
					pStreamGrabber->CancelGrab();

					// Consume all items from the output queue
					GrabResult Result;

					while ( pStreamGrabber->GetWaitObject().Wait(0) ) {

						pStreamGrabber->RetrieveResult( Result );

						if ( Result.Status() == Canceled )
							BOOST_LOG_SEV(log, normal) <<"Got canceled buffer";

					}

					// Deregister and free buffers
					for ( int i = 0; i < nbBuffers; ++i ) {

						pStreamGrabber->DeregisterBuffer(handles[i]);

						BOOST_LOG_SEV(log, normal) <<"Deregister and free buffer n° "<< i;

						if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono8){

							delete [] ppBuffersUC[i];

						}else if (pCamera->PixelFormat.GetValue() == PixelFormat_Mono12){

							delete [] ppBuffersUS[i];

						}
					}

					//grabStatus = false;

					// Free all resources used for grabbing
					pStreamGrabber->FinishGrab ();
					pStreamGrabber->Close();
					pEventGrabber->Close();

				//}

			}catch (GenICam::GenericException &e){

				// Error handling.
				BOOST_LOG_SEV(log, critical) <<"An exception occurred."<<e.GetDescription();

			}
		}
	}
};

int     CameraSDKPylon::getHeight(){

	int imgH = 0;

	if(pCamera){

		try{

			if(pCamera->IsAttached() && pCamera->IsOpen()){

				imgH = pCamera->Height.GetValue();

			}else{

				BOOST_LOG_SEV(log, fail) <<"Can't access height image. Camera not opened or not attached.";

			}

		}catch(GenICam::GenericException &e){

			// Error handling
			BOOST_LOG_SEV(log, critical) <<"An exception occurred."<<e.GetDescription();

		}
	}

	return imgH;

}

int     CameraSDKPylon::getWidth(){

	int imgW = 0;

	if (pCamera){

		try{

			if (pCamera->IsAttached() && pCamera->IsOpen()){

				imgW = pCamera->Width.GetValue();

			}else{

				BOOST_LOG_SEV(log, fail) <<"Can't access width image. Camera not opened or not attached.";

			}

		}catch (GenICam::GenericException &e){
            cout << "An exception occured "<<endl;
			// Error handling
			BOOST_LOG_SEV(log, critical) <<"An exception occurred."<<e.GetDescription();

		}
	}

	return imgW;

}

double  CameraSDKPylon::getExposureTime(){


     return (double)pCamera->ExposureTimeAbs.GetValue();

}

int     CameraSDKPylon::getGain(){

    return (int)((double)pCamera->GainRaw.GetValue());

}

bool    CameraSDKPylon::grabImage( Frame *&newFrame, Mat newImg ){

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
            BOOST_LOG_SEV(log, fail) << "Failed to retrieve an item from the output queue";

        }

        CGrabResultPtr ptrGrabResult;

        int payload = pCamera->PayloadSize.GetValue();

        if(result.Succeeded()){

            //Image datation
            string acquisitionDate = TimeDate::localDateTime(second_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");

            //save the grabbed image in opencv mat

            memcpy(newImg.ptr(), result.Buffer(), payload);

            double exp = (double)pCamera->ExposureTimeAbs.GetValue();

            double gain = (int)((double)pCamera->GainRaw.GetValue());

            newFrame = new Frame(newImg,gain, (int)exp, acquisitionDate);

        }else{

            res = false;
            BOOST_LOG_SEV(log, fail) << "Grab failed: " << result.GetErrorDescription();

        }

        // Requeue the buffer
        pStreamGrabber->QueueBuffer( result.Handle(), result.Context() );

    }else{

        BOOST_LOG_SEV(log, fail) <<"Timeout occurred when waiting for a grabbed image" << result.GetErrorDescription();
        res = false;
    }

    return res;

}

#endif
