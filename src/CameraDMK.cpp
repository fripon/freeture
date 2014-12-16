/*
								CameraAravis.cpp

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
 * @file    CameraAravis.cpp
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    13/06/2014
 */

#include "CameraDMK.h"
//#include "arv.h"
//#include "arvinterface.h"
//#include "arvconfig.h"
//#include "Fits2D.h"
#include "CameraSDKAravis.h"

CameraDMK::CameraDMK(){

    m_thread = NULL;
    camera = new CameraSDKAravis();

}

CameraDMK::CameraDMK(string cam,
                     int format,
                     Fifo<Frame> *queue,
                     boost::mutex *m_mutex_queue,
					 boost::condition_variable *m_cond_queue_fill,
					 boost::condition_variable *m_cond_queue_new_element,
					 boost::mutex *m_mutex_thread_terminated, bool *stop){

    cameraFormat        = format;
    cameraName          = cam;
	m_thread			= NULL;
	framesQueue			= queue;
	mutexQueue			= m_mutex_queue;
	mustStopMutex       = m_mutex_thread_terminated;
	mustStop			= stop;
	condQueueFill		= m_cond_queue_fill;
	condQueueNewElement	= m_cond_queue_new_element;

	camera              = new CameraSDKAravis();

}

CameraDMK::~CameraDMK(){


    if (camera!=NULL) delete camera;

	if (m_thread!=NULL) delete m_thread;

}

void    CameraDMK::join(){

	m_thread->join();

}

void     CameraDMK::startThread(){

    camera->grabStart();

	// Launch acquisition thread
	m_thread = new boost::thread(boost::ref(*this));
    cout << "t"<<endl;

}

// Stop the thread
void    CameraDMK::stopThread(){

	// Signal the thread to stop (thread-safe)
	mustStopMutex->lock();
	*mustStop = true;
	mustStopMutex->unlock();



    cout << "DMK Thread must stop" <<endl;


	// Wait for the thread to finish.
	if (m_thread!=NULL) m_thread->join();

	cout << "DMK Thread stopped"<<endl;

}

int		CameraDMK::getCameraWidth(){

    camera->getWidth();
}

int		CameraDMK::getCameraHeight(){

    camera->getHeight();
}

void	CameraDMK::getListCameras(){

    camera->listCameras();

}

bool	CameraDMK::setSelectedDevice(int id, string name){

    camera->chooseDevice(1, "The Imaging Source Europe GmbH-17410147");

    return true;

}

void	CameraDMK::setCameraPixelFormat(int depth){

    camera->setPixelFormat(depth);

}

void	CameraDMK::setCameraExposureTime(double value){

    camera->setExposureTime(value);

}

void	CameraDMK::setCameraGain(int value){

    camera->setGain(value);

}

void CameraDMK::grabOne(){

    Frame *newFrame;
    Mat img,imgFrame;

    namedWindow( "Grabbed frame", WINDOW_AUTOSIZE );// Create a window for display.

    Frame f;
    bool r = f.copyFrame(newFrame);

    if(camera->grabImage(newFrame, img )){

        Frame f;

        bool r = f.copyFrame(newFrame);

        newFrame->getImg().copyTo(imgFrame);

        if(camera->getPixelFormat() == 8){

            imshow( "Grabbed frame", f.getImg() );                   // Show our image inside it.



        //conversion to 8UC
        }else if(camera->getPixelFormat() == 12){

            cout << "12 bits image"<<endl;

        }

        waitKey(0);

    }

    camera->acqStop();

    camera->grabStop();

}

