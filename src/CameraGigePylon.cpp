/*
                            CameraGigePylon.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*   This file is part of:   freeture
*
*   Copyright:      (C) 2014-2015 Yoan Audureau
*                               FRIPON-GEOPS-UPSUD-CNRS
*
*   License:        GNU General Public License
*
*   FreeTure is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*   FreeTure is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*   You should have received a copy of the GNU General Public License
*   along with FreeTure. If not, see <http://www.gnu.org/licenses/>.
*
*   Last modified:      20/07/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    CameraGigePylon.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/07/2014
* \brief   Use Pylon library to pilot GigE Cameras.
*/

#include "CameraGigePylon.h"

#ifdef USE_PYLON

boost::log::sources::severity_logger< LogSeverityLevel >  CameraGigePylon::logger;

CameraGigePylon::Init CameraGigePylon::initializer;

CameraGigePylon::CameraGigePylon(){

    pTlFactory = NULL;
    pCamera = NULL;
    pDevice = NULL;
    pEventGrabber = NULL;
    pEventAdapter = NULL;
    pStreamGrabber = NULL;
    nbEventBuffers = 20;
    connectionStatus = false;
    mFrameCounter = 0;

}

CameraGigePylon::~CameraGigePylon(void){

    if(pEventAdapter != NULL) delete pEventAdapter;
    if(pEventGrabber != NULL) delete pEventGrabber;
    if(pStreamGrabber != NULL) delete pStreamGrabber;
    if(pDevice != NULL) delete pDevice;
    if(pCamera != NULL) delete pCamera;
    if(pTlFactory != NULL) delete pTlFactory;

}

bool CameraGigePylon::listCameras() {

    try {

        if(!pTlFactory) {

            pTlFactory=&CTlFactory::GetInstance();

        }

        // Exit the application if the specific transport layer is not available
        if(!pTlFactory) {
            BOOST_LOG_SEV(logger,fail) << "Fail to create TransportLayer.";
            return false;
        }

        if(!connectionStatus) {

            devices.clear();

            if(pTlFactory) {

                cout << endl << "------------ GIGE CAMERAS WITH PYLON -----------" << endl << endl;

                if (0 == pTlFactory->EnumerateDevices(devices)) {

                    cout << "-> No cameras detected..." << endl;
                    cout << endl << "------------------------------------------------" << endl << endl;
                    return false;

                }

                int id=0;

                if( !devices.empty() && !connectionStatus) {

                    DeviceInfoList_t::const_iterator it;

                    for(it = devices.begin(); it != devices.end(); ++it ) {

                        if(!devices.empty()){

                            if(devices[id].GetFullName().find_first_of("Basler")==0||devices[id].GetFullName().find_first_of("Prosilica")==0) {

                                /*list<string> ch;
                                Conversion::stringTok(ch,devices[id].GetFullName().c_str(), "#");*/

                                cout << "-> ID[" << id << "]  NAME[" << devices[id].GetModelName().c_str() << "]  S/N[" << devices[id].GetSerialNumber().c_str() <<"]"/* ADRESS[" << ch.back() << "]"*/ << endl;

                            }
                        }

                        id++;
                    }
                }
            }

            cout << endl << "------------------------------------------------" << endl << endl;

        }

        return true;

    }catch (GenICam::GenericException &e){

        BOOST_LOG_SEV(logger,fail) << "An exception occured : " << e.GetDescription() ;
        cout << "An exception occured : " << e.GetDescription() << endl;

    }
}

bool CameraGigePylon::createDevice(int id){

    bool chooseState  = false;

    try{

        if(!devices.empty()){

            if(!pDevice){

                if(id >= 0 && id < devices.size()){

                    pDevice = pTlFactory->CreateDevice(devices[id]);

                }else{

                    cout << "No camera with id : " << id << endl;

                }
            }

            if(pDevice){

                if(!pDevice->IsOpen()) pDevice->Open();

                if(pDevice->IsOpen()){

                    connectionStatus = true;
                    chooseState = true;
                    BOOST_LOG_SEV(logger,notification) << "Success to open the device.";

                }
            }

            if (pDevice && ! pCamera){

                pCamera=new CBaslerGigECamera(pDevice,true);
                BOOST_LOG_SEV(logger,notification) << "Basler GigE camera created." << endl;
            }

        }else{
            BOOST_LOG_SEV(logger,fail) << "Devices list is empty. " << endl;
        }

    }catch (GenICam::GenericException &e){

        cout << e.GetDescription() << endl;

    }

    if(!chooseState){

        cout << "Camera is not accesible. " << endl << " (You can try to disconnect and reconnect ethernet link.)" << endl;

    }

    return chooseState;

}

