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

    #define CLEAR(x) memset(&(x), 0, sizeof(x))

    enum io_method
    {
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

    struct buffer          *buffers;
    unsigned int     n_buffers;
    int              out_buf = 1;
    int              force_format = 1;
    int              frame_count = 10;
    int              frame_number = 0;


    boost::log::sources::severity_logger< LogSeverityLevel >  CameraV4l2::logger;
    CameraV4l2::Init CameraV4l2::initializer;

    CameraV4l2::CameraV4l2(){

        io_method io = IO_METHOD_MMAP;
        fd = -1;
        out_buf = 1;
        force_format = 1;
        frame_count = 10;
        frame_number = 0;

    }

    CameraV4l2::~CameraV4l2(){}

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

        struct v4l2_cropcap cropcap = {0};
        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl (fd, VIDIOC_CROPCAP, &cropcap)) {
            perror("Querying Cropping Capabilities");
            return false;
        }

        printf( "Camera Cropping :\n"
                "  Bounds: %dx%d+%d+%d\n"
                "  Default: %dx%d+%d+%d\n"
                "  Aspect: %d/%d\n",
                cropcap.bounds.width, cropcap.bounds.height, cropcap.bounds.left, cropcap.bounds.top,
                cropcap.defrect.width, cropcap.defrect.height, cropcap.defrect.left, cropcap.defrect.top,
                cropcap.pixelaspect.numerator, cropcap.pixelaspect.denominator);

        int support_grbg10 = 0;

        struct v4l2_fmtdesc fmtdesc = {0};
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        char fourcc[5] = {0};
        char c, e;
        printf("  FMT : CE Desc\n--------------------\n");
        while (0 == xioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc)) {
            strncpy(fourcc, (char *)&fmtdesc.pixelformat, 4);
            if (fmtdesc.pixelformat == V4L2_PIX_FMT_SGRBG10)
                support_grbg10 = 1;
            c = fmtdesc.flags & 1? 'C' : ' ';
            e = fmtdesc.flags & 2? 'E' : ' ';
            printf("  %s: %c%c %s\n", fourcc, c, e, fmtdesc.description);
            fmtdesc.index++;
        }

        struct v4l2_format fmt = {0};
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
        printf( "Selected Camera Mode:\n"
                "  Width: %d\n"
                "  Height: %d\n"
                "  PixFmt: %s\n"
                "  Field: %d\n",
                fmt.fmt.pix.width,
                fmt.fmt.pix.height,
                fourcc,
                fmt.fmt.pix.field);

        return true;

    };

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
        deviceName = deviceNameStr.c_str();

        struct stat st;

        if (-1 == stat(deviceName, &st)) {
            fprintf(stderr, "Cannot identify '%s': %d, %s\n",
                    deviceName, errno, strerror(errno));
            return false;
            exit(EXIT_FAILURE);
        }

        if (!S_ISCHR(st.st_mode)) {
            fprintf(stderr, "%s is no device\n", deviceName);
            return false;
            exit(EXIT_FAILURE);
        }

        fd = open(deviceName, O_RDWR /* required */ | O_NONBLOCK, 0);

        if (-1 == fd) {
            fprintf(stderr, "Cannot open '%s': %d, %s\n",
                    deviceName, errno, strerror(errno));
            return false;
            exit(EXIT_FAILURE);
        }

        return true;

    }

    bool CameraV4l2::getDeviceNameById(int id, string &device){

        return false;

    }

    bool CameraV4l2::grabInitialization(){

        int width = 1024;
        int height = 768;

        struct v4l2_capability cap;
        struct v4l2_cropcap cropcap;
        struct v4l2_crop crop;
        struct v4l2_format fmt;


        /*struct v4l2_streamparm setfps;
        setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        setfps.parm.capture.timeperframe.numerator = 4;
        setfps.parm.capture.timeperframe.denominator = 15;
        ioctl(fd, VIDIOC_S_PARM, &setfps);*/

        unsigned int min;

        if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap))
        {
            if (EINVAL == errno)
            {
                fprintf(stderr,
                        "%s is no V4L2 device\n",
                        deviceName);
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
                    deviceName);
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
                            deviceName);
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
                            deviceName);
                    exit(EXIT_FAILURE);
                }
                break;
            }
        }


        /* Select video input, video standard and tune here. */


        CLEAR(cropcap);

        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap))
        {
            crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            crop.c = cropcap.defrect; /* reset to default */

            if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop))
            {
                switch (errno)
                {
                    case EINVAL:
                        /* Cropping not supported. */
                        break;
                    default:
                        /* Errors ignored. */
                        break;
                }
            }
        }
        else
        {
            /* Errors ignored. */
        }


        CLEAR(fmt);

        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (force_format)
        {
            fmt.fmt.pix.width       = width;
            fmt.fmt.pix.height      = height;
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
            fmt.fmt.pix.field       = V4L2_FIELD_NONE;

            if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
                errno_exit("VIDIOC_S_FMT");

            /* Note VIDIOC_S_FMT may change width and height. */
        }
        else
        {
            /* Preserve original settings as set by v4l2-ctl for example */
            if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
                errno_exit("VIDIOC_G_FMT");
        }

        /* Buggy driver paranoia. */
        min = fmt.fmt.pix.width * 2;
        if (fmt.fmt.pix.bytesperline < min)
            fmt.fmt.pix.bytesperline = min;
        min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
        if (fmt.fmt.pix.sizeimage < min)
            fmt.fmt.pix.sizeimage = min;

        switch (io)
        {
            case IO_METHOD_READ:
                init_read(fmt.fmt.pix.sizeimage);
                break;

            case IO_METHOD_MMAP:
                init_mmap();
                break;

            case IO_METHOD_USERPTR:
                init_userp(fmt.fmt.pix.sizeimage);
                break;
        }

        return true;

    }

    void CameraV4l2::grabCleanse(){

        // Uninit device

        unsigned int i;

        switch (io)
        {
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

        // Close device

        if (-1 == close(fd))
            errno_exit("close");

        fd = -1;

    }

    void CameraV4l2::acqStart(){

        unsigned int i;
        enum v4l2_buf_type type;

        switch (io)
        {
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

                    CLEAR(buf);
                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    buf.memory = V4L2_MEMORY_MMAP;
                    buf.index = i;

                    if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                    {
                        errno_exit("VIDIOC_QBUF");
                    }
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
                {
                    errno_exit("VIDIOC_STREAMON");
                }
                break;
            }
            case IO_METHOD_USERPTR:
            {
                for (i = 0; i < n_buffers; ++i)
                {
                    struct v4l2_buffer buf;

                    CLEAR(buf);
                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    buf.memory = V4L2_MEMORY_USERPTR;
                    buf.index = i;
                    buf.m.userptr = (unsigned long)buffers[i].start;
                    buf.length = buffers[i].length;

                    if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                    {
                        errno_exit("VIDIOC_QBUF");
                    }
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
                {
                    errno_exit("VIDIOC_STREAMON");
                }
                break;
            }
        }
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

        Mat img = Mat(768,1024,CV_8UC1, Scalar(0));
        size_t s = 1024*768;

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
              exit(EXIT_FAILURE);
            }

            if(read_frame()) {
                grabSuccess = true;
                break;
            }
            /* EAGAIN - continue select loop. */
        }

        if(grabSuccess) {

            ImageBuffer = (unsigned char*)buffers[buf.index].start;

            if(ImageBuffer != NULL) {

                memcpy(img.ptr(), ImageBuffer, s);

                boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();

                newFrame = Frame(img, 0, 0, to_iso_extended_string(time));
                //newFrame.mFps = fps;
                newFrame.mBitDepth = MONO_8;
                newFrame.mSaturatedValue = 255;
                //newFrame.mFrameNumber = frameCounter;

                return true;

            }

        }

        return false;

    }

    bool CameraV4l2::grabSingleImage(Frame &frame, int camID){

        return false;

    }

    void CameraV4l2::getExposureBounds(int &eMin, int &eMax){


    }

    int CameraV4l2::getExposureTime(){

        return 0;

    }

    void CameraV4l2::getGainBounds(int &gMin, int &gMax){

        double gainMin = 0.0;
        double gainMax = 0.0;


        //gMin = gainMin;
        //gMax = gainMax;

    }

    bool CameraV4l2::getPixelFormat(CamBitDepth &format){


        return true;
    }

    int CameraV4l2::getFrameWidth(){

        int w = 0, h = 0;

        return w;

    }

    int CameraV4l2::getFrameHeight(){

        int w = 0, h = 0;


        return h;

    }

    int CameraV4l2::getFPS(void){

        return 0;

    }

    string CameraV4l2::getModelName(){

        return "";

    }
    bool CameraV4l2::setExposureTime(int val){

        double expMin, expMax;

        return false;
    }

    bool CameraV4l2::setGain(int val){

        double gMin, gMax;

        return false;

    }

    bool CameraV4l2::setFPS(int fps){

        return false;

    }

    bool CameraV4l2::setPixelFormat(CamBitDepth depth){

        return false;

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

                printf("IO_METHOD_READ\n");

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
                printf("IO_METHOD_MMAP\n");

                CLEAR(buf);

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

                printf("IO_METHOD_USERPTR\n");
                CLEAR(buf);

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

        CLEAR(req);

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
        {
            if (EINVAL == errno)
            {
                fprintf(stderr, "%s does not support "
                        "memory mapping\n", deviceName);
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
                    deviceName);
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

            CLEAR(buf);

            buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory      = V4L2_MEMORY_MMAP;
            buf.index       = n_buffers;

            if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
                errno_exit("VIDIOC_QUERYBUF");

            buffers[n_buffers].length = buf.length;
            printf("buf.length: %d",buf.length);
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

        CLEAR(req);

        req.count  = 4;
        req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_USERPTR;

        if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
        {
            if (EINVAL == errno)
            {
                fprintf(stderr, "%s does not support "
                        "user pointer i/o\n", deviceName);
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