//https://github.com/xamox/aravis/blob/master/tests/arvheartbeattest.c
//http://blogs.gnome.org/emmanuel/2010/04/03/chose-promise-chose-due/
//https://code.google.com/p/tiscamera/source/browse/gige/aravis/examples/c/gstexample.c?spec=svn0c121ab10f32440f10bdad3ed7264fbf1e6b55f9&r=0c121ab10f32440f10bdad3ed7264fbf1e6b55f9
//https://gitorious.org/rock-drivers/camera-aravis/source/5d1b8f749670e0348b9b2e2d760204ed71c73010:src/recorder.cpp#L106
void CameraDMK::operator()(){

    bool stop = false;

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "acqThread");
	BOOST_LOG_SEV(log, notification) << "\n";
	BOOST_LOG_SEV(log, notification) << "Acquisition thread started.";
    //camera->grabStart();
    camera->acqStart();

    int c = 0;
    do {

                Frame *newFrame;
                Mat img;

                if(camera->grabImage(newFrame, img)){

                    boost::mutex::scoped_lock lock(*mutexQueue);

                    Frame f;
                    bool r = f.copyFrame(newFrame);

                    framesQueue->pushInFifo(f);

                    lock.unlock();
                    cout << "save : " << "testDMK-"+Conversion::intToString(c) <<endl;
                    SaveImg::saveBMP(f.getImg(),"testDMK-"+Conversion::intToString(c));
                    c++;
    /*
                    if(framesQueue->getFifoIsFull())
                        condQueueFill->notify_all();

                    framesQueue->setThreadRead("imgCap", true);
                    framesQueue->setThreadRead("astCap", true);
                    framesQueue->setThreadRead("det", true);

                    condQueueNewElement->notify_all();*/
                }

        // Get the "must stop" state (thread-safe)
        mustStopMutex->lock();

        stop = *mustStop;

        mustStopMutex->unlock();

    }while (stop == false);

    camera->acqStop();

    camera->grabStop();

    BOOST_LOG_SEV(log, notification) <<"Acquisition thread terminated";

}

















/* The appsink has received a buffer */
/* void new_buffer (GstElement *sink, customData *data) {

    cout << "Enter in new buffer function"<<endl;
    GstBuffer *buffer;
    IplImage* img;
    Mat img2 ;

    gint height, width;
    // Retrieve the buffer
    g_signal_emit_by_name (sink, "pull-buffer", &buffer);

    if (buffer) {
        // The only thing we do in this example is print a * to indicate a received buffer
        g_print ("*");

        // get format frames properties
        /* GstCaps* buffer_caps = gst_buffer_get_caps(buffer);

        GstStructure* structure = gst_caps_get_structure(buffer_caps, 0);

        if(!gst_structure_get_int(structure, "width", &width) ||!gst_structure_get_int(structure, "height", &height))
        {//
            //printf("getNewBuffer function returns false");
          //  return FALSE;


        }

        img2 = Mat::zeros(height,width, CV_8UC1);
        // img = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
        //img->imageData = (unsigned char*)GST_BUFFER_DATA(buffer);
        img2.data = (unsigned char*)GST_BUFFER_DATA(buffer);

        // processBuffer(img);

        conversion c;

        string pathAndname		= "/home/fripon/image"+c.int2String(data->cptt);

        const char * filename ;

        filename		= pathAndname.c_str();

        savei(img2, filename);
        // cvSaveImage(filename,img);
        */
/*
        gst_buffer_unref (buffer);
    }

    data->cptt++;

    cout << "cpt :"<<data->cptt<<endl;

}*/

//https://github.com/GNOME/aravis/blob/master/viewer/arvviewer.c
/*void get_new_buffer(ArvStream *stream, customData_ *d){

    ArvBuffer *arv_buffer;
	GstBuffer *buffer;
    gint height, width;

    CvMat* img ;
    GstMapInfo mapp;

    arv_buffer = arv_stream_timeout_pop_buffer(stream, 2000000);
	//arv_buffer = arv_stream_pop_buffer (stream);

	if (arv_buffer == NULL){
        cout << "arv_buffer is null"<<endl;
		return;
    }


    if (arv_buffer->status == ARV_BUFFER_STATUS_SUCCESS) {


        buffer = gst_buffer_new_wrapped_full (  GST_MEMORY_FLAG_READONLY,
                                                arv_buffer->data,
                                                arv_buffer->size,
                                                0,
                                                arv_buffer->size,
                                                NULL,
                                                NULL);


        //http://www.raspberrypi.org/forums/viewtopic.php?f=37&t=38837
        img = cvCreateMatHeader(960,1280, CV_8UC1);

        gboolean res = gst_buffer_map(buffer, &mapp, GST_MAP_READ);
        cvSetData(img, mapp.data, 1280);

		cout << "YES -> " <<d->cptt<<endl;

		gst_buffer_unmap(buffer, &mapp);
        gst_buffer_unref(buffer);
        cvReleaseMat(&img);

        d->cptt++;

    }else{

        int status = arv_buffer->status;

        switch(status){

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

    }

    arv_stream_push_buffer (stream, arv_buffer);

}*/



