bool CameraGigePylon::getDeviceNameById(int id, string &device){

    pTlFactory = &CTlFactory::GetInstance();

    devices.clear();

    if(pTlFactory){

        if (0 == pTlFactory->EnumerateDevices(devices)){

            cout <<"No cameras detected..." << endl;
            return false;

        }else{

            cout << " Camera (ID:" << id << ") detected " << endl;
            cout << " Name :        " << devices[id].GetModelName().c_str() << endl;
            return true;
        }
    }else
        return false;

}

bool CameraGigePylon::grabInitialization(){

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
                pCamera->AcquisitionMode.SetValue(AcquisitionMode_Continuous);

                //Set exposure settings
                pCamera->ExposureMode.SetValue(ExposureMode_Timed);

                if (!pStreamGrabber){

                    pStreamGrabber = new (CBaslerGigECamera::StreamGrabber_t)(pCamera->GetStreamGrabber(0));

                }

                pStreamGrabber->Open();

                // Get the image buffer size
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

            }catch (GenICam::GenericException &e){

                // Error handling.
                BOOST_LOG_SEV(logger,fail) << "An exception occurred." << e.GetDescription();
                cout << "An exception occurred." << e.GetDescription() << endl;

            }
        }
    }

    return true;

}

void CameraGigePylon::grabCleanse(){

    if(pCamera){

        if(pCamera->IsOpen()){

            try{

                // Flush the input queue, grabbing may have failed
                BOOST_LOG_SEV(logger,notification) << "Flush the input queue.";

                if(pStreamGrabber != NULL) {

                    pStreamGrabber->CancelGrab();

                    // Consume all items from the output queue
                    GrabResult Result;

                        while (pStreamGrabber->GetWaitObject().Wait(0)){

                            pStreamGrabber->RetrieveResult(Result);

                            if (Result.Status() == Canceled)
                                BOOST_LOG_SEV(logger,notification) << "Got canceled buffer.";

                        }

                        // Deregister and free buffers
                        for(int i = 0; i < nbBuffers; ++i){

                            pStreamGrabber->DeregisterBuffer(handles[i]);

                            BOOST_LOG_SEV(logger,notification) << "Deregister and free buffer n° "<< i ;

                            if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono8){

                                delete [] ppBuffersUC[i];

                            }else if (pCamera->PixelFormat.GetValue() == PixelFormat_Mono12){

                                delete [] ppBuffersUS[i];

                            }
                        }

                    // Free all resources used for grabbing
                    pStreamGrabber->FinishGrab();
                    pStreamGrabber->Close();

                }

                if(pEventGrabber!=NULL)
                    pEventGrabber->Close();

            }catch (GenICam::GenericException &e){

                // Error handling.
                BOOST_LOG_SEV(logger,fail) << "An exception occurred." << e.GetDescription();
                cout << "An exception occurred." << e.GetDescription() << endl;

            }
        }

        BOOST_LOG_SEV(logger,notification) << "Close device.";
        pCamera->Close();
    }
}

void CameraGigePylon::acqStart(){

    pCamera->AcquisitionStart.Execute();

}

void CameraGigePylon::acqStop(){

    pCamera->AcquisitionStop.Execute();

}

bool CameraGigePylon::grabImage(Frame &newFrame){

    bool res = true;

    if(pStreamGrabber->GetWaitObject().Wait(3000)){

        // Get an item from the grabber's output queue
        if(!pStreamGrabber->RetrieveResult(result)){

            BOOST_LOG_SEV(logger,fail) << "Fail to retrieve an item from the output queue.";
            res = false;

        }

        CGrabResultPtr ptrGrabResult;

        if(result.Succeeded()){

            //Timestamping.
            boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();

            Mat newImg;

            if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono8){

                newImg = Mat(pCamera->Height.GetValue(), pCamera->Width.GetValue(), CV_8UC1, Scalar(0));

            }else if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono12){

                newImg = Mat(pCamera->Height.GetValue(), pCamera->Width.GetValue(), CV_16UC1, Scalar(0));

            }

            memcpy(newImg.ptr(), result.Buffer(), pCamera->PayloadSize.GetValue());

            newFrame = Frame(   newImg,
                                pCamera->GainRaw.GetValue(),
                                (double)pCamera->ExposureTimeAbs.GetValue(),
                                to_iso_extended_string(time));

            newFrame.mFps = pCamera->AcquisitionFrameRateAbs.GetValue();
            newFrame.mFrameNumber = mFrameCounter;
            mFrameCounter++;

            if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono8){

                newFrame.mBitDepth = MONO_8;
                newFrame.mSaturatedValue = 255;

            }else if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono12){

                newFrame.mBitDepth = MONO_12;
                newFrame.mSaturatedValue = 4095;

            }

        }else{

            BOOST_LOG_SEV(logger,fail) << "Fail to grab a frame : " << result.GetErrorDescription();
            res = false;

        }

        // Requeue the buffer
        pStreamGrabber->QueueBuffer( result.Handle(), result.Context() );

    }else{

        BOOST_LOG_SEV(logger,fail) <<"Fail to grab a frame (timeout) : " << result.GetErrorDescription();
        res = false;
    }

    return res;

}

