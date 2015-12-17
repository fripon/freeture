/*
                        CameraV4l2.cpp

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
*   Last modified:      17/08/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    CameraV4l2.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    17/08/2015
*/

#include "CameraV4l2.h"

#ifdef LINUX

    enum io_method {
        IO_METHOD_READ,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
    };

    struct buffer
    {
        void   *start;
        size_t  length;
    };

    struct v4l2_buffer buf;

    enum io_method   io = IO_METHOD_MMAP;

    struct buffer    *buffers = NULL;
    unsigned int     n_buffers;
    int              out_buf = 1;
    int              frame_count = 10;
    int              frame_number = 0;


    boost::log::sources::severity_logger< LogSeverityLevel >  CameraV4l2::logger;
    CameraV4l2::Init CameraV4l2::initializer;

    CameraV4l2::CameraV4l2(){

        io_method io = IO_METHOD_MMAP;
        fd = -1;
        out_buf = 1;
        frame_count = 10;
        frame_number = 0;
        expMin = 0;
        expMax = 0;
        exp =0;
        gain = 0;
        gainMin = 0;
        gainMax = 0;
        mFrameCounter = 0;
        mWidth = 640;
        mHeight = 480;
        n_buffers = 3;

        mExposureAvailable = true;
        mGainAvailable = true;
        mCustomSize = false;
        mInputDeviceType = CAMERA;

    }

    CameraV4l2::~CameraV4l2(){


    }

    bool CameraV4l2::getInfos() {

        struct v4l2_capability caps = {};

        // http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-querycap.html

        if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &caps)) {
            perror("Querying Capabilities");
            return false;
        }

        cout << "Driver name     : " << caps.driver << endl;
        cout << "Device name     : " << caps.card << endl;
        cout << "Device location : " << caps.bus_info << endl;
        printf ("Driver version  : %u.%u.%u\n",(caps.version >> 16) & 0xFF, (caps.version >> 8) & 0xFF, caps.version & 0xFF);
        cout << "Capabilities    : " << caps.capabilities << endl;

        struct v4l2_cropcap cropcap;
        memset(&cropcap, 0, sizeof(cropcap));
        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl (fd, VIDIOC_CROPCAP, &cropcap)) {
            perror("Querying Cropping Capabilities");
            return false;
        }

        printf( "Camera Cropping :\n"
                "  Bounds  : %dx%d+%d+%d\n"
                "  Default : %dx%d+%d+%d\n"
                "  Aspect  : %d/%d\n",
                cropcap.bounds.width, cropcap.bounds.height, cropcap.bounds.left, cropcap.bounds.top,
                cropcap.defrect.width, cropcap.defrect.height, cropcap.defrect.left, cropcap.defrect.top,
                cropcap.pixelaspect.numerator, cropcap.pixelaspect.denominator);

        int support_grbg10 = 0;

        struct v4l2_fmtdesc fmtdesc = {0};
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        char fourcc[5] = {0};
        char c, e;
        printf( "  FORMAT    : CE Desc\n");
        while (0 == xioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc)) {
            strncpy(fourcc, (char *)&fmtdesc.pixelformat, 4);
            if (fmtdesc.pixelformat == V4L2_PIX_FMT_SGRBG10)
                support_grbg10 = 1;
            c = fmtdesc.flags & 1? 'C' : ' ';
            e = fmtdesc.flags & 2? 'E' : ' ';
            printf("  %s : %c%c %s\n", fourcc, c, e, fmtdesc.description);
            fmtdesc.index++;
        }

        /*struct v4l2_format fmt = {0};
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = 640;
        fmt.fmt.pix.height = 480;
        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;

        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)) {
            perror("Setting Pixel Format");
            return false;
        }

        strncpy(fourcc, (char *)&fmt.fmt.pix.pixelformat, 4);
        printf( "Selected mode   :\n"
                "  Width  : %d\n"
                "  Height : %d\n"
                "  PixFmt : %s\n"
                "  Field  : %d\n",
                fmt.fmt.pix.width,
                fmt.fmt.pix.height,
                fourcc,
                fmt.fmt.pix.field);*/

        double eMin, eMax; int gMin, gMax;
        getExposureBounds(eMin, eMax);
        cout << "Min exposure    : " << eMin << endl;
        cout << "Max exposure    : " << eMax << endl;

        getGainBounds(gMin, gMax);
        cout << "Min gain        : " << gMin << endl;
        cout << "Max gain        : " << gMax << endl;

        return true;

    };

    vector<pair<int,string>> CameraV4l2::getCamerasList() {

        vector<pair<int,string>> camerasList;

        bool loop = true;
        bool res = true;
        int deviceNumber = 0;

        do {

            string devicePathStr = "/dev/video" + Conversion::intToString(deviceNumber);

            // http://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c-cross-platform

            if(access(devicePathStr.c_str(), F_OK) != -1 ) {

                // file exists

                // http://stackoverflow.com/questions/4290834/how-to-get-a-list-of-video-capture-devices-web-cameras-on-linux-ubuntu-c

                int fd;

                if((fd = open(devicePathStr.c_str(), O_RDONLY)) == -1){
                    perror("Can't open device");
                    res = false;
                }else {

                    struct v4l2_capability caps = {};

                    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &caps)) {
                        cout << "Fail Querying Capabilities." << endl;
                        perror("Querying Capabilities");
                        res = false;
                    }else {

                        pair<int,string> c;
                        c.first = deviceNumber;
                        std::string s( reinterpret_cast< char const* >(caps.card) ) ;
                        c.second = "NAME[" + s + "] SDK[V4L2]";
                        camerasList.push_back(c);

                    }
                }

                close(fd);

                deviceNumber++;

            } else {

                loop = false;

            }

        }while(loop);

        return camerasList;

    }

    bool CameraV4l2::listCameras() {

        bool loop = true;
        bool res = true;
        int deviceNumber = 0;

        cout << endl << "------------ USB2 CAMERAS WITH V4L2 ----------" << endl << endl;

        do {

            string devicePathStr = "/dev/video" + Conversion::intToString(deviceNumber);

            // http://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c-cross-platform

            if(access(devicePathStr.c_str(), F_OK) != -1 ) {

                // file exists

                // http://stackoverflow.com/questions/4290834/how-to-get-a-list-of-video-capture-devices-web-cameras-on-linux-ubuntu-c

                int fd;

                if((fd = open(devicePathStr.c_str(), O_RDONLY)) == -1){
                    perror("Can't open device");
                    res = false;
                }else {

                    struct v4l2_capability caps = {};

                    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &caps)) {
                        perror("Querying Capabilities");
                        res = false;
                    }else {

                        cout << "-> [" << deviceNumber << "] " << caps.card << endl;

                    }
                }

                close(fd);

                deviceNumber++;

            } else {

                // file doesn't exist
                if(deviceNumber == 0)
                    cout << "-> No cameras detected ..." << endl;
                loop = false;

            }

        }while(loop);

        cout << endl << "------------------------------------------------" << endl << endl;

        return res;

    }

    bool CameraV4l2::createDevice(int id){

        string deviceNameStr = "/dev/video" + Conversion::intToString(id);
        mDeviceName = deviceNameStr.c_str();

        struct stat st;

        if (-1 == stat(mDeviceName, &st)) {
            fprintf(stderr, "Cannot identify '%s': %d, %s\n", mDeviceName, errno, strerror(errno));
            return false;
        }

        if (!S_ISCHR(st.st_mode)) {
            fprintf(stderr, "%s is no device\n", mDeviceName);
            return false;
        }

        fd = open(mDeviceName, O_RDWR /* required */ | O_NONBLOCK, 0);

        if (-1 == fd) {
            fprintf(stderr, "Cannot open '%s': %d, %s\n", mDeviceName, errno, strerror(errno));
            return false;
        }

        getExposureBounds(expMin, expMax);
        getGainBounds(gainMin, gainMax);

        memset(&mFormat, 0, sizeof(mFormat));
        mFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        // Preserve original settings as set by v4l2-ctl for example
        if (-1 == xioctl(fd, VIDIOC_G_FMT, &mFormat)){
            return false;
        }

        return true;

    }

    bool CameraV4l2::setSize(int width, int height, bool customSize) {
        mWidth = width;
        mHeight = height;
        mCustomSize = customSize;
        return true;
    }

    // if customSize = true --> set width and height values passed in argument
    // if customSize = false --> set maximum size
    bool CameraV4l2::setSize() {

        int chooseWidth = 0;
        int chooseHeight = 0;
        bool discreteSize = false;

        bool res = false;

        struct v4l2_frmsizeenum frmsize;
        memset(&frmsize, 0, sizeof(frmsize));
        frmsize.pixel_format = mFormat.fmt.pix.pixelformat; // Necessary to set size.

        while(ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {

            switch(frmsize.type) {

                case V4L2_FRMSIZE_TYPE_DISCRETE :

                    if(chooseHeight == 0 && chooseWidth == 0) {

                        chooseHeight = frmsize.discrete.height;
                        chooseWidth = frmsize.discrete.width;

                    }else {

                        if((abs(mWidth - chooseWidth) > abs(mWidth - frmsize.discrete.width)) && (abs(mHeight - chooseHeight) > abs(mHeight - frmsize.discrete.height))) {
                            chooseWidth = frmsize.discrete.width;
                            chooseHeight = frmsize.discrete.height;
                        }

                    }

                    discreteSize = true;
                    res = true;

                    break;

                case V4L2_FRMSIZE_TYPE_CONTINUOUS :

                    break;

                case V4L2_FRMSIZE_TYPE_STEPWISE :

                    if(mCustomSize) {

                        if(mWidth >= frmsize.stepwise.min_width && mWidth <=frmsize.stepwise.max_width) {

                            mFormat.fmt.pix.width = mWidth;

                        }else {

                            mFormat.fmt.pix.width = frmsize.stepwise.max_width;

                        }

                        if(mHeight >= frmsize.stepwise.min_height && mHeight <=frmsize.stepwise.max_height) {

                            mFormat.fmt.pix.height = mHeight;

                        }else {

                            mFormat.fmt.pix.height = frmsize.stepwise.max_height;

                        }

                    }else {

                        mFormat.fmt.pix.height = frmsize.stepwise.max_height;
                        mFormat.fmt.pix.width = frmsize.stepwise.max_width;

                    }

                    res = true;

                    break;

            }

            frmsize.index++;

        }

        if(discreteSize && res) {

            mFormat.fmt.pix.height = chooseHeight;
            mFormat.fmt.pix.width = chooseWidth;

        }


        return res;

    }


    bool CameraV4l2::getDeviceNameById(int id, string &device){

        return false;

    }

    bool CameraV4l2::getCameraName() {

        if(fd != -1) {

            struct v4l2_capability caps = {};

            // http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-querycap.html

            if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &caps)) {
                perror("Querying Capabilities");
                return false;
            }

            cout << "Driver name     : " << caps.driver << endl;
            cout << "Device name     : " << caps.card << endl;
            cout << "Device location : " << caps.bus_info << endl;
            printf ("Driver version  : %u.%u.%u\n",(caps.version >> 16) & 0xFF, (caps.version >> 8) & 0xFF, caps.version & 0xFF);
            cout << "Capabilities    : " << caps.capabilities << endl;

            return true;

        }

        return false;

    }

    bool CameraV4l2::grabInitialization(){

        struct v4l2_capability cap;
        struct v4l2_cropcap cropcap;
        struct v4l2_crop crop;

        unsigned int min;

        if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap))
        {
            if (EINVAL == errno)
            {
                fprintf(stderr,
                        "%s is no V4L2 device\n",
                        mDeviceName);
                exit(EXIT_FAILURE);
            }
            else
            {
                errno_exit("VIDIOC_QUERYCAP");
            }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
        {
            fprintf(stderr,
                    "%s is no video capture device\n",
                    mDeviceName);
            exit(EXIT_FAILURE);
        }

        switch (io)
        {
            case IO_METHOD_READ:
            {
                if (!(cap.capabilities & V4L2_CAP_READWRITE))
                {
                    fprintf(stderr,
                            "%s does not support read i/o\n",
                            mDeviceName);
                    exit(EXIT_FAILURE);
                }
                break;
            }
            case IO_METHOD_MMAP:
            case IO_METHOD_USERPTR:
            {
                if (!(cap.capabilities & V4L2_CAP_STREAMING))
                {
                    fprintf(stderr, "%s does not support streaming i/o\n",
                            mDeviceName);
                    exit(EXIT_FAILURE);
                }
                break;
            }
        }

        // Select video input, video standard and tune here.

        memset(&cropcap, 0, sizeof(cropcap));

        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
            crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            crop.c = cropcap.defrect; // reset to default

            if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
                switch (errno) {
                    case EINVAL:
                        // Cropping not supported.
                        break;
                    default:
                        // Errors ignored.
                        break;
                }
            }
        } else {
            // Errors ignored.
        }

        // Set some parameters...SIZE

        if(!setSize())
            return false;

        if(-1 == xioctl(fd, VIDIOC_S_FMT, &mFormat)) {
            cout << "Fail to set fmt." << endl;
            return false;
        }

        /* Buggy driver paranoia. */
        min = mFormat.fmt.pix.width * 2;
        if (mFormat.fmt.pix.bytesperline < min)
            mFormat.fmt.pix.bytesperline = min;
        min = mFormat.fmt.pix.bytesperline * mFormat.fmt.pix.height;
        if (mFormat.fmt.pix.sizeimage < min)
            mFormat.fmt.pix.sizeimage = min;

        return true;

    }

    void CameraV4l2::grabCleanse(){

        // Uninit device

        unsigned int i;

        if(buffers != NULL) {
            switch (io) {

                case IO_METHOD_READ:
                    free(buffers[0].start);
                    break;

                case IO_METHOD_MMAP:

                    for (i = 0; i < n_buffers; ++i)
                        if (-1 == munmap(buffers[i].start, buffers[i].length))
                            errno_exit("munmap");
                    break;

                case IO_METHOD_USERPTR:
                    for (i = 0; i < n_buffers; ++i)
                        free(buffers[i].start);
                    break;
            }

            free(buffers);
        }

        // Close device

        if (-1 == close(fd))
            errno_exit("close");

        fd = -1;

    }

    bool CameraV4l2::acqStart(){

        // INIT DEVICE

        unsigned int i;
        enum v4l2_buf_type type;

        switch (io) {
            case IO_METHOD_READ:
                init_read(mFormat.fmt.pix.sizeimage);
                break;

            case IO_METHOD_MMAP:
                init_mmap();
                break;

            case IO_METHOD_USERPTR:
                init_userp(mFormat.fmt.pix.sizeimage);
                break;
        }

        // START CAPTURING

        switch (io) {
            case IO_METHOD_READ:
            {
                /* Nothing to do. */
                break;
            }
            case IO_METHOD_MMAP:
            {
                for (i = 0; i < n_buffers; ++i)
                {
                    struct v4l2_buffer buf;

                    memset(&buf, 0, sizeof(buf));
                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    buf.memory = V4L2_MEMORY_MMAP;
                    buf.index = i;

                    if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                    {
                        errno_exit("VIDIOC_QBUF");
                        return false;
                    }
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
                {
                    errno_exit("VIDIOC_STREAMON");
                    return false;
                }
                break;
            }
            case IO_METHOD_USERPTR:
            {
                for (i = 0; i < n_buffers; ++i)
                {
                    struct v4l2_buffer buf;

                    memset(&buf, 0, sizeof(buf));
                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    buf.memory = V4L2_MEMORY_USERPTR;
                    buf.index = i;
                    buf.m.userptr = (unsigned long)buffers[i].start;
                    buf.length = buffers[i].length;

                    if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                    {
                        errno_exit("VIDIOC_QBUF");
                        return false;
                    }
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
                {
                    errno_exit("VIDIOC_STREAMON");
                    return false;
                }
                break;
            }
        }

        return true;
    }

    void CameraV4l2::acqStop(){

        enum v4l2_buf_type type;

        switch (io)
        {
            case IO_METHOD_READ:
            {
                /* Nothing to do. */
                break;
            }
            case IO_METHOD_MMAP:
            case IO_METHOD_USERPTR:
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
                {
                    errno_exit("VIDIOC_STREAMOFF");
                }
                break;
        }

    }

    bool CameraV4l2::grabImage(Frame &newFrame) {

        unsigned char* ImageBuffer = NULL;

        Mat img = Mat(mFormat.fmt.pix.height,mFormat.fmt.pix.width,CV_8UC1, Scalar(0));
        size_t s = mFormat.fmt.pix.width*mFormat.fmt.pix.height;

        bool grabSuccess = false;

        for(;;) {

            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            /* Timeout. */
            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select(fd + 1, &fds, NULL, NULL, &tv);

            if(-1 == r) {
              if (EINTR == errno)
              continue;
              errno_exit("select");
            }

            if(0 == r) {
              fprintf(stderr, "select timeout\n");
              BOOST_LOG_SEV(logger, warning) << "Select timeout !";
              //exit(EXIT_FAILURE);
            }

            if(read_frame()) {
                grabSuccess = true;
                break;
            }
            /* EAGAIN - continue select loop. */
        }

       if(grabSuccess) {

            ImageBuffer = (unsigned char*)buffers[buf.index].start;

            boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
            newFrame.mDate = TimeDate::splitIsoExtendedDate(to_iso_extended_string(time));

            double fps = 0;
            if(getFPS(fps))
                newFrame.mFps = fps;
            newFrame.mFormat = MONO8;
            newFrame.mSaturatedValue = 255;
            newFrame.mFrameNumber = mFrameCounter;
            newFrame.mExposure = exp;
            newFrame.mGain = gain;
            mFrameCounter++;

            if(!convertImage(ImageBuffer, newFrame.mImg))
                grabSuccess = false;

        }

        return grabSuccess;

    }

    bool CameraV4l2::grabSingleImage(Frame &frame, int camID){

        createDevice(camID);

        if(frame.mHeight > 0 && frame.mWidth > 0) {

            cout << "Setting size to : " << frame.mWidth << "x" << frame.mHeight << endl;
            mWidth = frame.mWidth;
            mHeight = frame.mHeight;
            mCustomSize = true;

        }

        grabInitialization();

        acqStart();

        cout << ">> Height : " << mFormat.fmt.pix.height << endl;
        cout << ">> Width  : " << mFormat.fmt.pix.width << endl;

        if(!setPixelFormat(frame.mFormat))
            return false;

        if(expMin != -1 && expMax != -1)
            setExposureTime(frame.mExposure);
        if(expMin != -1 && expMax != -1)
            setGain(frame.mGain);

        unsigned char* ImageBuffer = NULL;

        Mat img = Mat(mFormat.fmt.pix.height,mFormat.fmt.pix.width,CV_8UC1, Scalar(0));
        size_t s = mFormat.fmt.pix.width*mFormat.fmt.pix.height;

        bool grabSuccess = false;

        for(int i = 0; i< n_buffers; i++) {


            for(;;) {

                fd_set fds;
                struct timeval tv;
                int r;

                FD_ZERO(&fds);
                FD_SET(fd, &fds);

                /* Timeout. */
                int timeout = 2;

                if(frame.mExposure/1000000 > 1)
                    timeout = timeout + (int)(frame.mExposure/1000000);

                tv.tv_sec = timeout;
                tv.tv_usec = 0;

                r = select(fd + 1, &fds, NULL, NULL, &tv);

                if(-1 == r) {
                  if (EINTR == errno)
                    continue;
                  errno_exit("select");
                }

                if(0 == r) {
                  fprintf(stderr, "select timeout\n");
                  exit(EXIT_FAILURE);
                }

                if(read_frame()) {
                    grabSuccess = true;
                    break;
                }
                /* EAGAIN - continue select loop. */
            }
        }

        if(grabSuccess) {

            ImageBuffer = (unsigned char*)buffers[buf.index].start;


            boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
            frame.mDate = TimeDate::splitIsoExtendedDate(to_iso_extended_string(time));

            double fps = 0;
            if(getFPS(fps))
                frame.mFps = fps;
            frame.mSaturatedValue = 255;
            frame.mFrameNumber = mFrameCounter;

            cout << "size image buffer : " << sizeof(buffers[buf.index].start)  << endl;
            if(!convertImage(ImageBuffer, frame.mImg))
                grabSuccess = false;

        }

        acqStop();
        grabCleanse();

        return grabSuccess;

    }

    bool CameraV4l2::convertImage(unsigned char* buffer, Mat &image) {

        bool res = false;

        if(buffer != NULL) {

            switch(mFormat.fmt.pix.pixelformat) {

                case V4L2_PIX_FMT_GREY :

                    {

                        image = Mat(mFormat.fmt.pix.height, mFormat.fmt.pix.width, CV_8UC1, Scalar(0));
                        memcpy(image.ptr(), buffer, mFormat.fmt.pix.width*mFormat.fmt.pix.height);
                        res = true;

                    }

                    break;

                case V4L2_PIX_FMT_YUYV :

                    {
                        unsigned char* bigbuffer = (unsigned char*)malloc(mFormat.fmt.pix.height * mFormat.fmt.pix.width*3*sizeof(char));
                        Mat dispimg(mFormat.fmt.pix.height, mFormat.fmt.pix.width, CV_8UC3, bigbuffer);
                        PixFmtConv::YUYV_to_BGR24(buffer, bigbuffer, mFormat.fmt.pix.width, mFormat.fmt.pix.height, mFormat.fmt.pix.bytesperline);
                        cvtColor(dispimg,image,CV_BGR2GRAY);
                        res = true;
                        free(bigbuffer);

                    }

                    break;

                case V4L2_PIX_FMT_UYVY :

                    {
                        unsigned char bigbuffer[mFormat.fmt.pix.height * mFormat.fmt.pix.width*3];
                        PixFmtConv::UYVY_to_BGR24(buffer, bigbuffer, mFormat.fmt.pix.width, mFormat.fmt.pix.height, mFormat.fmt.pix.bytesperline);
                        Mat dispimg(mFormat.fmt.pix.height, mFormat.fmt.pix.width, CV_8UC3, bigbuffer);
                        cvtColor(dispimg,image,CV_BGR2GRAY);
                        res = true;
                    }

                    break;

                case V4L2_PIX_FMT_RGB565 :

                    {

                        unsigned char bigbuffer[mFormat.fmt.pix.height * mFormat.fmt.pix.width*3];
                        PixFmtConv::RGB565_to_BGR24(buffer, bigbuffer, mFormat.fmt.pix.width, mFormat.fmt.pix.height);
                        Mat dispimg(mFormat.fmt.pix.height, mFormat.fmt.pix.width, CV_8UC3, bigbuffer);
                        cvtColor(dispimg,image,CV_BGR2GRAY);
                        res = true;

                    }

                    break;

                case V4L2_PIX_FMT_BGR24 :

                    {
                        Mat dispimg = Mat(mFormat.fmt.pix.height, mFormat.fmt.pix.width, CV_8UC3, buffer);
                        cvtColor(dispimg,image,CV_BGR2GRAY);
                        res = true;

                    }

                    break;

                case V4L2_PIX_FMT_RGB24 :

                    {
                        Mat dispimg = Mat(mFormat.fmt.pix.height, mFormat.fmt.pix.width, CV_8UC3, buffer);
                        cvtColor(dispimg,image,CV_BGR2GRAY);
                        res = true;

                    }

                    break;

            }

        }

        return res;

    }

    void CameraV4l2::getExposureBounds(double &eMin, double &eMax){

        struct v4l2_queryctrl queryctrl;
        memset(&queryctrl, 0, sizeof(queryctrl));
        queryctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;

        if (-1 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) {

            if (errno != EINVAL) {

                perror("VIDIOC_QUERYCTRL");
                exit(EXIT_FAILURE);

            } else {

                printf(">> V4L2_CID_EXPOSURE_ABSOLUTE is not supported\n");
                eMin = -1;
                eMax = -1;

            }

        } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {

            printf(">> V4L2_CID_EXPOSURE_ABSOLUTE is not supported\n");
            eMin = -1;
            eMax = -1;

        } else {

            /*cout << "Name    : " << queryctrl.name << endl;
            cout << "Min     : " << queryctrl.minimum << endl;
            cout << "Max     : " << queryctrl.maximum << endl;
            cout << "Step    : " << queryctrl.step << endl;
            cout << "Default : " << queryctrl.default_value << endl;
            cout << "Flags   : " << queryctrl.flags << endl;*/

            eMin = queryctrl.minimum;
            eMax = queryctrl.maximum;

        }

    }

    double CameraV4l2::getExposureTime(){

        struct v4l2_control control;
        memset(&control, 0, sizeof(control));
        control.id = V4L2_CID_EXPOSURE_ABSOLUTE;

        if(0 == ioctl(fd, VIDIOC_G_CTRL, &control)) {

            return control.value * 100;

        // Ignore if V4L2_CID_CONTRAST is unsupported
        } else if (errno != EINVAL) {

            perror("VIDIOC_G_CTRL");

        }

        return 0;

    }

    void CameraV4l2::getGainBounds(int &gMin, int &gMax){

        struct v4l2_queryctrl queryctrl;
        memset(&queryctrl, 0, sizeof(queryctrl));
        queryctrl.id = V4L2_CID_GAIN;

        if (-1 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) {

            if (errno != EINVAL) {

                perror("VIDIOC_QUERYCTRL");
                exit(EXIT_FAILURE);

            } else {

                printf(">> V4L2_CID_GAIN is not supported\n");
                gainMin = -1;
                gainMax = -1;

            }

        } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {

            printf(">> V4L2_CID_GAIN is not supported\n");
            gainMin = -1;
            gainMax = -1;

        } else {

            /*cout << "Name    : " << queryctrl.name << endl;
            cout << "Min     : " << queryctrl.minimum << endl;
            cout << "Max     : " << queryctrl.maximum << endl;
            cout << "Step    : " << queryctrl.step << endl;
            cout << "Default : " << queryctrl.default_value << endl;
            cout << "Flags   : " << queryctrl.flags << endl;*/

            gMin = queryctrl.minimum;
            gMax = queryctrl.maximum;

        }

    }

    bool CameraV4l2::getPixelFormat(CamPixFmt &format){

        /*char fourcc[5] = {0};

        struct v4l2_format fmt;
        memset(&fmt, 0, sizeof(fmt));
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = mWidth;
        fmt.fmt.pix.height = mHeight;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;

        if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt)) {
            perror("Getting Pixel Format");
            return false;
        }

        // http://linuxtv.org/downloads/v4l-dvb-apis/V4L2-PIX-FMT-GREY.html
        if(fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_GREY) {

            strncpy(fourcc, (char *)&fmt.fmt.pix.pixelformat, 4);
            cout << "Pixel format : V4L2_PIX_FMT_GREY" << endl;
            format = MONO_8;

        // http://linuxtv.org/downloads/v4l-dvb-apis/V4L2-PIX-FMT-Y12.html
        }else if(fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_Y12) {

            strncpy(fourcc, (char *)&fmt.fmt.pix.pixelformat, 4);
            cout << "Pixel format : V4L2_PIX_FMT_Y12" << endl;
            format = MONO_12;

        }*/

        return true;
    }

    bool CameraV4l2::getFrameSizeEnum() {

        bool res = false;

        struct v4l2_frmsizeenum frmsize;
        memset(&frmsize, 0, sizeof(frmsize));
        frmsize.pixel_format = mFormat.fmt.pix.pixelformat;

        while(ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {

            switch(frmsize.type) {

                case V4L2_FRMSIZE_TYPE_DISCRETE :

                    cout << "[" << frmsize.index << "] : " << frmsize.discrete.width << "x" << frmsize.discrete.height << endl;
                    res = true;

                    break;

                case V4L2_FRMSIZE_TYPE_CONTINUOUS :

                    break;

                case V4L2_FRMSIZE_TYPE_STEPWISE :

                    cout << "Min width : " << frmsize.stepwise.min_width << endl;
                    cout << "Max width : " << frmsize.stepwise.max_width << endl;
                    cout << "Step width : " << frmsize.stepwise.step_width << endl;

                    cout << "Min height : " << frmsize.stepwise.min_height << endl;
                    cout << "Max height : " << frmsize.stepwise.max_height << endl;
                    cout << "Step height : " << frmsize.stepwise.step_height << endl;

                    break;

            }

            frmsize.index++;

        }

        return res;

    }

    bool CameraV4l2::getFrameSize(int &w, int &h) {

        w = 0;
        h = 0;

        struct v4l2_format fmt;
        memset(&fmt, 0, sizeof(fmt));
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;

        if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt)) {
            perror("Getting Pixel Format");
            return false;
        }

        h = fmt.fmt.pix.height;
        w = fmt.fmt.pix.width;

        return true;

    }

    bool CameraV4l2::getFpsEnum(vector<double> &values){

        bool res = false;

        struct v4l2_frmivalenum temp;
        memset(&temp, 0, sizeof(temp));
        temp.pixel_format = mFormat.fmt.pix.pixelformat;
        temp.width = mFormat.fmt.pix.width;
        temp.height = mFormat.fmt.pix.height;

        ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &temp);
        if (temp.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
            while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &temp) != -1) {
                values.push_back(float(temp.discrete.denominator)/temp.discrete.numerator);
                cout << values.back() << " fps" << endl;
                temp.index += 1;
                res = true;
            }
        }
        float stepval = 0;
        if (temp.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
            stepval = 1;
        }
        if (temp.type == V4L2_FRMIVAL_TYPE_STEPWISE || temp.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
            float minval = float(temp.stepwise.min.numerator)/temp.stepwise.min.denominator;
            float maxval = float(temp.stepwise.max.numerator)/temp.stepwise.max.denominator;
            if (stepval == 0) {
                stepval = float(temp.stepwise.step.numerator)/temp.stepwise.step.denominator;
            }
            for (float cval = minval; cval <= maxval; cval += stepval) {
                cout << 1/cval << " fps" << endl;
                values.push_back(1.0/cval);
                res = true;
            }
        }

        return res;

    }

    bool CameraV4l2::getFPS(double &value) {

        struct v4l2_streamparm streamparm;
        struct v4l2_fract *tpf;

        streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == ioctl(fd, VIDIOC_G_PARM, &streamparm)) {
            cout << "Fail to read fps value." << endl;
            return false;
        }

        tpf = &streamparm.parm.capture.timeperframe;

        value = (double)tpf->denominator / (double)tpf->numerator;

        return true;
    }

    string CameraV4l2::getModelName(){

        struct v4l2_capability caps = {};

        // http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-querycap.html

        if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &caps)) {
            perror("Querying device's name");
            return "";
        }

        return (char*)caps.card;

    }

    bool CameraV4l2::setExposureTime(double val){

        if(expMax > 0 && expMin > 0 && val >= expMin && val <= expMax) {

            // ************************ DISABLE AUTO EXPOSURE *****************************

	    struct v4l2_queryctrl queryctrl1;
            struct v4l2_control control1;
            memset(&queryctrl1, 0, sizeof(queryctrl1));
            queryctrl1.id = V4L2_CID_EXPOSURE_AUTO;

            if(-1 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl1)) {

                if(errno != EINVAL) {

                    perror("VIDIOC_QUERYCTRL");
                    return false;

                }else {

                    printf(">> V4L2_CID_EXPOSURE_AUTO is not supported\n");

                }

            }else if (queryctrl1.flags & V4L2_CTRL_FLAG_DISABLED) {

                printf(">> V4L2_CID_EXPOSURE_AUTO is not supported\n");

            }else {

                memset(&control1, 0, sizeof (control1));
                control1.id = V4L2_CID_EXPOSURE_AUTO;
                control1.value = V4L2_EXPOSURE_MANUAL;

                if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control1)) {
                    perror("VIDIOC_S_CTRL");
                    return false;
                }

                cout << ">> Manual exposure setted." << endl;

            }

            // ************************ SET AUTO EXPOSURE *****************************

	    struct v4l2_queryctrl queryctrl;
            struct v4l2_control control;
            memset(&queryctrl, 0, sizeof(queryctrl));
            queryctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;

            if(-1 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) {

                if(errno != EINVAL) {

                    perror("VIDIOC_QUERYCTRL");
                    return false;

                }else {

                    printf(">> V4L2_CID_EXPOSURE_ABSOLUTE is not supported\n");

                }

            }else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {

                printf(">> V4L2_CID_EXPOSURE_ABSOLUTE is not supported\n");

            }else {

                memset(&control, 0, sizeof (control));
                control.id = V4L2_CID_EXPOSURE_ABSOLUTE;

                /*
                V4L2_CID_EXPOSURE_ABSOLUTE integer
                Determines the exposure time of the camera sensor.
                The exposure time is limited by the frame interval.
                Drivers should interpret the values as 100 µs units, w
                here the value 1 stands for 1/10000th of a second, 10000
                for 1 second and 100000 for 10 seconds.
                */

                control.value = val/100;
                exp = val;
                printf(">> V4L2_CID_EXPOSURE_ABSOLUTE setted to %f (%f with V4L2)\n", val, val/100);

                if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control)) {
                    perror("VIDIOC_S_CTRL");
                    return false;
                }

            }

            return true;

        }else {

            if(expMin == -1 && expMax == -1) {

                cout << "Exposure time not supported." << endl;
                return true;

            }

            cout << "> Exposure value (" << val << ") is not in range [ " << expMin << " - " << expMax << " ]" << endl;

        }

        return false;
    }

    bool CameraV4l2::setGain(int val){

        if(gainMax > 0 && gainMin > 0 && val >= gainMin && val <= gainMax) {

            struct v4l2_queryctrl queryctrl;
            struct v4l2_control control;
            memset(&queryctrl, 0, sizeof(queryctrl));
            queryctrl.id = V4L2_CID_GAIN;

            if(-1 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) {

                if(errno != EINVAL) {

                    perror("VIDIOC_QUERYCTRL");
                    return false;

                }else {

                    printf(">> V4L2_CID_GAIN is not supported\n");

                }

            }else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {

                printf(">> V4L2_CID_GAIN is not supported\n");

            }else {

                memset(&control, 0, sizeof (control));
                control.id = V4L2_CID_GAIN;
                control.value = val;
                gain = val;

                if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control)) {
                    perror("VIDIOC_S_CTRL");
                    return false;
                }

            }

            return true;

        }else {

            if(gainMin == -1 && gainMax == -1) {

                cout << "Gain not supported." << endl;
                return true;

            }

            cout << "> Gain value (" << val << ") is not in range [ " << gainMin << " - " << gainMax << " ]" << endl;

        }

        return false;

    }

    bool CameraV4l2::setFPS(double fps){

        bool res = true;
        struct v4l2_frmivalenum temp;
        memset(&temp, 0, sizeof(temp));
        temp.pixel_format = mFormat.fmt.pix.pixelformat;
        temp.width = mFormat.fmt.pix.width;
        temp.height = mFormat.fmt.pix.height;

        ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &temp);

        if (temp.type == V4L2_FRMIVAL_TYPE_DISCRETE) {

            vector<double> frameIntervals;
            while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &temp) != -1) {

                if(fps == (float(temp.discrete.denominator)/temp.discrete.numerator)) {

                    struct v4l2_streamparm setfps;
                    struct v4l2_fract *tpf;
                    memset (&setfps, 0, sizeof (setfps));
                    setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    tpf = &setfps.parm.capture.timeperframe;

                    tpf->numerator = temp.discrete.numerator;
                    //cout << "numerator : " << tpf->numerator << endl;
                    tpf->denominator = temp.discrete.denominator;//cvRound(fps);
                    //cout << "denominator : " << tpf->denominator << endl;
                    //retval=1;
                    if (ioctl(fd, VIDIOC_S_PARM, &setfps) < 0) {
                        cout << "Failed to set camera FPS:"  << strerror(errno) << endl;
                        res = false;
                        break;
                    }

                    break;

                }

                temp.index += 1;

            }
        }

        float stepval = 0;
        if (temp.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
            stepval = 1;
            cout << "V4L2_FRMIVAL_TYPE_CONTINUOUS" << endl;
            struct v4l2_streamparm setfps;
            struct v4l2_fract *tpf;
            memset (&setfps, 0, sizeof (setfps));
            setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            tpf = &setfps.parm.capture.timeperframe;

            tpf->numerator = 1000;
            //cout << "numerator : " << tpf->numerator << endl;
            tpf->denominator = fps*1000;//cvRound(fps);
            //cout << "denominator : " << tpf->denominator << endl;
            //retval=1;
            if (ioctl(fd, VIDIOC_S_PARM, &setfps) < 0) {
                cout << "Failed to set camera FPS:"  << strerror(errno) << endl;
                res = false;

            }else{

                if (!tpf->denominator || !tpf->numerator)
                    printf("Invalid frame rate\n");
                else
				printf("Frame rate set to %.3f fps\n",
					1.0 * tpf->denominator / tpf->numerator);
            }
        }

        if (temp.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
            cout << "V4L2_FRMIVAL_TYPE_STEPWISE" << endl;
            float minval = float(temp.stepwise.min.numerator)/temp.stepwise.min.denominator;
            float maxval = float(temp.stepwise.max.numerator)/temp.stepwise.max.denominator;
            if (stepval == 0) {
                stepval = float(temp.stepwise.step.numerator)/temp.stepwise.step.denominator;
            }
            /*for (float cval = minval; cval <= maxval; cval += stepval) {
                cout << 1/cval << " fps" << endl;

            }*/



        }

        return res;

    }

    bool CameraV4l2::setPixelFormat(CamPixFmt depth){

        struct v4l2_fmtdesc fmtdesc = {0};
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        char fourcc[5] = {0};
        bool fmtFound = false;
        char c, e;
        mFormat.fmt.pix.field = V4L2_FIELD_NONE;
        EParser<CamPixFmt> fmt;
        string fstring = fmt.getStringEnum(depth);

        while (0 == xioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc)) {

            strncpy(fourcc, (char *)&fmtdesc.pixelformat, 4);

            if(string(fourcc) == fstring) {

                fmtFound = true;

                switch(depth) {

                    case MONO8 :

                        {

                            mFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;

                        }

                        break;

                    case GREY :

                        {

                            mFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;

                        }

                        break;

                    case YUYV :

                        {

                            mFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

                        }

                        break;

                    case UYVY :

                        {

                            mFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;

                        }

                        break;

                    case RGB565 :

                        {

                            mFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;

                        }

                        break;

                    case BGR3 :

                        {

                            mFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;

                        }

                        break;

                    case RGB3 :

                        {

                            mFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;

                        }

                        break;

                }


                break;
            }

            fmtdesc.index++;
        }

        if(!fmtFound) {

            BOOST_LOG_SEV(logger, critical) << ">> FORMAT " << fstring << " NOT SUPPORTED !";
            return false;
        }

        //strncpy(fourcc, (char *)&mFormat.fmt.pix.pixelformat, 4);

        return true;

    }

    void CameraV4l2::getAvailablePixelFormats(){

        struct v4l2_fmtdesc fmtdesc = {0};
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        char fourcc[5] = {0};
        vector<string> pixfmt;
        char c, e;
        struct v4l2_format pfmt;
        memset(&pfmt, 0, sizeof(pfmt));
        pfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        pfmt.fmt.pix.field = V4L2_FIELD_NONE;

        cout << ">> Device pixel formats :" << endl;

        while (0 == xioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc)) {

            strncpy(fourcc, (char *)&fmtdesc.pixelformat, 4);

            c = fmtdesc.flags & 1? 'C' : ' ';
            e = fmtdesc.flags & 2? 'E' : ' ';
            //printf("  %s : %c%c %s\n", fourcc, c, e, fmtdesc.description);
            string fmt = string(fourcc);
            std::transform(fmt.begin(), fmt.end(),fmt.begin(), ::toupper);
            pixfmt.push_back(fmt);
            cout << "- " << fmt << endl;
            fmtdesc.index++;
        }

        // Compare found pixel formats to currently formats supported by freeture

        cout << endl <<  ">> Available pixel formats :" << endl;
        EParser<CamPixFmt> fmt;

        for( int i = 0; i != pixfmt.size(); i++ ) {

            if(fmt.isEnumValue(pixfmt.at(i))) {

                cout << "- " << pixfmt.at(i) << " available --> ID : " << fmt.parseEnum(pixfmt.at(i)) << endl;

            }

        }

    }

    void CameraV4l2::errno_exit (const char *s) {
        fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    int CameraV4l2::xioctl (int fh, int request, void *arg) {
        int r;

        do
        {
            r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);

        return r;
    }

    int CameraV4l2::read_frame (void) {
        //struct v4l2_buffer buf;
        unsigned int i;

        switch (io)
        {
            case IO_METHOD_READ:
            {

                if (-1 == read(fd, buffers[0].start, buffers[0].length))
                {
                    switch (errno)
                    {
                        case EAGAIN:
                            return 0;

                        case EIO:
                            /* Could ignore EIO, see spec. */

                            /* fall through */

                        default:
                            errno_exit("read");
                    }
                }

                break;
            }
            case IO_METHOD_MMAP:
            {

                memset(&buf, 0, sizeof(buf));
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;

                if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
                {
                    switch (errno)
                    {
                        case EAGAIN:
                            return 0;

                        case EIO:
                            /* Could ignore EIO, see spec. */

                            /* fall through */

                        default:
                            errno_exit("VIDIOC_DQBUF");
                    }
                }

                assert(buf.index < n_buffers);

                if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                    errno_exit("VIDIOC_QBUF");
                break;
            }
            case IO_METHOD_USERPTR:
            {

                memset(&buf, 0, sizeof(buf));
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_USERPTR;

                if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
                {
                    switch (errno)
                    {
                        case EAGAIN:
                            return 0;

                        case EIO:
                            /* Could ignore EIO, see spec. */

                            /* fall through */

                        default:
                        {
                            errno_exit("VIDIOC_DQBUF");
                        }
                    }
                }

                for (i = 0; i < n_buffers; ++i)
                {
                    if (buf.m.userptr == (unsigned long)buffers[i].start
                        && buf.length == buffers[i].length)
                        break;
                }
                assert(i < n_buffers);

                if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                {
                    errno_exit("VIDIOC_QBUF");
                }
                break;
            }
        }

        return 1;
    }

    void CameraV4l2::init_read (unsigned int buffer_size) {
        buffers = (buffer*)(calloc(1, sizeof(*buffers)));

        if (!buffers)
        {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }

        buffers[0].length = buffer_size;
        buffers[0].start = malloc(buffer_size);

        if (!buffers[0].start)
        {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }
    }

    void CameraV4l2::init_mmap (void) {

        struct v4l2_requestbuffers req;

        memset(&req, 0, sizeof(req));

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
        {
            if (EINVAL == errno)
            {
                fprintf(stderr, "%s does not support "
                        "memory mapping\n", mDeviceName);
                exit(EXIT_FAILURE);
            }
            else
            {
                errno_exit("VIDIOC_REQBUFS");
            }
        }

        if (req.count < 2)                          \
        {
            fprintf(stderr, "Insufficient buffer memory on %s\n",
                    mDeviceName);
            exit(EXIT_FAILURE);
        }

        buffers = (buffer*)calloc(req.count, sizeof(*buffers));

        if (!buffers)
        {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
        {
            struct v4l2_buffer buf;

            memset(&buf, 0, sizeof(buf));

            buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory      = V4L2_MEMORY_MMAP;
            buf.index       = n_buffers;

            if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
                errno_exit("VIDIOC_QUERYBUF");

            buffers[n_buffers].length = buf.length;

            buffers[n_buffers].start =
                mmap(NULL /* start anywhere */,
                     buf.length,
                     PROT_READ | PROT_WRITE /* required */,
                     MAP_SHARED /* recommended */,
                     fd, buf.m.offset);

            if (MAP_FAILED == buffers[n_buffers].start)
                errno_exit("mmap");
       }

    }

    void CameraV4l2::init_userp (unsigned int buffer_size) {

        struct v4l2_requestbuffers req;

        memset(&req, 0, sizeof(req));

        req.count  = 4;
        req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_USERPTR;

        if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
        {
            if (EINVAL == errno)
            {
                fprintf(stderr, "%s does not support "
                        "user pointer i/o\n", mDeviceName);
                exit(EXIT_FAILURE);
            }
            else
            {
                errno_exit("VIDIOC_REQBUFS");
            }
        }

        buffers = (buffer*)calloc(4, sizeof(*buffers));

        if (!buffers)
        {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < 4; ++n_buffers)
        {
            buffers[n_buffers].length = buffer_size;
            buffers[n_buffers].start = malloc(buffer_size);

            if (!buffers[n_buffers].start)
            {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
            }
        }
    }

#endif