/*
    bool stop = false;

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "acqThread");
	BOOST_LOG_SEV(log, notification) << "\n";
	BOOST_LOG_SEV(log, notification) << "Acquisition thread started.";

    gst_init (NULL, NULL);

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

    guint64 n_completed_buffers;
    guint64 n_failures;
    guint64 n_underruns;



    gboolean terminate = FALSE;

    customData_ data;


    //liste des caméras détectées
    arv_update_device_list ();

    //choix d'une caméra en fonction de son id
    camera = arv_camera_new(arv_get_device_id (1));

    cout << endl;
    cout << "DEVICE SELECTED : " << arv_camera_get_device_id(camera)   << endl;
    cout << "DEVICE NAME :     " << arv_camera_get_model_name(camera)  << endl;
    cout << "DEVICE VENDOR :   " << arv_camera_get_vendor_name(camera) << endl;
    cout << endl;

    //création d'un thread qui va stocker dans une pile les images reçues
    stream = arv_camera_create_stream (camera, NULL, NULL);

	if (stream == NULL) {
		g_object_unref (camera);
		camera = NULL;
		cout<< "stream is NULL" <<endl;
	}

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

	}

	//arv_stream_set_emit_signals (stream, TRUE);

    payload = arv_camera_get_payload (camera);
	cout << "PAYLOAD: " << payload<<endl;

	for (int i = 0; i < 20; i++)
		arv_stream_push_buffer (stream, arv_buffer_new (payload, NULL));

	arv_camera_get_region (camera, NULL, NULL, &width, &height);
	cout << "Width :  " << width << endl << "Height : "<< height <<endl;

	pixFormat = arv_camera_get_pixel_format (camera);
	pixel_format_string = arv_camera_get_pixel_format_as_string (camera);
	cout << "FORMAT : "<<pixel_format_string <<endl;

	arv_camera_get_exposure_time_bounds (camera, &exposureMin, &exposureMax);
	cout << "EXP :    "<< exposureMin<<" to "<< exposureMax<<endl;

	arv_camera_get_gain_bounds (camera, &gainMin, &gainMax);
	cout << "GAIN :   "<< gainMin<<" to "<< gainMax<<endl;

	fps = arv_camera_get_frame_rate (camera);
	cout << "FPS :    " << fps <<endl;


    // Pour la Mako -> StreamFrameRateConstrain
    // Pour la basler -> GevSCBWA

	gint64 fpsLimit =  arv_device_get_integer_feature_value (arv_camera_get_device(camera),"GevSCBWA");
    cout << "FPS limit : " << (int)fpsLimit <<endl;

    caps_string = arv_pixel_format_to_gst_caps_string (pixFormat);
    cout << "CAPS STRING : "<<caps_string<<endl;

	if (caps_string == NULL) {
		g_message ("GStreamer cannot understand the camera pixel format: 0x%x!\n", (int) pixFormat);
	}


    arv_camera_set_acquisition_mode(camera, ARV_ACQUISITION_MODE_CONTINUOUS);

	arv_camera_start_acquisition (camera);

	//Create pipeline
    pipeline = gst_pipeline_new ("pipeline");

    data.appsrc = gst_element_factory_make ("appsrc", NULL);

	//d.capsfilter = gst_element_factory_make ("capsfilter",       "filter");
    data.videoconvert = gst_element_factory_make ("videoconvert", NULL);
    data.autovideosink    = gst_element_factory_make("autovideosink", NULL);

    // check if factories messed up and could not create some elements
    if (!pipeline){
        g_printerr ("The pipeline element could not be created.\n");
        exit (-1);
    }

    if (!data.appsrc){
        g_printerr ("gstaravis could not be created.\n");
        exit (-1);
    }

    if (!data.videoconvert){
        g_printerr ("colorspace element could not be created.\n");
        exit (-1);
    }

    if (!data.autovideosink){
        g_printerr ("sink element could not be created.\n");
        exit (-1);
    }

    // add all elements into the pipeline
    gst_bin_add_many (GST_BIN (pipeline), data.appsrc, data.videoconvert, data.autovideosink, NULL);

    // link the elements together
    gst_element_link_many (data.appsrc, data.videoconvert, data.autovideosink, NULL);

	data.caps = gst_caps_new_simple ("video/x-raw",
                                     "bpp", G_TYPE_INT, 8,
                                     "depth", G_TYPE_INT, 8,
                                     "width", G_TYPE_INT, width,
                                     "height", G_TYPE_INT, height,
                                     "framerate", GST_TYPE_FRACTION, (unsigned int ) (double) (0.5 + fps), 1,
                                     NULL);

	gst_app_src_set_caps (GST_APP_SRC (data.appsrc), data.caps);
	gst_caps_unref (data.caps);

	g_object_set(G_OBJECT (data.appsrc), "format", GST_FORMAT_TIME, NULL);

   // g_signal_connect (stream, "new-buffer", G_CALLBACK (get_new_buffer), &data);

    // ready to play
    gst_element_set_state (pipeline, GST_STATE_READY);

    // Set the pipeline to "playing" state
    g_print ("Now playing stream from: %s\n", arv_camera_get_model_name(camera));

    //  IMPORTANT after state changed to playing the capabilities are fixed.
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    int ccpptt = 0;
    ArvBuffer *last_buffer;

    guint64 timestamp_offset =0;
	guint64 last_timestamp = 0;

    do {

        ArvBuffer *arv_buffer;
        GstBuffer *buffer;
        gint height, width;

        CvMat* img ;
        GstMapInfo mapp;

         GstFlowReturn ret;

        arv_buffer = arv_stream_timeout_pop_buffer(stream, 2000000);
        //arv_buffer = arv_stream_pop_buffer (stream);

        if (arv_buffer == NULL){

            cout << "arv_buffer is null"<<endl;
            return;

        }else{


            if (arv_buffer->status == ARV_BUFFER_STATUS_SUCCESS) {


            arv_buffer->timestamp_ns = g_get_real_time () * 1000LL;

            cout <<"timestamp : " <<arv_buffer->timestamp_ns;



               buffer = gst_buffer_new_wrapped_full (  GST_MEMORY_FLAG_READONLY,
                                                arv_buffer->data,
                                                arv_buffer->size,
                                                0,
                                                arv_buffer->size,
                                                NULL,
                                                NULL);


                if (timestamp_offset == 0) {
                    timestamp_offset = arv_buffer->timestamp_ns;
                    last_timestamp = arv_buffer->timestamp_ns;
                }


                GST_BUFFER_DTS (buffer) = arv_buffer->timestamp_ns - timestamp_offset;
                GST_BUFFER_DURATION (buffer) = arv_buffer->timestamp_ns - last_timestamp;

                last_timestamp = arv_buffer->timestamp_ns;

                //http://www.raspberrypi.org/forums/viewtopic.php?f=37&t=38837
                img = cvCreateMatHeader(960,1280, CV_8UC1);
                //img = cvCreateMatHeader(964,1292, CV_8UC1);

                // img = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
                //img->imageData = (unsigned char*)GST_BUFFER_DATA(buffer);
                //img2.data = (unsigned char*)GST_BUFFER_DATA(buffer);
                gboolean res = gst_buffer_map(buffer, &mapp, GST_MAP_READ);
                cvSetData(img, mapp.data, 1280);
                //cvSetData(img, mapp.data, 1292);

               // processBuffer(img);

                conversion c;

                string pathAndname		= "/home/fripon/image"+c.int2String(ccpptt);

                const char * filename ;

                filename = pathAndname.c_str();

                Mat m = Mat(img, true);

                savei(m, filename);

                //cout << "before"<<endl;
                ret = gst_app_src_push_buffer (GST_APP_SRC (data.appsrc), buffer);


                if(ret !=  GST_FLOW_OK){
                    g_debug("push buffer returned %d  \n", ret);
                    //return FALSE;
                }


                cout << "YES -> " <<endl;

                gst_buffer_unmap(buffer, &mapp);
                gst_buffer_unref(buffer);
                cvReleaseMat(&img);

                ccpptt++;

            }else{

                int status = arv_buffer->status;

                switch(status){

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

            }

            //usleep(10);
           // arv_stream_push_buffer (stream, arv_buffer);
            if (last_buffer != NULL)
                arv_stream_push_buffer (stream, last_buffer);
           last_buffer = arv_buffer;

        }
        // Get the "must stop" state (thread-safe)
        mustStopMutex.lock();

        stop = mustStop;

        mustStopMutex.unlock();

    } while (!terminate && stop == false);



	arv_stream_get_statistics (stream,&n_completed_buffers,&n_failures,&n_underruns);

    cout << "Completed buffers = " << (unsigned long long) n_completed_buffers<<endl;
    cout << "Failures          = " << (unsigned long long) n_failures<<endl;
    cout << "Underruns         = "<< (unsigned long long) n_underruns<<endl;

    arv_camera_stop_acquisition (camera);

    // Free ressources
    //gst_object_unref (bus);
    g_print ("Returned, stopping playback\n");
    gst_element_set_state (pipeline, GST_STATE_PAUSED);
    gst_element_set_state (pipeline, GST_STATE_NULL);

    g_object_unref (stream);
    g_object_unref (camera);

    g_print("Deleting pipeline\n");
    gst_object_unref (GST_OBJECT (pipeline));


    BOOST_LOG_SEV(log, notification) <<"Acquisition thread terminated";

}























/*

    bool stop = false;

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "acqThread");
	BOOST_LOG_SEV(log, notification) << "\n";
	BOOST_LOG_SEV(log, notification) << "Acquisition thread started.";

    gst_init (NULL, NULL);


    // initialize out gstreamer pipeline
    char CAMERA[]      = "Basler-21418131";//"The Imaging Source Europe GmbH-25010394"; //
    int  WIDTH         = 1280;
    int  HEIGHT        = 960;
    int  INIT_FPS      = 30;
    char FORMAT_MONO[] = "video/x-raw-gray";

    GstBus          *bus;
    GstMessage      *msg;
    ArvCamera       *camera;
    ArvPixelFormat  pixFormat;
    int             width;
	int             height;
	double          fps;
	double          gainMin,
                    gainMax;

    gboolean terminate = FALSE;

    customData data;

    data.cptt = 0;

    // Create gstreamer elements
    pipeline   = gst_pipeline_new ("player");
    GstElementFactory *factory = gst_element_factory_find("aravissrc");

    if(factory){
      data.source = gst_element_factory_create(factory,"source");
    }

    data.capsfilter = gst_element_factory_make ("capsfilter",       "filter");
    data.colorspace = gst_element_factory_make ("ffmpegcolorspace", "colorspace");

    // Configure appsink. This plugin will allow us to access buffer data
    data.appSink    = gst_element_factory_make("appsink", "output");

    //emit the .new-sample. and .new-preroll. signals when a sample can be pulled without blocking.
    gst_app_sink_set_emit_signals(GST_APP_SINK(data.appSink), TRUE);
    //Check if appsink will drop old buffers when the maximum amount of queued buffers is reached.
    gst_app_sink_set_drop(GST_APP_SINK(data.appSink), FALSE);
    //Set the maximum amount of buffers that can be queued in appsink . After this amount of buffers
    //are queued in appsink, any more buffers will block upstream elements until a sample is pulled from appsink .
    gst_app_sink_set_max_buffers(GST_APP_SINK(data.appSink), 1);
    //define our conditions and set them for the capsfilter
    data.caps = gst_caps_new_simple (FORMAT_MONO,
                                "bpp",       G_TYPE_INT,    8,
                                "depth",     G_TYPE_INT,    8,
                                "width",     G_TYPE_INT,    WIDTH,
                                "height",    G_TYPE_INT,    HEIGHT,
                                "framerate", GST_TYPE_FRACTION, (guint) INIT_FPS, 1,
                                NULL);

    gst_app_sink_set_caps(GST_APP_SINK(data.appSink),data.caps);
    g_signal_connect (data.appSink, "new-buffer", G_CALLBACK (new_buffer), &data);
    gst_caps_unref(data.caps);

    // check if factories messed up and could not create some elements
    if (!pipeline)
    {
        g_printerr ("The pipeline element could not be created.\n");
        exit (-1);
    }

    if (!data.source)
    {
        g_printerr ("gstaravis could not be created.\n");
        exit (-1);
    }

    if (!data.colorspace)
    {
        g_printerr ("colorspace element could not be created.\n");
        exit (-1);
    }

    if (!data.appSink)
    {
        g_printerr ("app sink element could not be created.\n");
        exit (-1);
    }

    // set the input camera that shall be used by the source element
    g_object_set (G_OBJECT (data.source), "camera-name", CAMERA, NULL);

    arv_update_device_list ();

    camera = arv_camera_new(arv_get_device_id (0));

    cout << endl;
    cout << "DEVICE SELECTED : " << arv_camera_get_device_id(camera)   << endl;
    cout << "DEVICE NAME :     " << arv_camera_get_model_name(camera)  << endl;
    cout << "DEVICE VENDOR :   " << arv_camera_get_vendor_name(camera) << endl;
    cout << endl;

    //camera = (ArvCamera*)malloc(sizeof(ArvCamera));
    g_object_get (G_OBJECT (data.source), "camera", &camera, NULL);

    if (!ARV_IS_CAMERA(camera))
    {
        printf ("Unable to retrieve camera\n");
        exit(1);
    }


    // add all elements into the pipeline
    gst_bin_add_many (GST_BIN (pipeline),
                      data.source,
                      data.capsfilter,
                      data.colorspace,
                      data.appSink,
                      NULL);

    // link the elements together
    gst_element_link_many (data.source,
                           data.capsfilter,
                           data.colorspace,
                           data.appSink,
                           NULL);

    // ready to play
    gst_element_set_state (pipeline, GST_STATE_READY);

    // Set the pipeline to "playing" state
    g_print ("Now playing stream from: %s\n", CAMERA);

    //  IMPORTANT after state changed to playing the capabilities are fixed.
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    g_print ("Running...\n");
    cout <<"CPT = "<<data.cptt<<endl;

    double rmin,rmax;

    arv_camera_get_exposure_time_bounds (camera, &rmin, &rmax);

    cout <<"EXPOSURE RANGE : "<<rmin<<" to "<<rmax<<endl;


  // Wait until error or EOS
    bus = gst_element_get_bus (pipeline);

    do {

        msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

        // Parse message
        if (msg != NULL) {

          GError *err;
          gchar *debug_info;

          switch (GST_MESSAGE_TYPE (msg)) {

            case GST_MESSAGE_ERROR:
              gst_message_parse_error (msg, &err, &debug_info);
              g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
              g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
              g_clear_error (&err);
              g_free (debug_info);
              terminate = TRUE;
              break;

            case GST_MESSAGE_EOS:
              g_print ("End-Of-stream reached.\n");
              break;

            case GST_MESSAGE_STATE_CHANGED:
              // We are only interested in state-changed messages from the pipeline
              if (GST_MESSAGE_SRC (msg) == GST_OBJECT (pipeline)) {
                GstState old_state, new_state, pending_state;
                gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
                g_print ("Pipeline state changed from %s to %s:\n", gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
              }
              break;

            default:
              //we should not reach here because we only asked for ERRORs and EOS and State Changes
              g_printerr ("Unexpected message received.\n");
              break;
          }
          gst_message_unref (msg);
        }

        // Get the "must stop" state (thread-safe)
        mustStopMutex.lock();

        stop = mustStop;

        mustStopMutex.unlock();

    } while (!terminate && stop == false);

    // Free ressources
    gst_object_unref (bus);
    g_print ("Returned, stopping playback\n");
    gst_element_set_state (pipeline, GST_STATE_PAUSED);
    gst_element_set_state (pipeline, GST_STATE_NULL);

    g_print("Deleting pipeline\n");
    gst_object_unref (GST_OBJECT (pipeline));

    BOOST_LOG_SEV(log, notification) <<"Acquisition thread terminated";

}

*/
