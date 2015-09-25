/*
                            CameraGigeTis.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*   This file is part of:   freeture
*
*   Copyright:      (C) 2014-2015 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
*   Last modified:      21/01/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    CameraGigeTis.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    21/01/2015
* \brief   Use Imaging source sdk to pilot GigE Cameras.
*          https://wiki.gnome.org/action/show/Projects/Aravis?action=show&redirect=Aravis
*/

#include "CameraGigeTis.h"

#ifdef WINDOWS

    boost::log::sources::severity_logger< LogSeverityLevel >  CameraGigeTis::logger;

    CameraGigeTis::Init CameraGigeTis::initializer;

    CameraGigeTis::CameraGigeTis(){

        if(!DShowLib::InitLibrary())
            throw "Fail DShowLib::InitLibrary().";

        m_pGrabber = new DShowLib::Grabber();
        mFrameCounter = 0;
        mGain = 0;
        mExposure = 0;
        mFPS = 30;
        mImgDepth = MONO_8;
        mSaturateVal = 0;
        mGainMin = -1;
        mGainMax = -1;
        mExposureMin = -1;
        mExposureMax = -1;

    }

    bool CameraGigeTis::listCameras(){

        // Retrieve a list with the video capture devices connected to the computer.
        pVidCapDevList = m_pGrabber->getAvailableVideoCaptureDevices();

        cout << endl << "-------------- GIGE CAMERAS WITH TIS -----------" << endl << endl;

        // Print available devices.
        for(int i = 0; i < pVidCapDevList->size(); i++) {

            LARGE_INTEGER iSerNum;
            if(pVidCapDevList->at(0).getSerialNumber(iSerNum.QuadPart) == false) iSerNum.QuadPart = 0;
            std::ostringstream ossSerNum;
            ossSerNum << std::hex << iSerNum.QuadPart;
            string SerNum = ossSerNum.str();

            cout << "-> ID[" << i << "]  NAME[" << pVidCapDevList->at(0).c_str() << "]  S/N[" << SerNum <<"]" << endl;

        }

        if(pVidCapDevList->size() == 0) {
            cout << "-> No cameras detected..." << endl;
            cout << endl << "------------------------------------------------" << endl << endl;
            return false;
        }

        cout << endl << "------------------------------------------------" << endl << endl;

        return true;

    }

    // https://valelab.ucsf.edu/svn/micromanager2/branches/micromanager1.3/DeviceAdapters/TISCam/SimplePropertyAccess.cpp
    DShowLib::tIVCDRangePropertyPtr CameraGigeTis::getPropertyRangeInterface( _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr& pItems, const GUID& id ){

        GUID itemID = id;
        GUID elemID = DShowLib::VCDElement_Value;

        DShowLib::tIVCDPropertyElementPtr pFoundElement = pItems->findElement( itemID, elemID );

        if( pFoundElement != 0 ){

            DShowLib::tIVCDRangePropertyPtr pRange;

            if( pFoundElement->getInterfacePtr( pRange ) != 0 ) {
                return pRange;
            }
        }
        return 0;
    }

    bool CameraGigeTis::propertyIsAvailable( const GUID& id, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer ){

        return m_pItemContainer->findItem( id ) != 0;

    }

    long CameraGigeTis::getPropertyValue( const GUID& id, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer ){

        long rval = 0;
        DShowLib::tIVCDRangePropertyPtr pRange = getPropertyRangeInterface( m_pItemContainer, id );
        if( pRange != 0 ){
            rval = pRange->getValue();
        }
        return rval;

    }

    void CameraGigeTis::setPropertyValue( const GUID& id, long val, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer ){

        DShowLib::tIVCDRangePropertyPtr pRange = getPropertyRangeInterface( m_pItemContainer, id );

        if( pRange != 0 ) {
            pRange->setValue( val );
        }
    }

    long CameraGigeTis::getPropertyRangeMin( const GUID& id, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer ){

        long rval = 0;
        DShowLib::tIVCDRangePropertyPtr pRange = getPropertyRangeInterface( m_pItemContainer, id );

        if( pRange != 0 ){
            rval = pRange->getRangeMin();
        }
        return rval;
    }

    long CameraGigeTis::getPropertyRangeMax(const GUID& id, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer){

        long rval = 0;
        DShowLib:: tIVCDRangePropertyPtr pRange = getPropertyRangeInterface( m_pItemContainer, id );

        if( pRange != 0 ) {
            rval = pRange->getRangeMax();
        }
        return rval;
    }

    /*bool CameraGigeTis::getAvailableFormat(){

        // Open device first.

        // Query for all available video formats.
        DShowLib::Grabber::tVidFmtListPtr pVidFmtList = m_pGrabber->getAvailableVideoFormats();

        if(pVidFmtList == 0){

            std::cerr << "Error: " << m_pGrabber->getLastError().toString() << std::endl;
            return false;

        }else{

            unsigned int counter = 0;
            // List the available video formats.
            for(DShowLib::Grabber::tVidFmtList::iterator it = pVidFmtList->begin(); it != pVidFmtList->end();++it){
	            std::cout << "\t[" << counter++ << "] " << it->toString() << std::endl;
            }

            return true;

        }

        // Close device.

    }*/

    bool CameraGigeTis::setFPS(double value) {

        mFPS = value;
        return m_pGrabber->setFPS((double)value);

    }

    bool CameraGigeTis::createDevice(int id){

        if(pVidCapDevList == 0 || pVidCapDevList->empty()){

            BOOST_LOG_SEV(logger,fail) << "No device available.";
            return false;

        }else {

            if(((id+1)>pVidCapDevList->size()) || id < 0) {

                BOOST_LOG_SEV(logger,fail) << "Camera ID not correct. Can't be found.";
                return false;

            }
            
            // Open the selected video capture device.
            m_pGrabber->openDev(pVidCapDevList->at(id));
            return true;

        }
    }

    bool CameraGigeTis::setPixelFormat(CamBitDepth format) {

        mImgDepth = format;

        switch(format){

            case MONO_8 :

                m_pGrabber->setVideoFormat("Y8 (1280x960-1280x960)");

                // Set the image buffer format to eY800. eY800 means monochrome, 8 bits (1 byte) per pixel.
                // Let the sink create a matching MemBufferCollection with 1 buffer.
                pSink = DShowLib::FrameHandlerSink::create( DShowLib::eY800, NUMBER_OF_BUFFERS );

                break;

            case MONO_12 :

                m_pGrabber->setVideoFormat("Y16 (1280x960-1280x960)");

                // Disable overlay.
                // http://www.theimagingsourceforums.com/archive/index.php/t-319880.html
                m_pGrabber->setOverlayBitmapPathPosition(DShowLib::ePP_NONE);

                // Set the image buffer format to eY16. eY16 means monochrome, 16 bits (2 byte) per pixel.
                // Let the sink create a matching MemBufferCollection with 1 buffer.
                pSink = DShowLib::FrameHandlerSink::create( DShowLib::eY16, NUMBER_OF_BUFFERS );

                break;

            default:

                return false;

                break;
        }

        return true;

    }

    bool CameraGigeTis::getFPS(double &value){

        value = m_pGrabber->getFPS();
        return true;

    }

    void CameraGigeTis::getExposureBounds(int &eMin, int &eMax) {

        // Get properties.
        _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr pItems = m_pGrabber->getAvailableVCDProperties();

        eMin  = (int)getPropertyRangeMin(DShowLib::VCDID_Exposure, pItems);
        eMax  = (int)getPropertyRangeMax(DShowLib::VCDID_Exposure, pItems);

    }

    void CameraGigeTis::getGainBounds(int &gMin, int &gMax) {

        // Get properties.
        _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr pItems = m_pGrabber->getAvailableVCDProperties();

        gMin  = (int)getPropertyRangeMin(DShowLib::VCDID_Gain, pItems);
        gMax  = (int)getPropertyRangeMax(DShowLib::VCDID_Gain, pItems);

    }

    bool CameraGigeTis::setExposureTime(int value) {

        _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr pItems = m_pGrabber->getAvailableVCDProperties();

        if(mExposureMin == -1 && mExposureMax == -1) {

            mExposureMin = (int)getPropertyRangeMin(DShowLib::VCDID_Exposure, pItems);
            mExposureMax = (int)getPropertyRangeMax(DShowLib::VCDID_Exposure, pItems);

    }

        if(value > mExposureMax || value < mExposureMin){

            cout << endl << ">> Fail to set exposure. Available range value is " << mExposureMin << " to " << mExposureMax << endl;
            BOOST_LOG_SEV(logger,fail) << "Fail to set EXPOSURE TIME. Available range value is " << mExposureMin << " to " << mExposureMax;
            return false;
        }

        setPropertyValue(DShowLib::VCDID_Exposure, (long)value, pItems);
        mExposure = value;
        return true;

    }

    bool CameraGigeTis::setGain(int value) {

        _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr pItems = m_pGrabber->getAvailableVCDProperties();

        if(mGainMin == -1 && mGainMax == -1) {

            mGainMin  = (int)getPropertyRangeMin(DShowLib::VCDID_Gain, pItems);
            mGainMax  = (int)getPropertyRangeMax(DShowLib::VCDID_Gain, pItems);

        }

        if(value > mGainMax || value < mGainMin){

            BOOST_LOG_SEV(logger,fail) << "Fail to set GAIN. Available range value is " << mGainMin << " to " << mGainMax;
            cout << endl << ">> Fail to set GAIN. Available range value is " << mGainMin << " to " << mGainMax << endl;

            return false;

        }

        setPropertyValue(DShowLib::VCDID_Gain, (long)value, pItems);
        mGain = value;
        return true;

    }

    bool CameraGigeTis::grabInitialization() {

        // Set the sink.
        m_pGrabber->setSinkType(pSink);

        // We use snap mode.
        pSink->setSnapMode(true);

        // Prepare the live mode, to get the output size if the sink.
        if(!m_pGrabber->prepareLive(false)){

            std::cerr << "Could not render the VideoFormat into a eY800 sink.";
            return false;
        }

        // Retrieve the output type and dimension of the handler sink.
        // The dimension of the sink could be different from the VideoFormat, when
        // you use filters.
        DShowLib::FrameTypeInfo info;
        pSink->getOutputFrameType(info);

        // Allocate NUMBER_OF_BUFFERS image buffers of the above (info) buffer size.
        for (int ii = 0; ii < NUMBER_OF_BUFFERS; ++ii) {
            pBuf[ii] = new BYTE[info.buffersize];
            assert(pBuf[ii]);
        }

        // Create a new MemBuffer collection that uses our own image buffers.
        pCollection = DShowLib::MemBufferCollection::create(info, NUMBER_OF_BUFFERS, pBuf);
        if (pCollection == 0) return false;
        if (!pSink->setMemBufferCollection(pCollection)) return false;

        if (!m_pGrabber->startLive(false)) return false;


    }

    void CameraGigeTis::acqStart() {

    if (!m_pGrabber->isLive()) {

        m_pGrabber->startLive(false);

    }

    pSink->snapImages(1,(DWORD)-1);

    }

    bool CameraGigeTis::grabImage(Frame &newFrame) {

        Mat newImg;

        // Retrieve the output type and dimension of the handler sink.
        // The dimension of the sink could be different from the VideoFormat, when
        // you use filters.
        DShowLib::FrameTypeInfo info;
        pSink->getOutputFrameType(info);

        //Timestamping.
        boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();

        switch(info.getBitsPerPixel()){

            case 8 :

                {

                    newImg = Mat(info.dim.cy, info.dim.cx, CV_8UC1, Scalar(0));
                    pSink->snapImages(1,(DWORD)-1);
                    memcpy(newImg.ptr(), pBuf[0], info.buffersize);

                }

                break;

            case 16 :

                {

                    newImg = Mat(info.dim.cy, info.dim.cx, CV_16UC1, Scalar(0));

                    pSink->snapImages(1,(DWORD)-1);

                    memcpy(newImg.ptr(), pBuf[0], info.buffersize);

                    unsigned short * ptr;

                    double t = (double)getTickCount();

                    for(int i = 0; i < newImg.rows; i++){

                        ptr = newImg.ptr<unsigned short>(i);

                        for(int j = 0; j < newImg.cols; j++){

                            ptr[j] = ptr[j] >> 4;

                        }
                    }
                }

                break;

            default:

                    return false;

                break;
        }

        if(newImg.data) {

            newFrame = Frame(newImg, mGain, mExposure, to_iso_extended_string(time));

            newFrame.mFps = mFPS;
            newFrame.mBitDepth = mImgDepth;
            newFrame.mSaturatedValue = mSaturateVal;

            newFrame.mFrameNumber = mFrameCounter;
            mFrameCounter++;

            return true;

        }

        return false;

    }

    void CameraGigeTis::acqStop() {

        m_pGrabber->stopLive();
        m_pGrabber->closeDev();

    }

    void CameraGigeTis::grabCleanse() {

        if(m_pGrabber!=NULL)
            m_pGrabber->closeDev();

    }

    int CameraGigeTis::getExposureTime() {

        _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr pItems = m_pGrabber->getAvailableVCDProperties();

        return (int)getPropertyValue(DShowLib::VCDID_Exposure,pItems);

    }

    bool CameraGigeTis::getPixelFormat(CamBitDepth &format) {

        if(m_pGrabber->getVideoFormat().getBitsPerPixel() == 8) {

            format = MONO_8;

        }else if(m_pGrabber->getVideoFormat().getBitsPerPixel() == 16 || m_pGrabber->getVideoFormat().getBitsPerPixel() == 12) {

            format = MONO_12;

        }else {

            return false;

        }

        return true;

    }

    bool CameraGigeTis::grabSingleImage(Frame &frame, int camID) {

        listCameras();

        if(createDevice(camID)) {

        if(!setPixelFormat(frame.mBitDepth))
            return false;

        if(!setExposureTime(frame.mExposure))
            return false;

        if(!setGain(frame.mGain))
            return false;

            cout << ">> Acquisition in progress... (Please wait)" << endl;

            // Set the sink.
            m_pGrabber->setSinkType(pSink);

            // We use snap mode.
            pSink->setSnapMode(true);

            // Prepare the live mode, to get the output size if the sink.
            if(!m_pGrabber->prepareLive(false)){

                std::cerr << "Could not render the VideoFormat into a eY800 sink.";
                return false;
            }

            // Retrieve the output type and dimension of the handler sink.
            // The dimension of the sink could be different from the VideoFormat, when
            // you use filters.
            DShowLib::FrameTypeInfo info;
            pSink->getOutputFrameType(info);

            Mat newImg;

            switch(info.getBitsPerPixel()){

                case 8 :

                    {

                        newImg = Mat(info.dim.cy, info.dim.cx, CV_8UC1, Scalar(0));
                        //BYTE* pBuf[1];
                        // Allocate image buffers of the above calculate buffer size.
                        pBuf[0] = new BYTE[info.buffersize];

                        // Create a new MemBuffer collection that uses our own image buffers.
                        pCollection = DShowLib::MemBufferCollection::create( info, 1, pBuf );
                        if( pCollection == 0 || !pSink->setMemBufferCollection(pCollection)){

                            std::cerr << "Could not set the new MemBufferCollection, because types do not match.";
                            return false;

                        }

                        m_pGrabber->startLive(false);

                        pSink->snapImages(1);

                        memcpy(newImg.ptr(), pBuf[0], info.buffersize);

                    }

                    break;

                case 16 :

                    {

                        newImg = Mat(info.dim.cy, info.dim.cx, CV_16UC1, Scalar(0));
                        BYTE * pBuf[1];
                        // Allocate image buffers of the above calculate buffer size.
                        pBuf[0] = new BYTE[info.buffersize];

                        // Create a new MemBuffer collection that uses our own image buffers.
                        pCollection = DShowLib::MemBufferCollection::create(info, 1, pBuf);
                        if(pCollection == 0 || !pSink->setMemBufferCollection(pCollection)){

                            std::cerr << "Could not set the new MemBufferCollection, because types do not match.";
                            return false;

                        }

                        m_pGrabber->startLive(false);

                        pSink->snapImages(1);

                        memcpy(newImg.ptr(), pBuf[0], info.buffersize);

                        unsigned short * ptr;

                        double t = (double)getTickCount();

                        for(int i = 0; i < newImg.rows; i++){

                            ptr = newImg.ptr<unsigned short>(i);

                            for(int j = 0; j < newImg.cols; j++){

                                ptr[j] = ptr[j] >> 4;

                            }
                        }

                    }

                    break;

                default:

                        return false;

                    break;
            }

            m_pGrabber->stopLive();

            m_pGrabber->closeDev();

            //Timestamping.
            boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();

            newImg.copyTo(frame.mImg);
            frame.mDate = TimeDate::splitIsoExtendedDate(to_iso_extended_string(time));
            frame.mFps = 0;

            return true;

        }

        return false;
    }

    CameraGigeTis::~CameraGigeTis(){

         delete m_pGrabber;

    }

#endif