bool CameraGigePylon::grabSingleImage(Frame &frame, int camID){

    try {

        pTlFactory = &CTlFactory::GetInstance();

        devices.clear();

        if(pTlFactory) {
            cout << pTlFactory->EnumerateDevices(devices) << endl;
            if (((camID + 1 ) > pTlFactory->EnumerateDevices(devices)) || camID < 0){

                throw "Camera ID not correct. Can't be found.";

            }else {

                cout << ">> Camera (ID:" << camID << ") found. " << endl;

            }
        }

        // Create an instant camera object with the correct id camera device.
        CInstantCamera camera( CTlFactory::GetInstance().CreateDevice(devices[camID].GetFullName()));

        INodeMap& nodemap = camera.GetNodeMap();

        // Open the camera for accessing the parameters.
        camera.Open();

        CIntegerPtr width(nodemap.GetNode("Width"));
        CIntegerPtr height(nodemap.GetNode("Height"));

        // Set width and height to the maximum sensor's size.
        width->SetValue(width->GetMax());
        height->SetValue(height->GetMax());

        // Set pixel format.
        // Access the PixelFormat enumeration type node.
        CEnumerationPtr pixelFormat(nodemap.GetNode("PixelFormat"));

        if(frame.mBitDepth == MONO_8) {

            if(IsAvailable(pixelFormat->GetEntryByName("Mono8"))){
                pixelFormat->FromString("Mono8");

        }else{
            cout << ">> Fail to set pixel format to MONO_8" << endl;
            return false;
        }

        }else if(frame.mBitDepth == MONO_12){

            if(IsAvailable(pixelFormat->GetEntryByName("Mono12"))){
                pixelFormat->FromString("Mono12");

            }else{
                cout << ">> Fail to set pixel format to MONO_12" << endl;
                return false;
            }

        }else{

            cout << ">> No depth specified for the frame container." << endl;
            return false;
        }

        // Set exposure.
        CIntegerPtr ExposureTimeRaw(nodemap.GetNode("ExposureTimeRaw"));

        if(ExposureTimeRaw.IsValid()) {

            if(frame.mExposure >= ExposureTimeRaw->GetMin() && frame.mExposure <= ExposureTimeRaw->GetMax()) {

                ExposureTimeRaw->SetValue(frame.mExposure);

            }else {

                ExposureTimeRaw->SetValue(ExposureTimeRaw->GetMin());
                cout << ">> Exposure has been setted with the minimum available value." << endl;
                cout <<	">> The available exposure range is [" << ExposureTimeRaw->GetMin() << "-" << ExposureTimeRaw->GetMax() << "] (us)" << endl;
            }

        }else {

            cout << ">> Fail to set exposure value." << endl;
            return false;
        }

        // Set gain.
        // Access the GainRaw integer type node. This node is available for Firewire and GigE Devices.
        CIntegerPtr gainRaw(nodemap.GetNode("GainRaw"));
        if(gainRaw.IsValid()) {

            if(frame.mGain >= gainRaw->GetMin() && frame.mGain <= gainRaw->GetMax()) {

                gainRaw->SetValue(frame.mGain);

            }else {

                gainRaw->SetValue(gainRaw->GetMin());
                cout << ">> Gain has been setted to the minimum available value." << endl;
                cout <<	">> The available gain range is [" << gainRaw->GetMin() << "-" << gainRaw->GetMax() << "]" << endl;
            }
        }

        camera.Close();

        // This smart pointer will receive the grab result data.
        CGrabResultPtr ptrGrabResult;

        cout << ">> Acquisition in progress... (Please wait)" << endl;

        int timeout = 1000 + frame.mExposure/1000;

        camera.GrabOne(timeout , ptrGrabResult);

        Mat newImg;

        // Image grabbed successfully ?
        if(ptrGrabResult->GrabSucceeded()){

            //Timestamping.
            boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
            string acqDateInMicrosec = to_iso_extended_string(time);

            frame.mDate = TimeDate::splitIsoExtendedDate(to_iso_extended_string(time));
            frame.mFps = 0;

            if(ptrGrabResult->GetPixelType()== PixelType_Mono8) {

                newImg = Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC1, Scalar(0));

            }else if(ptrGrabResult->GetPixelType()== PixelType_Mono12) {

                newImg = Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_16UC1, Scalar(0));

            }

            memcpy(newImg.ptr(), ptrGrabResult->GetBuffer(), ptrGrabResult->GetPayloadSize());

            newImg.copyTo(frame.mImg);

            return true;

        }

    }catch(GenICam::GenericException &e) {

        BOOST_LOG_SEV(logger,fail) << e.GetDescription();

    }catch(exception& e) {

        BOOST_LOG_SEV(logger,fail) << e.what();

    }catch(const char * msg) {

        cout << msg << endl;
        BOOST_LOG_SEV(logger,fail) << msg;

    }

    return false;
}

