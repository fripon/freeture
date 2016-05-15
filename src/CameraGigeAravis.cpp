/*
                        CameraGigeAravis.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*   This file is part of:   freeture
*
*   Copyright:      (C) 2014-2016 Yoan Audureau
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
*   Last modified:      16/05/2016
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    CameraGigeAravis.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    16/05/2016
* \brief   Use Aravis library to pilot GigE Cameras.
*          https://wiki.gnome.org/action/show/Projects/Aravis?action=show&redirect=Aravis
*/

#include "CameraGigeAravis.h"

#ifdef LINUX

    boost::log::sources::severity_logger< LogSeverityLevel >  CameraGigeAravis::logger;
    CameraGigeAravis::Init CameraGigeAravis::initializer;

    CameraGigeAravis::CameraGigeAravis(bool shift):
    camera(NULL), mWidth(0), mHeight(0), fps(0), gainMin(0.0), gainMax(0.0),
    payload(0), exposureMin(0), exposureMax(0), gain(0), exp(0), nbCompletedBuffers(0),
    nbFailures(0), nbUnderruns(0), frameCounter(0), shiftBitsImage(shift), stream(NULL) {
        mExposureAvailable = true;
        mGainAvailable = true;
        mInputDeviceType = CAMERA;
    }

    CameraGigeAravis::CameraGigeAravis():
    camera(NULL), mWidth(0), mHeight(0), fps(0), gainMin(0.0), gainMax(0.0),
    payload(0), exposureMin(0), exposureMax(0), gain(0), exp(0), nbCompletedBuffers(0),
    nbFailures(0), nbUnderruns(0), frameCounter(0), shiftBitsImage(false), stream(NULL) {
        mExposureAvailable = true;
        mGainAvailable = true;
        mInputDeviceType = CAMERA;
    }

    CameraGigeAravis::~CameraGigeAravis(){

        if(stream != NULL)
            g_object_unref(stream);

        if(camera != NULL)
            g_object_unref(camera);

    }

    vector<pair<int,string>> CameraGigeAravis::getCamerasList() {

        vector<pair<int,string>> camerasList;

        ArvInterface *interface;

        //arv_update_device_list();

        int ni = arv_get_n_interfaces();


        for (int j = 0; j< ni; j++){

            const char* name = arv_get_interface_id (j);
            if (strcmp(name,"GigEVision") == 0) {
                interface = arv_gv_interface_get_instance();
                arv_interface_update_device_list(interface);
                //int nb = arv_get_n_devices();

                int nb = arv_interface_get_n_devices(interface);

                for(int i = 0; i < nb; i++){

                    pair<int,string> c;
                    c.first = i;
                    //const char* str = arv_get_device_id(i);
                    const char* str = arv_interface_get_device_id(interface,i);
                    const char* addr = arv_interface_get_device_address(interface,i);
                    std::string s = str;
                    c.second = "NAME[" + s + "] SDK[ARAVIS] IP: " + addr;
                    camerasList.push_back(c);
                }
            }
        }

       return camerasList;

    }

    bool CameraGigeAravis::listCameras(){

        ArvInterface *interface;
        //arv_update_device_list();

        int ni = arv_get_n_interfaces ();

        cout << endl << "------------ GIGE CAMERAS WITH ARAVIS ----------" << endl << endl;

        for (int j = 0; j< ni; j++){

            interface = arv_gv_interface_get_instance();
            arv_interface_update_device_list(interface);
            //int nb = arv_get_n_devices();

            int nb = arv_interface_get_n_devices(interface);
            for(int i = 0; i < nb; i++){

                cout << "-> [" << i << "] " << arv_interface_get_device_id(interface,i)<< endl;
                //cout << "-> [" << i << "] " << arv_get_device_id(i)<< endl;

            }

            if(nb == 0) {
                cout << "-> No cameras detected..." << endl;
                return false;
            }
        }
        cout << endl << "------------------------------------------------" << endl << endl;

        return true;

    }

    bool CameraGigeAravis::createDevice(int id){

        string deviceName;

        if(!getDeviceNameById(id, deviceName))
            return false;

        camera = arv_camera_new(deviceName.c_str());

        if(camera == NULL){

            BOOST_LOG_SEV(logger, fail) << "Fail to connect the camera.";
            return false;

        }

        return true;
    }

    bool CameraGigeAravis::setSize(int width, int height, bool customSize) {

        if(customSize) {

            arv_camera_set_region(camera, 0, 0,width,height);
            arv_camera_get_region (camera, NULL, NULL, &mWidth, &mHeight);
            BOOST_LOG_SEV(logger, notification) << "Camera region size : " << mWidth << "x" << mHeight;

        // Default is maximum size
        }else {

            int sensor_width, sensor_height;

            arv_camera_get_sensor_size(camera, &sensor_width, &sensor_height);
            BOOST_LOG_SEV(logger, notification) << "Camera sensor size : " << sensor_width << "x" << sensor_height;

            arv_camera_set_region(camera, 0, 0,sensor_width,sensor_height);
            arv_camera_get_region (camera, NULL, NULL, &mWidth, &mHeight);

        }

        return true;

    }

    bool CameraGigeAravis::getDeviceNameById(int id, string &device){

        arv_update_device_list();

        int n_devices = arv_get_n_devices();

        for(int i = 0; i< n_devices; i++){

            if(id == i){

                device = arv_get_device_id(i);
                return true;

            }
        }

        BOOST_LOG_SEV(logger, fail) << "Fail to retrieve camera with this ID.";
        return false;

    }

    bool CameraGigeAravis::grabInitialization(){

        frameCounter = 0;

        payload = arv_camera_get_payload (camera);
        BOOST_LOG_SEV(logger, notification) << "Camera payload : " << payload;

        pixFormat = arv_camera_get_pixel_format(camera);

        arv_camera_get_exposure_time_bounds (camera, &exposureMin, &exposureMax);
        BOOST_LOG_SEV(logger, notification) << "Camera exposure bound min : " << exposureMin;
        BOOST_LOG_SEV(logger, notification) << "Camera exposure bound max : " << exposureMax;

        arv_camera_get_gain_bounds (camera, &gainMin, &gainMax);
        BOOST_LOG_SEV(logger, notification) << "Camera gain bound min : " << gainMin;
        BOOST_LOG_SEV(logger, notification) << "Camera gain bound max : " << gainMax;

        arv_camera_set_frame_rate(camera, 30);

        fps = arv_camera_get_frame_rate(camera);
        BOOST_LOG_SEV(logger, notification) << "Camera frame rate : " << fps;

        capsString = arv_pixel_format_to_gst_caps_string(pixFormat);
        BOOST_LOG_SEV(logger, notification) << "Camera format : " << capsString;

        gain = arv_camera_get_gain(camera);
        BOOST_LOG_SEV(logger, notification) << "Camera gain : " << gain;

        exp = arv_camera_get_exposure_time(camera);
        BOOST_LOG_SEV(logger, notification) << "Camera exposure : " << exp;

        cout << endl;

        cout << "DEVICE SELECTED : " << arv_camera_get_device_id(camera)    << endl;
        cout << "DEVICE NAME     : " << arv_camera_get_model_name(camera)   << endl;
        cout << "DEVICE VENDOR   : " << arv_camera_get_vendor_name(camera)  << endl;
        cout << "PAYLOAD         : " << payload                             << endl;
        cout << "Width           : " << mWidth                               << endl
             << "Height          : " << mHeight                              << endl;
        cout << "Exp Range       : [" << exposureMin    << " - " << exposureMax   << "]"  << endl;
        cout << "Exp             : " << exp                                 << endl;
        cout << "Gain Range      : [" << gainMin        << " - " << gainMax       << "]"  << endl;
        cout << "Gain            : " << gain                                << endl;
        cout << "Fps             : " << fps                                 << endl;
        cout << "Type            : " << capsString                         << endl;

        cout << endl;

        // Create a new stream object. Open stream on Camera.
        stream = arv_camera_create_stream(camera, NULL, NULL);

        if(stream == NULL){

            BOOST_LOG_SEV(logger, critical) << "Fail to create stream with arv_camera_create_stream()";
            return false;

        }

        if (ARV_IS_GV_STREAM(stream)){

            bool            arv_option_auto_socket_buffer   = true;
            bool            arv_option_no_packet_resend     = true;
            unsigned int    arv_option_packet_timeout       = 20;
            unsigned int    arv_option_frame_retention      = 100;

            if(arv_option_auto_socket_buffer){

                g_object_set(stream,
                            // ARV_GV_STREAM_SOCKET_BUFFER_FIXED : socket buffer is set to a given fixed value.
                            // ARV_GV_STREAM_SOCKET_BUFFER_AUTO: socket buffer is set with respect to the payload size.
                            "socket-buffer", ARV_GV_STREAM_SOCKET_BUFFER_AUTO,
                            // Socket buffer size, in bytes.
                            // Allowed values: >= G_MAXULONG
                            // Default value: 0
                            "socket-buffer-size", 0, NULL);

            }

            if(arv_option_no_packet_resend){

                // # packet-resend : Enables or disables the packet resend mechanism

                // If packet resend is disabled and a packet has been lost during transmission,
                // the grab result for the returned buffer holding the image will indicate that
                // the grab failed and the image will be incomplete.
                //
                // If packet resend is enabled and a packet has been lost during transmission,
                // a request is sent to the camera. If the camera still has the packet in its
                // buffer, it will resend the packet. If there are several lost packets in a
                // row, the resend requests will be combined.

                g_object_set(stream,
                            // ARV_GV_STREAM_PACKET_RESEND_NEVER: never request a packet resend
                            // ARV_GV_STREAM_PACKET_RESEND_ALWAYS: request a packet resend if a packet was missing
                            // Default value: ARV_GV_STREAM_PACKET_RESEND_ALWAYS
                            "packet-resend", ARV_GV_STREAM_PACKET_RESEND_NEVER, NULL);

            }

            g_object_set(stream,
                        // # packet-timeout

                        // The Packet Timeout parameter defines how long (in milliseconds) we will wait for
                        // the next expected packet before it initiates a resend request.

                        // Packet timeout, in µs.
                        // Allowed values: [1000,10000000]
                        // Default value: 40000
                        "packet-timeout",/* (unsigned) arv_option_packet_timeout * 1000*/(unsigned)40000,
                        // # frame-retention

                        // The Frame Retention parameter sets the timeout (in milliseconds) for the
                        // frame retention timer. Whenever detection of the leader is made for a frame,
                        // the frame retention timer starts. The timer resets after each packet in the
                        // frame is received and will timeout after the last packet is received. If the
                        // timer times out at any time before the last packet is received, the buffer for
                        // the frame will be released and will be indicated as an unsuccessful grab.

                        // Packet retention, in µs.
                        // Allowed values: [1000,10000000]
                        // Default value: 200000
                        "frame-retention", /*(unsigned) arv_option_frame_retention * 1000*/(unsigned) 200000,NULL);

        }else
            return false;

        // Push 50 buffer in the stream input buffer queue.
        for (int i = 0; i < 50; i++)
            arv_stream_push_buffer(stream, arv_buffer_new(payload, NULL));

        return true;

    }

    void CameraGigeAravis::grabCleanse(){}

    bool CameraGigeAravis::acqStart(){

        BOOST_LOG_SEV(logger, notification) << "Set camera to CONTINUOUS MODE";
        arv_camera_set_acquisition_mode(camera, ARV_ACQUISITION_MODE_CONTINUOUS);

        BOOST_LOG_SEV(logger, notification) << "Set camera TriggerMode to Off";
        arv_device_set_string_feature_value(arv_camera_get_device (camera), "TriggerMode" , "Off");

        BOOST_LOG_SEV(logger, notification) << "Start acquisition on camera";
        arv_camera_start_acquisition(camera);

        return true;

    }

    void CameraGigeAravis::acqStop(){

        arv_stream_get_statistics(stream, &nbCompletedBuffers, &nbFailures, &nbUnderruns);

        //cout << "Completed buffers = " << (unsigned long long) nbCompletedBuffers   << endl;
        //cout << "Failures          = " << (unsigned long long) nbFailures           << endl;
        //cout << "Underruns         = " << (unsigned long long) nbUnderruns          << endl;

        BOOST_LOG_SEV(logger, notification) << "Completed buffers = " << (unsigned long long) nbCompletedBuffers;
        BOOST_LOG_SEV(logger, notification) << "Failures          = " << (unsigned long long) nbFailures;
        BOOST_LOG_SEV(logger, notification) << "Underruns         = " << (unsigned long long) nbUnderruns;

        BOOST_LOG_SEV(logger, notification) << "Stopping acquisition...";
        arv_camera_stop_acquisition(camera);
        BOOST_LOG_SEV(logger, notification) << "Acquisition stopped.";

        BOOST_LOG_SEV(logger, notification) << "Unreferencing stream.";
        g_object_unref(stream);
        stream = NULL;

        BOOST_LOG_SEV(logger, notification) << "Unreferencing camera.";
        g_object_unref(camera);
        camera = NULL;

    }

    bool CameraGigeAravis::grabImage(Frame &newFrame){

        ArvBuffer *arv_buffer;
        //exp = arv_camera_get_exposure_time(camera);

        arv_buffer = arv_stream_timeout_pop_buffer(stream,2000000); //us
        char *buffer_data;
        size_t buffer_size;

        if(arv_buffer == NULL){

            throw runtime_error("arv_buffer is NULL");
            return false;

        }else{

            try{

                if(arv_buffer_get_status(arv_buffer) == ARV_BUFFER_STATUS_SUCCESS){

                    //BOOST_LOG_SEV(logger, normal) << "Success to grab a frame.";

                    buffer_data = (char *) arv_buffer_get_data (arv_buffer, &buffer_size);

                    //Timestamping.
                    //string acquisitionDate = TimeDate::localDateTime(microsec_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");
                    //BOOST_LOG_SEV(logger, normal) << "Date : " << acquisitionDate;
                    boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
                    string acquisitionDate = to_iso_extended_string(time);
                    //BOOST_LOG_SEV(logger, normal) << "Date : " << acqDateInMicrosec;

                    Mat image;
                    CamPixFmt imgDepth = MONO8;
                    int saturateVal = 0;

                    if(pixFormat == ARV_PIXEL_FORMAT_MONO_8){

                        //BOOST_LOG_SEV(logger, normal) << "Creating Mat 8 bits ...";
                        image = Mat(mHeight, mWidth, CV_8UC1, buffer_data);
                        imgDepth = MONO8;
                        saturateVal = 255;

                    }else if(pixFormat == ARV_PIXEL_FORMAT_MONO_12){

                        //BOOST_LOG_SEV(logger, normal) << "Creating Mat 16 bits ...";
                        image = Mat(mHeight, mWidth, CV_16UC1, buffer_data);
                        imgDepth = MONO12;
                        saturateVal = 4095;

                        //double t3 = (double)getTickCount();

                        if(shiftBitsImage){

                            //BOOST_LOG_SEV(logger, normal) << "Shifting bits ...";


                                unsigned short * p;

                                for(int i = 0; i < image.rows; i++){
                                    p = image.ptr<unsigned short>(i);
                                    for(int j = 0; j < image.cols; j++)
                                        p[j] = p[j] >> 4;
                                }

                            //BOOST_LOG_SEV(logger, normal) << "Bits shifted.";

                        }

                        //t3 = (((double)getTickCount() - t3)/getTickFrequency())*1000;
                        //cout << "> Time shift : " << t3 << endl;
                    }

                    //BOOST_LOG_SEV(logger, normal) << "Creating frame object ...";
                    newFrame = Frame(image, gain, exp, acquisitionDate);
                    //BOOST_LOG_SEV(logger, normal) << "Setting date of frame ...";
                    //newFrame.setAcqDateMicro(acqDateInMicrosec);
                    //BOOST_LOG_SEV(logger, normal) << "Setting fps of frame ...";
                    newFrame.mFps = fps;
                    newFrame.mFormat = imgDepth;
                    //BOOST_LOG_SEV(logger, normal) << "Setting saturated value of frame ...";
                    newFrame.mSaturatedValue = saturateVal;
                    newFrame.mFrameNumber = frameCounter;
                    frameCounter++;

                    //BOOST_LOG_SEV(logger, normal) << "Re-pushing arv buffer in stream ...";
                    arv_stream_push_buffer(stream, arv_buffer);

                    return true;

                }else{

                    switch(arv_buffer_get_status(arv_buffer)){

                        case 0 :
                            cout << "ARV_BUFFER_STATUS_SUCCESS : the buffer contains a valid image"<<endl;
                            break;
                        case 1 :
                            cout << "ARV_BUFFER_STATUS_CLEARED: the buffer is cleared"<<endl;
                            break;
                        case 2 :
                            cout << "ARV_BUFFER_STATUS_TIMEOUT: timeout was reached before all packets are received"<<endl;
                            break;
                        case 3 :
                            cout << "ARV_BUFFER_STATUS_MISSING_PACKETS: stream has missing packets"<<endl;
                            break;
                        case 4 :
                            cout << "ARV_BUFFER_STATUS_WRONG_PACKET_ID: stream has packet with wrong id"<<endl;
                            break;
                        case 5 :
                            cout << "ARV_BUFFER_STATUS_SIZE_MISMATCH: the received image didn't fit in the buffer data space"<<endl;
                            break;
                        case 6 :
                            cout << "ARV_BUFFER_STATUS_FILLING: the image is currently being filled"<<endl;
                            break;
                        case 7 :
                            cout << "ARV_BUFFER_STATUS_ABORTED: the filling was aborted before completion"<<endl;
                            break;

                    }

                    arv_stream_push_buffer(stream, arv_buffer);

                    return false;
                }

            }catch(exception& e){

                cout << e.what() << endl;
                BOOST_LOG_SEV(logger, critical) << e.what() ;
                return false;

            }
        }
    }


    bool CameraGigeAravis::grabSingleImage(Frame &frame, int camID){

        bool res = false;

        if(!createDevice(camID))
            return false;

        if(!setPixelFormat(frame.mFormat))
            return false;

        if(!setExposureTime(frame.mExposure))
            return false;

        if(!setGain(frame.mGain))
            return false;

        if(frame.mWidth > 0 && frame.mHeight > 0) {

            arv_camera_set_region(camera, 0, 0,frame.mWidth,frame.mHeight);
            arv_camera_get_region (camera, NULL, NULL, &mWidth, &mHeight);

        }else{

            int sensor_width, sensor_height;

            arv_camera_get_sensor_size(camera, &sensor_width, &sensor_height);

            // Use maximum sensor size.
            arv_camera_set_region(camera, 0, 0,sensor_width,sensor_height);
            arv_camera_get_region (camera, NULL, NULL, &mWidth, &mHeight);

        }

        payload = arv_camera_get_payload (camera);

        pixFormat = arv_camera_get_pixel_format (camera);

        arv_camera_get_exposure_time_bounds (camera, &exposureMin, &exposureMax);

        arv_camera_get_gain_bounds (camera, &gainMin, &gainMax);

        arv_camera_set_frame_rate(camera, 1);

        fps = arv_camera_get_frame_rate(camera);

        capsString = arv_pixel_format_to_gst_caps_string(pixFormat);

        gain    = arv_camera_get_gain(camera);
        exp     = arv_camera_get_exposure_time(camera);

        cout << endl;

        cout << "DEVICE SELECTED : " << arv_camera_get_device_id(camera)    << endl;
        cout << "DEVICE NAME     : " << arv_camera_get_model_name(camera)   << endl;
        cout << "DEVICE VENDOR   : " << arv_camera_get_vendor_name(camera)  << endl;
        cout << "PAYLOAD         : " << payload                             << endl;
        cout << "Width           : " << mWidth                               << endl
             << "Height          : " << mHeight                              << endl;
        cout << "Exp Range       : [" << exposureMin    << " - " << exposureMax   << "]"  << endl;
        cout << "Exp             : " << exp                                 << endl;
        cout << "Gain Range      : [" << gainMin        << " - " << gainMax       << "]"  << endl;
        cout << "Gain            : " << gain                                << endl;
        cout << "Fps             : " << fps                                 << endl;
        cout << "Type            : " << capsString                         << endl;

        cout << endl;

        if(arv_camera_is_gv_device (camera)) {

            // http://www.baslerweb.com/media/documents/AW00064902000%20Control%20Packet%20Timing%20With%20Delays.pdf
            // https://github.com/GNOME/aravis/blob/06ac777fc6d98783680340f1c3f3ea39d2780974/src/arvcamera.c

            // Configure the inter packet delay to insert between each packet for the current stream
            // channel. This can be used as a crude flow-control mechanism if the application or the network
            // infrastructure cannot keep up with the packets coming from the device.
            arv_camera_gv_set_packet_delay (camera, 4000);

            // Specifies the stream packet size, in bytes, to send on the selected channel for a GVSP transmitter
            // or specifies the maximum packet size supported by a GVSP receiver.
            arv_camera_gv_set_packet_size (camera, 1444);

        }

        // Create a new stream object. Open stream on Camera.
        stream = arv_camera_create_stream(camera, NULL, NULL);

        if(stream != NULL){

            if(ARV_IS_GV_STREAM(stream)){

                bool            arv_option_auto_socket_buffer   = true;
                bool            arv_option_no_packet_resend     = true;
                unsigned int    arv_option_packet_timeout       = 20;
                unsigned int    arv_option_frame_retention      = 100;

                if(arv_option_auto_socket_buffer){

                    g_object_set(stream, "socket-buffer", ARV_GV_STREAM_SOCKET_BUFFER_AUTO, "socket-buffer-size", 0, NULL);

                }

                if(arv_option_no_packet_resend){

                    g_object_set(stream, "packet-resend", ARV_GV_STREAM_PACKET_RESEND_NEVER, NULL);

                }

                g_object_set(stream, "packet-timeout", (unsigned)40000, "frame-retention", (unsigned) 200000,NULL);

            }

            // Push 50 buffer in the stream input buffer queue.
            arv_stream_push_buffer(stream, arv_buffer_new(payload, NULL));

            // Set acquisition mode to continuous.
            arv_camera_set_acquisition_mode(camera, ARV_ACQUISITION_MODE_SINGLE_FRAME);

            // Very usefull to avoid arv buffer timeout status
            sleep(1);

            // Start acquisition.
            arv_camera_start_acquisition(camera);

            // Get image buffer.
            ArvBuffer *arv_buffer = arv_stream_timeout_pop_buffer(stream, frame.mExposure + 5000000); //us

            char *buffer_data;
            size_t buffer_size;

            cout << ">> Acquisition in progress... (Please wait)" << endl;

            if (arv_buffer != NULL){

                if(arv_buffer_get_status(arv_buffer) == ARV_BUFFER_STATUS_SUCCESS){

                    buffer_data = (char *) arv_buffer_get_data (arv_buffer, &buffer_size);

                    //Timestamping.
                    boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();

                    if(pixFormat == ARV_PIXEL_FORMAT_MONO_8){

                        Mat image = Mat(mHeight, mWidth, CV_8UC1, buffer_data);
                        image.copyTo(frame.mImg);

                    }else if(pixFormat == ARV_PIXEL_FORMAT_MONO_12){

                        // Unsigned short image.
                        Mat image = Mat(mHeight, mWidth, CV_16UC1, buffer_data);

                        // http://www.theimagingsource.com/en_US/support/documentation/icimagingcontrol-class/PixelformatY16.htm
                        // Some sensors only support 10-bit or 12-bit pixel data. In this case, the least significant bits are don't-care values.
                        if(shiftBitsImage){
                            unsigned short * p;
                            for(int i = 0; i < image.rows; i++){
                                p = image.ptr<unsigned short>(i);
                                for(int j = 0; j < image.cols; j++) p[j] = p[j] >> 4;
                            }
                        }

                        image.copyTo(frame.mImg);

                    }

                    frame.mDate = TimeDate::splitIsoExtendedDate(to_iso_extended_string(time));
                    frame.mFps = arv_camera_get_frame_rate(camera);

                    res = true;

                }else{

                    switch(arv_buffer_get_status(arv_buffer)){

                        case 0 :

                            cout << "ARV_BUFFER_STATUS_SUCCESS : the buffer contains a valid image"<<endl;

                            break;

                        case 1 :

                            cout << "ARV_BUFFER_STATUS_CLEARED: the buffer is cleared"<<endl;

                            break;

                        case 2 :

                            cout << "ARV_BUFFER_STATUS_TIMEOUT: timeout was reached before all packets are received"<<endl;

                            break;

                        case 3 :

                            cout << "ARV_BUFFER_STATUS_MISSING_PACKETS: stream has missing packets"<<endl;

                            break;

                        case 4 :

                            cout << "ARV_BUFFER_STATUS_WRONG_PACKET_ID: stream has packet with wrong id"<<endl;

                            break;

                        case 5 :

                            cout << "ARV_BUFFER_STATUS_SIZE_MISMATCH: the received image didn't fit in the buffer data space"<<endl;

                            break;

                        case 6 :

                            cout << "ARV_BUFFER_STATUS_FILLING: the image is currently being filled"<<endl;

                            break;

                        case 7 :

                            cout << "ARV_BUFFER_STATUS_ABORTED: the filling was aborted before completion"<<endl;

                            break;


                    }

                    res = false;

                }

                arv_stream_push_buffer(stream, arv_buffer);

           }else{

                BOOST_LOG_SEV(logger, fail) << "Fail to pop buffer from stream.";
                res = false;

           }

            arv_stream_get_statistics(stream, &nbCompletedBuffers, &nbFailures, &nbUnderruns);

            cout << ">> Completed buffers = " << (unsigned long long) nbCompletedBuffers	<< endl;
            cout << ">> Failures          = " << (unsigned long long) nbFailures           << endl;
            //cout << ">> Underruns         = " << (unsigned long long) nbUnderruns          << endl;

            // Stop acquisition.
            arv_camera_stop_acquisition(camera);

            g_object_unref(stream);
            stream = NULL;
            g_object_unref(camera);
            camera = NULL;

        }

        return res;

    }

    void CameraGigeAravis::saveGenicamXml(string p){

        const char *xml;

        size_t size;

        xml = arv_device_get_genicam_xml (arv_camera_get_device(camera), &size);

        if (xml != NULL){

            ofstream infFile;
            string infFilePath = p + "genicam.xml";
            infFile.open(infFilePath.c_str());
            infFile << string ( xml, size );
            infFile.close();

        }

    }

    //https://github.com/GNOME/aravis/blob/b808d34691a18e51eee72d8cac6cfa522a945433/src/arvtool.c
    void CameraGigeAravis::getAvailablePixelFormats() {

        ArvGc *genicam;
        ArvDevice *device;
        ArvGcNode *node;

        if(camera != NULL) {

            device = arv_camera_get_device(camera);
            genicam = arv_device_get_genicam(device);
            node = arv_gc_get_node(genicam, "PixelFormat");

            if (ARV_IS_GC_ENUMERATION (node)) {

                const GSList *childs;
                const GSList *iter;
                vector<string> pixfmt;

                cout << ">> Device pixel formats :" << endl;

                childs = arv_gc_enumeration_get_entries (ARV_GC_ENUMERATION (node));
                for (iter = childs; iter != NULL; iter = iter->next) {
                    if (arv_gc_feature_node_is_implemented (ARV_GC_FEATURE_NODE (iter->data), NULL)) {

                        if(arv_gc_feature_node_is_available (ARV_GC_FEATURE_NODE (iter->data), NULL)) {

                            {
                                string fmt = string(arv_gc_feature_node_get_name(ARV_GC_FEATURE_NODE (iter->data)));
                                std::transform(fmt.begin(), fmt.end(),fmt.begin(), ::toupper);
                                pixfmt.push_back(fmt);
                                cout << "- " << fmt << endl;

                            }
                        }
                    }
                }

                // Compare found pixel formats to currently formats supported by freeture

                cout << endl <<  ">> Available pixel formats :" << endl;
                EParser<CamPixFmt> fmt;

                for( int i = 0; i != pixfmt.size(); i++ ) {

                    if(fmt.isEnumValue(pixfmt.at(i))) {

                        cout << "- " << pixfmt.at(i) << " available --> ID : " << fmt.parseEnum(pixfmt.at(i)) << endl;

                    }

                }

            }else {

                cout << ">> Available pixel formats not found." << endl;

            }

            g_object_unref(device);

        }

    }

    void CameraGigeAravis::getExposureBounds(double &eMin, double &eMax){

        double exposureMin = 0.0;
        double exposureMax = 0.0;

        arv_camera_get_exposure_time_bounds(camera, &exposureMin, &exposureMax);

        eMin = exposureMin;
        eMax = exposureMax;

    }

    double CameraGigeAravis::getExposureTime(){

        return arv_camera_get_exposure_time(camera);

    }

    void CameraGigeAravis::getGainBounds(int &gMin, int &gMax){

        double gainMin = 0.0;
        double gainMax = 0.0;

        arv_camera_get_gain_bounds(camera, &gainMin, &gainMax);

        gMin = gainMin;
        gMax = gainMax;

    }

    bool CameraGigeAravis::getPixelFormat(CamPixFmt &format){

        ArvPixelFormat pixFormat = arv_camera_get_pixel_format(camera);

        switch(pixFormat){

            case ARV_PIXEL_FORMAT_MONO_8 :

                format = MONO8;

                break;

            case ARV_PIXEL_FORMAT_MONO_12 :

                format = MONO12;

                break;

            default :

                return false;

                break;

        }

        return true;
    }


    bool CameraGigeAravis::getFrameSize(int &w, int &h) {

        if(camera != NULL) {

            int ww = 0, hh = 0;
            arv_camera_get_region(camera, NULL, NULL, &ww, &h);
            w = ww;
            h = hh;

        }

        return false;

    }

    bool CameraGigeAravis::getFPS(double &value){

        if(camera != NULL) {

            value = arv_camera_get_frame_rate(camera);
            return true;

        }

        return false;

    }

    string CameraGigeAravis::getModelName(){

        return arv_camera_get_model_name(camera);

    }

    bool CameraGigeAravis::setExposureTime(double val){

        double expMin, expMax;

        arv_camera_get_exposure_time_bounds(camera, &expMin, &expMax);

        if(camera != NULL){

            if(val >= expMin && val <= expMax) {

                exp = val;
                arv_camera_set_exposure_time(camera, val);

            }else{

                cout << "> Exposure value (" << val << ") is not in range [ " << expMin << " - " << expMax << " ]" << endl;
                return false;

            }

            return true;

        }

        return false;
    }

    bool CameraGigeAravis::setGain(int val){

        double gMin, gMax;

        arv_camera_get_gain_bounds (camera, &gMin, &gMax);

        if (camera != NULL){

            if((double)val >= gMin && (double)val <= gMax){

                gain = val;
                arv_camera_set_gain (camera, (double)val);

            }else{

                cout << "> Gain value (" << val << ") is not in range [ " << gMin << " - " << gMax << " ]" << endl;
                BOOST_LOG_SEV(logger, fail) << "> Gain value (" << val << ") is not in range [ " << gMin << " - " << gMax << " ]";
                return false;

            }

        return true;

        }

        return false;

    }

    bool CameraGigeAravis::setFPS(double fps){

        if (camera != NULL){

            arv_camera_set_frame_rate(camera, fps);

            return true;

        }

        return false;

    }

    bool CameraGigeAravis::setPixelFormat(CamPixFmt depth){

        if (camera != NULL){

            switch(depth){

                case MONO8 :
                    {
                        arv_camera_set_pixel_format(camera, ARV_PIXEL_FORMAT_MONO_8);
                    }
                    break;

                case MONO12 :
                    {
                        arv_camera_set_pixel_format(camera, ARV_PIXEL_FORMAT_MONO_12);
                    }
                    break;

            }

            return true;
        }

        return false;

    }

#endif
