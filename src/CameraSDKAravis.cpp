/*
								CameraSDKAravis.cpp

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
*	Last modified:		22/12/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
 * @file    CameraSDKAravis.cpp
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    22/12/2014
 */

#include "CameraSDKAravis.h"

            CameraSDKAravis::CameraSDKAravis(){}

            CameraSDKAravis::~CameraSDKAravis(){}

/// ACQUISITION FUNCTIONS

void        CameraSDKAravis::listCameras(){

    arv_update_device_list ();

	unsigned int n_devices = arv_get_n_devices ();

    cout << endl;
    cout << "********** DETECTED CAMERAS WITH ARAVIS ************ " << endl;
    cout << "*" << endl;

	for(int i = 0; i< n_devices; i++){

        cout << "* ->"<< arv_get_device_id (i)<<endl;

	}

    cout << "*" << endl;
	cout << "**************************************************** " << endl << endl;

}

bool CameraSDKAravis::getDeviceById(int id, string &device){

    arv_update_device_list();

	unsigned int n_devices = arv_get_n_devices();

	for(int i = 0; i< n_devices; i++){

        if(id == i){

            device = arv_get_device_id(i);

            return true;

        }

	}

	return false;

}

bool	    CameraSDKAravis::chooseDevice(string name){

    camera = arv_camera_new(name.c_str());

    if(camera != NULL)
        return true;
    else
        return false;

}



bool		CameraSDKAravis::grabStart(){

    payload = arv_camera_get_payload (camera);

    arv_camera_get_region (camera, NULL, NULL, &width, &height);

    pixFormat = arv_camera_get_pixel_format (camera);

    pixel_format_string = arv_camera_get_pixel_format_as_string (camera);

    arv_camera_get_exposure_time_bounds (camera, &exposureMin, &exposureMax);

    arv_camera_get_gain_bounds (camera, &gainMin, &gainMax);

    arv_camera_set_frame_rate (camera, 30);

    fps = arv_camera_get_frame_rate (camera);

    caps_string = arv_pixel_format_to_gst_caps_string (pixFormat);

    if (caps_string == NULL) {

        g_message ("GStreamer cannot understand the camera pixel format: 0x%x!\n", (int) pixFormat);

    }

    gain = arv_camera_get_gain (camera);

    exp =  arv_camera_get_exposure_time (camera);

    cout << endl;

    cout << "DEVICE SELECTED : " << arv_camera_get_device_id(camera)    << endl;

    cout << "DEVICE NAME     : " << arv_camera_get_model_name(camera)   << endl;

    cout << "DEVICE VENDOR   : " << arv_camera_get_vendor_name(camera)  << endl;

    cout << "PAYLOAD         : " << payload                             << endl;

    cout << "Width           : " << width                               << endl

         << "Height          : " << height                              << endl;

    //cout << "Format          : " << pixel_format_string                 << endl;

    cout << "Exp Range       : [" << exposureMin    << " - " << exposureMax   << "]"  << endl;

    cout << "Exp             : " << exp                                 << endl;

    cout << "Gain Range      : [" << gainMin        << " - " << gainMax       << "]"  << endl;

    cout << "Gain            : " << gain                                << endl;

    cout << "Fps             : " << fps                                 << endl;

    cout << "Type            : " << caps_string                         << endl;

    const char * feature;

    string stfeature = "TemperatureAbs";

    feature                = stfeature.c_str();

   // cout << "Temperature : " << arv_device_get_string_feature_value (camera, feature);

    cout << endl;



    stream = arv_camera_create_stream(camera, NULL, NULL);

    if (stream == NULL) {

        g_object_unref(camera);
        camera = NULL;

        return false;

    }else{

        if (ARV_IS_GV_STREAM (stream)) {

            g_object_set (stream,

                        //ARV_GV_STREAM_SOCKET_BUFFER_FIXED : socket buffer is set to a given fixed value
                        //ARV_GV_STREAM_SOCKET_BUFFER_AUTO: sockect buffer is set with respect to the payload size
                        "socket-buffer", ARV_GV_STREAM_SOCKET_BUFFER_AUTO,

                        //Socket buffer size, in bytes.
                        //Allowed values: >= G_MAXULONG
                        //Default value: 0
                        "socket-buffer-size", 0,

                        //Packet timeout, in µs.
                        //Allowed values: [1000,10000000]
                        //Default value: 40000
                        "packet-timeout", (unsigned) 10000000,

                        //Packet retention, in µs.
                        //Allowed values: [1000,10000000]
                        //Default value: 200000
                        "frame-retention", (unsigned) 200000,

                        //ARV_GV_STREAM_PACKET_RESEND_NEVER: never request a packet resend
                        //ARV_GV_STREAM_PACKET_RESEND_ALWAYS: request a packet resend if a packet was missing
                        //Default value: ARV_GV_STREAM_PACKET_RESEND_ALWAYS
                        "packet-resend", ARV_GV_STREAM_PACKET_RESEND_NEVER,

                        NULL);

        }else{

            return false;

        }

        for (int i = 0; i < 50; i++)
            arv_stream_push_buffer (stream, arv_buffer_new (payload, NULL));

        return true;

    }

}