void CameraGigePylon::getExposureBounds(int &eMin, int &eMax){

    INodeMap *nodemap = pCamera->GetNodeMap();

    CIntegerPtr exposureTimeRaw(nodemap->GetNode("ExposureTimeRaw"));

    if(exposureTimeRaw.IsValid()) {

            eMin = exposureTimeRaw->GetMin();
            eMax = exposureTimeRaw->GetMax();

    }

}

void CameraGigePylon::getGainBounds(int &gMin, int &gMax){

    INodeMap *nodemap = pCamera->GetNodeMap();

    CIntegerPtr gainRaw(nodemap->GetNode("GainRaw"));

    if(gainRaw.IsValid()) {

            gMin = gainRaw->GetMin();
            gMax = gainRaw->GetMax();

    }

}

bool CameraGigePylon::getPixelFormat(CamBitDepth &format){

    if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono8){

        format = MONO_8;
        return true;

    }else if(pCamera->PixelFormat.GetValue() == PixelFormat_Mono12){

        format = MONO_12;

        return true;

    }

    return false;

}

bool CameraGigePylon::getFrameSize(int &w , int &h) {

    if(pCamera){

        try{

            if (pCamera->IsAttached() && pCamera->IsOpen()){

                w = pCamera->Width.GetValue();
                h = pCamera->Height.GetValue();

                return true;

            }else{

                BOOST_LOG_SEV(logger,fail) << "Can't access width image. Camera not opened or not attached." << endl;

            }

        }catch (GenICam::GenericException &e){

            // Error handling
            BOOST_LOG_SEV(logger,fail) << "An exception occurred." << e.GetDescription();

        }
    }

    return false;

}

bool CameraGigePylon::getFPS(double &value){

    if(pCamera!=NULL){

        value = pCamera->AcquisitionFrameRateAbs.GetValue();
        return true;
    }

    value = 0;
    return false;

}

string CameraGigePylon::getModelName(){
 return "";
}

bool CameraGigePylon::setExposureTime(double exposition){

    if(pCamera){

        try{
            if ( pCamera->IsAttached() && pCamera->IsOpen() ){

                pCamera->ExposureTimeAbs = exposition;

            }else{

                 std::cout << "Camera not opened or not attached" << endl;
            }

            return true;

        }catch (GenICam::GenericException &e){

            // Error handling
            BOOST_LOG_SEV(logger,fail) << "An exception occurred." << e.GetDescription();
            cout << endl << ">> " << e.GetDescription() << endl;
            return false;
        }
    }

    return false;

}

bool CameraGigePylon::setGain(int gain){

    if(pCamera){
        try{

            if( pCamera->IsAttached() && pCamera->IsOpen() ){

                pCamera->GainRaw = gain;

            }else{

                BOOST_LOG_SEV(logger,fail) << "Camera not opened or not attached";

            }

            return true;

        }catch (GenICam::GenericException &e){

            // Error handling
            BOOST_LOG_SEV(logger,fail) << "An exception occurred." << e.GetDescription();
            cout << endl << ">> " << e.GetDescription() << endl;
            return false;
        }
    }

    return false;

}

bool CameraGigePylon::setFPS(double fps){

    pCamera->AcquisitionFrameRateAbs = fps;
    return true;
}

bool CameraGigePylon::setPixelFormat(CamBitDepth format){

    Basler_GigECamera::PixelFormatEnums fpix;

    if(format == MONO_8 ){

        fpix = PixelFormat_Mono8;

    }
    else if (format == MONO_12 ){

        fpix = PixelFormat_Mono12;

    }

    if (pCamera){

        try{
            if(pCamera->IsAttached() && pCamera->IsOpen()){

                pCamera->PixelFormat.SetValue(fpix);

            }else{

               BOOST_LOG_SEV(logger,fail) << "Camera not opened or not attached";

            }
        }
        catch (GenICam::GenericException &e){

            // Error handling
            BOOST_LOG_SEV(logger,fail) << "An exception occurred." << e.GetDescription();
            cout << endl << ">> " << e.GetDescription() << endl;

        }

        return true;
    }

    return false;

}


double CameraGigePylon::getExposureTime(){

    if(pCamera!=0)
         return pCamera->ExposureTimeAbs.GetValue();
    else
        return 0;

}
/*
int CameraGigePylon::getGain(){

    return (int)(pCamera->GainRaw.GetValue());

}*/


#endif