void        CameraSDKAravis::acqStart(){

    arv_camera_set_acquisition_mode(camera, ARV_ACQUISITION_MODE_CONTINUOUS);
    arv_camera_start_acquisition(camera);

}

void        CameraSDKAravis::acqStop(){

     arv_camera_stop_acquisition (camera);

}

void	    CameraSDKAravis::grabStop(){

    arv_stream_get_statistics(stream,&n_completed_buffers,&n_failures,&n_underruns);

    cout << "Completed buffers = " << (unsigned long long) n_completed_buffers<<endl;
    cout << "Failures          = " << (unsigned long long) n_failures<<endl;
    cout << "Underruns         = "<< (unsigned long long) n_underruns<<endl;

    // Free ressources
    g_object_unref (stream);
    g_object_unref (camera);

}

void	    CameraSDKAravis::grabRestart(){

}

bool	    CameraSDKAravis::grabImage(Frame *&newFrame, Mat newImage){

    ArvBuffer *arv_buffer;

    arv_buffer = arv_stream_timeout_pop_buffer(stream, 2000000); //us

    if (arv_buffer == NULL){

        return false;

    }else{

        if (arv_buffer->status == ARV_BUFFER_STATUS_SUCCESS){

            //cout << "frame id: "<< arv_buffer->frame_id <<endl;
           // cout << "timepstamp : " <<arv_buffer->timestamp_ns <<endl;

            //Timestamping.
            string acquisitionDate = TimeDate::localDateTime(second_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");

            //Get frame size.
            int width = arv_buffer->width, height = arv_buffer->height;

            Mat image;

            if(pixFormat == ARV_PIXEL_FORMAT_MONO_8){

                Mat img(height, width, CV_8UC1, arv_buffer->data);
                img.copyTo(image);

            }else{

                Mat img(height, width, CV_16UC1, arv_buffer->data);
                img.copyTo(image);

            }

            image.copyTo(newImage);
            newFrame = new Frame(image, arv_camera_get_gain(camera), arv_camera_get_exposure_time(camera), acquisitionDate);
            cout << "exp : " << arv_camera_get_exposure_time(camera) << endl;

        }else{

            switch(arv_buffer->status){

                case 1 :
                    cout << "ARV_BUFFER_STATUS_SUCCESS : the buffer contains a valid image"<<endl;
                    break;
                case 2 :
                    cout << "ARV_BUFFER_STATUS_CLEARED: the buffer is cleared"<<endl;
                    break;
                case 3 :
                    cout << "ARV_BUFFER_STATUS_TIMEOUT: timeout was reached before all packets are received"<<endl;
                    break;
                case 4 :
                    cout << "ARV_BUFFER_STATUS_MISSING_PACKETS: stream has missing packets"<<endl;
                    break;
                case 5 :
                    cout << "ARV_BUFFER_STATUS_WRONG_PACKET_ID: stream has packet with wrong id"<<endl;
                    break;
                case 6 :
                    cout << "ARV_BUFFER_STATUS_SIZE_MISMATCH: the received image didn't fit in the buffer data space"<<endl;
                    break;
                case 7 :
                    cout << "ARV_BUFFER_STATUS_FILLING: the image is currently being filled"<<endl;
                    break;
                case 8 :
                    cout << "ARV_BUFFER_STATUS_ABORTED: the filling was aborted before completion"<<endl;
                    break;

            }

            return false;
        }

        arv_stream_push_buffer(stream, arv_buffer);

     }

    return true;
}

/// GETTER FUNCTIONS

double	    CameraSDKAravis::getExpoMin(void){

    double exposureMin = 0.0;
    double exposureMax = 0.0;

    // Get bounds for the exposure.
    arv_camera_get_exposure_time_bounds(camera, &exposureMin, &exposureMax);

    return exposureMin;

}

double	    CameraSDKAravis::getExpoMax(void){

    double exposureMin = 0.0;
    double exposureMax = 0.0;

    // Get bounds for the exposure.
    arv_camera_get_exposure_time_bounds(camera, &exposureMin, &exposureMax);

    return exposureMax;

}

int		    CameraSDKAravis::getGainMin(void){

        double gainMin = 0.0;
        double gainMax = 0.0;

        // Get bounds for the gain.
        arv_camera_get_gain_bounds(camera, &gainMin, &gainMax);

        return (int)gainMin;

}

int		    CameraSDKAravis::getGainMax(void){

    double gainMin = 0.0;
    double gainMax = 0.0;

    // Get bounds for the gain.
    arv_camera_get_gain_bounds(camera, &gainMin, &gainMax);

    return (int)gainMax;

}

CamBitDepth CameraSDKAravis::getPixelFormat(void){

    ArvPixelFormat pixFormat = arv_camera_get_pixel_format(camera);
    const char *pixel_format_string;

    switch(pixFormat){

        case ARV_PIXEL_FORMAT_MONO_8 :

                return MONO_8;

            break;

        case ARV_PIXEL_FORMAT_MONO_12 :

                return MONO_12;

            break;

        default :

                return DEPTH_ERROR;

            break;

    }
}

int		    CameraSDKAravis::getWidth(void){

    int width = 0, height = 0;
    arv_camera_get_region(camera, NULL, NULL, &width, &height);
    return width;

}

int		    CameraSDKAravis::getHeight(void){

    int width = 0, height = 0;
    arv_camera_get_region(camera, NULL, NULL, &width, &height);
    return height;

}

double	    CameraSDKAravis::getFPS(void){

    return arv_camera_get_frame_rate(camera);

}

string	    CameraSDKAravis::getModelName(){

    return arv_camera_get_model_name(camera);

}

/// SETTER FUNCTIONS

bool	    CameraSDKAravis::setFPS(int fps){

    if (camera != NULL){

        arv_camera_set_frame_rate(camera, fps);

        return true;

    }

    return false;

}

bool	    CameraSDKAravis::setExposureTime(double val){

    double expMin, expMax;

    arv_camera_get_exposure_time_bounds(camera, &expMin, &expMax);

    if (camera != NULL){

        if(val >= expMin && val <= expMax)
            arv_camera_set_exposure_time(camera, val);
        else{
            cout << "> Exposure value (" << val << ") is not in range [ " << expMin << " - " << expMax << " ]" << endl;
            return false;
        }

        return true;

    }

    return false;
}

bool	    CameraSDKAravis::setGain(int val){

    double gMin, gMax;

    arv_camera_get_gain_bounds (camera, &gMin, &gMax);

    if (camera != NULL){

        if(val >= gMin && val <= gMax)
            arv_camera_set_gain (camera, val);
        else{
            cout << "> Gain value (" << val << ") is not in range [ " << gMin << " - " << gMax << " ]" << endl;
            return false;
        }

        return true;

    }

    return false;

}

bool	    CameraSDKAravis::setPixelFormat(CamBitDepth depth){

    if (camera != NULL){

        switch(depth){

            case MONO_8 :

                arv_camera_set_pixel_format(camera, ARV_PIXEL_FORMAT_MONO_8);

                break;

            case MONO_12 :

                arv_camera_set_pixel_format(camera, ARV_PIXEL_FORMAT_MONO_12);

                break;

        }

        return true;
    }

    return false;

}
