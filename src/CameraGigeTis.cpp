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
        mImgDepth = MONO8;
        mSaturateVal = 0;
        mGainMin = -1;
        mGainMax = -1;
        mExposureMin = -1;
        mExposureMax = -1;

        mExposureAvailable = true;
        mGainAvailable = true;
        mInputDeviceType = CAMERA;

    }

    vector<pair<int,string>> CameraGigeTis::getCamerasList() {

        vector<pair<int,string>> camerasList;

        // Retrieve a list with the video capture devices connected to the computer.
        pVidCapDevList = m_pGrabber->getAvailableVideoCaptureDevices();

        // Print available devices.
        for(int i = 0; i < pVidCapDevList->size(); i++) {

            LARGE_INTEGER iSerNum;
            if(pVidCapDevList->at(i).getSerialNumber(iSerNum.QuadPart) == false) iSerNum.QuadPart = 0;
            std::ostringstream ossSerNum;
            ossSerNum << std::hex << iSerNum.QuadPart;
            string SerNum = ossSerNum.str();

            pair<int,string> c;
            c.first = i;
            c.second = "NAME[" + pVidCapDevList->at(i).getName() + "] S/N[" + SerNum + "] SDK[TIS]";
            camerasList.push_back(c);

        }

        return camerasList;

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

    bool CameraGigeTis::setFpsToLowerValue() {

        try {

            // Get list of possible format.
            DShowLib::Grabber::tFrameRateListPtr VidFpsListPtr = m_pGrabber->getAvailableFrameRates();
            double chooseValue = 0.0;
            cout << "Available FPS : | " ;
            for(int i = 0; i < VidFpsListPtr->size(); i++) {

                double fps = Conversion::roundToNearest((1.0/((float)VidFpsListPtr->at(i) / 1000.0)), 0.25);
                cout << fps << " | ";
                if(chooseValue == 0.0) {
                    chooseValue = fps;
                }else {
                    if(fps < chooseValue) {
                        chooseValue = fps;
                    }
                }

            }
            cout << endl;

            if(chooseValue != 0.0) {
                mFPS = chooseValue;
                cout << ">> Fps setted to the lower value : " << chooseValue << endl;
                m_pGrabber->setFPS(chooseValue);
                return true;
            }

        }catch(exception& e) {

            BOOST_LOG_SEV(logger,critical) << "An error occured on set lower fps operation.";
            BOOST_LOG_SEV(logger,critical) << e.what();

        }

        return false;

    }

    bool CameraGigeTis::setFPS(double value) {

        try {

            // Get list of possible format.
            DShowLib::Grabber::tFrameRateListPtr VidFpsListPtr = m_pGrabber->getAvailableFrameRates();
            double chooseValue = 0.0;
            double resPrev = 0.0;
            cout << "Available FPS : | " ;
            for(int i = 0; i < VidFpsListPtr->size(); i++) {

                double fps = Conversion::roundToNearest((1.0/((float)VidFpsListPtr->at(i) / 1000.0)), 0.25);
                cout << fps << " | ";
                if(resPrev == 0.0) {
                    resPrev = abs(fps - value);
                    chooseValue = fps;
                }else {
                    if(resPrev > abs(fps - value)) {
                        resPrev = abs(fps - value);
                        chooseValue = fps;
                    }
                }

            }
            cout << endl;

            if(chooseValue != 0.0) {
                mFPS = chooseValue;
                cout << ">> Set fps to : " << chooseValue << endl;
                m_pGrabber->setFPS(chooseValue);
                return true;
            }

        }catch(exception& e) {

            BOOST_LOG_SEV(logger,critical) << "An error occured on set fps operation.";
            BOOST_LOG_SEV(logger,critical) << e.what();

        }

        return false;

    }

    bool CameraGigeTis::createDevice(int id){

        // Retrieve a list with the video capture devices connected to the computer.
        pVidCapDevList = m_pGrabber->getAvailableVideoCaptureDevices();

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

    bool CameraGigeTis::setPixelFormat(CamPixFmt format) {

        mImgDepth = format;

        vector<string> mono12, mono8;

        // Get list of possible format.
        DShowLib::Grabber::tVidFmtListPtr     VidFmtListPtr     = m_pGrabber->getAvailableVideoFormats();
        string dateDelimiter = " ";
        cout << "Available Format : " << endl;
        for(int i = 0; i < VidFmtListPtr->size(); i++) {

            string s = VidFmtListPtr->at(i).c_str();
            string s1 = s.substr(0, s.find(dateDelimiter));
            cout << "-> (" << Conversion::intToString(i) << ") " << VidFmtListPtr->at(i).c_str() << endl;

            if(s1 == "Y8" || s1 == "Y800"){

                mono8.push_back(VidFmtListPtr->at(i).c_str());

            }else if(s1 == "Y12" || s1 == "Y16"){

                mono12.push_back(VidFmtListPtr->at(i).c_str());

            }
        }

        cout << endl;

        switch(format){

            case MONO8 :

                if(mono8.size() == 0)
                    return false;

                m_pGrabber->setVideoFormat(mono8.front());//"Y8 (1280x960-1280x960)");

                // Set the image buffer format to eY800. eY800 means monochrome, 8 bits (1 byte) per pixel.
                // Let the sink create a matching MemBufferCollection with 1 buffer.
                pSink = DShowLib::FrameHandlerSink::create( DShowLib::eY800, NUMBER_OF_BUFFERS );

                break;

            case MONO12 :

                if(mono12.size() == 0)
                    return false;

                m_pGrabber->setVideoFormat(mono12.front());//"Y16 (1280x960-1280x960)");

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

    void CameraGigeTis::getExposureBounds(double &eMin, double &eMax) {

        DShowLib::tIVCDAbsoluteValuePropertyPtr pExposureRange;

        pExposureRange = NULL;

        DShowLib::tIVCDPropertyItemsPtr pItems = m_pGrabber->getAvailableVCDProperties();

        if( pItems != 0 ) {

            // Try to find the exposure item.
            DShowLib::tIVCDPropertyItemPtr pExposureItem = pItems->findItem( DShowLib::VCDID_Exposure );

            if( pExposureItem != 0 ) {

                // Try to find the value and auto elements
                DShowLib::tIVCDPropertyElementPtr pExposureValueElement = pExposureItem->findElement( DShowLib::VCDElement_Value );

                // If a value element exists, try to acquire a range interface
                if( pExposureValueElement != 0 ) {

                    pExposureValueElement->getInterfacePtr( pExposureRange );

                    eMin = pExposureRange->getRangeMin() * 1000000.0; // in us
                    eMax = pExposureRange->getRangeMax() * 1000000.0; // in us

                }
            }
        }
    }

    void CameraGigeTis::getGainBounds(int &gMin, int &gMax) {

        // Get properties.
        _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr pItems = m_pGrabber->getAvailableVCDProperties();

        gMin  = (int)getPropertyRangeMin(DShowLib::VCDID_Gain, pItems);
        gMax  = (int)getPropertyRangeMax(DShowLib::VCDID_Gain, pItems);

    }

    // http://www.theimagingsourceforums.com/faq.php?faq=ic_programming
    bool CameraGigeTis::setExposureTime(double value) {

        // Conversion in seconds
        value = value / 1000000.0;

        bool bOK = false;

        DShowLib::tIVCDAbsoluteValuePropertyPtr pExposureRange;
        DShowLib::tIVCDSwitchPropertyPtr pExposureAuto;

        pExposureRange = NULL;
        pExposureAuto = NULL;

        DShowLib::tIVCDPropertyItemsPtr pItems = m_pGrabber->getAvailableVCDProperties();

        if( pItems != 0 ) {
            // Try to find the exposure item.
            DShowLib::tIVCDPropertyItemPtr pExposureItem = pItems->findItem( DShowLib::VCDID_Exposure );
            if( pExposureItem != 0 ) {
                // Try to find the value and auto elements
                DShowLib::tIVCDPropertyElementPtr pExposureValueElement = pExposureItem->findElement( DShowLib::VCDElement_Value );
                DShowLib::tIVCDPropertyElementPtr pExposureAutoElement = pExposureItem->findElement( DShowLib::VCDElement_Auto );

                // If an auto element exists, try to acquire a switch interface
                if( pExposureAutoElement != 0 ) {
                    pExposureAutoElement->getInterfacePtr( pExposureAuto );
                    pExposureAuto->setSwitch(false); // Disable auto, otherwise we can not set exposure.
                }

                // If a value element exists, try to acquire a range interface
                if( pExposureValueElement != 0 ) {

                    pExposureValueElement->getInterfacePtr( pExposureRange );

                    mExposureMin = pExposureRange->getRangeMin();
                    mExposureMax = pExposureRange->getRangeMax();

                    cout << "Available exposure range : [ " << mExposureMin << " - "<< mExposureMax << " ]" << endl;

                    if ( value <= mExposureMin ) {
                        value = mExposureMin + 0.000010;
                        BOOST_LOG_SEV(logger,warning) << "EXPOSURE TIME setted to " << value << ". Available range [" << mExposureMin << " - " << mExposureMax<< "]";
                    } else if( value >= mExposureMax ) {
                        value = mExposureMax;
                        BOOST_LOG_SEV(logger,warning) << "EXPOSURE TIME setted to " << value << ". Available range [" << mExposureMin << " - " << mExposureMax<< "]";
                    }

                    // Here we set the the exposure value.
                    cout << ">> Set exposure time to : " << value << endl;
                    pExposureRange->setValue( value);
                    mExposure = value * 1000000.0;
                    bOK = true;
                }
            }
        }

        return bOK;
    }

    void CameraGigeTis::getAvailablePixelFormats() {

        if(m_pGrabber != NULL) {

            vector<string> pixfmt;
            EParser<CamPixFmt> fmt;
            DShowLib::Grabber::tVidFmtListPtr pVidFmtList  = m_pGrabber->getAvailableVideoFormats();

            // List the available video formats.
            for(DShowLib::Grabber::tVidFmtListPtr::value_type::iterator it = pVidFmtList->begin(); it != pVidFmtList->end(); ++it)
            {
                string pf = it->c_str();

                if(pf.find("Y8") != std::string::npos) {
                    pixfmt.push_back("MONO8");
                }

                if(pf.find("Y16") != std::string::npos) {
                    pixfmt.push_back("MONO12");
                }

            }

            std::cout << endl <<  ">> Available pixel formats :" << endl;

            for( int i = 0; i != pixfmt.size(); i++ ) {
                if(fmt.isEnumValue(pixfmt.at(i))) {
                    std::cout << "- " << pixfmt.at(i) << " available --> ID : " << fmt.parseEnum(pixfmt.at(i)) << endl;
                }
            }
        }

    }

    bool CameraGigeTis::setGain(int value) {

        bool bOK = false;
        DShowLib::tIVCDSwitchPropertyPtr pGainAuto;

        pGainAuto = NULL;

        DShowLib::tIVCDPropertyItemsPtr pItems = m_pGrabber->getAvailableVCDProperties();

        if( pItems != 0 ) {

            // Try to find the gain item.
            DShowLib::tIVCDPropertyItemPtr pGainItem = pItems->findItem( DShowLib::VCDID_Gain );

            if( pGainItem != 0 ) {

                // Try to find auto elements
                DShowLib::tIVCDPropertyElementPtr pGainAutoElement = pGainItem->findElement( DShowLib::VCDElement_Auto );

                // If an auto element exists, try to acquire a switch interface
                if( pGainAutoElement != 0 ) {
                    pGainAutoElement->getInterfacePtr( pGainAuto );
                    pGainAuto->setSwitch(false); // Disable auto, otherwise we can not set gain.
                }

                mGainMin  = (int)getPropertyRangeMin(DShowLib::VCDID_Gain, pItems);
                mGainMax  = (int)getPropertyRangeMax(DShowLib::VCDID_Gain, pItems);

                cout << "Available gain range : [ " << mGainMin << " - "<< mGainMax << " ]" << endl;

                if(value > mGainMax || value < mGainMin){

                    BOOST_LOG_SEV(logger,warning) << "Fail to set GAIN. Available range value is " << mGainMin << " to " << mGainMax;
                    cout << endl << ">> Fail to set GAIN. Available range value is " << mGainMin << " to " << mGainMax << endl;
                    value = mGainMin;
                }

                setPropertyValue(DShowLib::VCDID_Gain, (long)value, pItems);
                cout << ">> Set gain to : " << value << endl;
                mGain = value;
                bOK = true;

            }
        }
        return bOK;

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

        return true;

    }

    bool CameraGigeTis::acqStart() {

        if (!m_pGrabber->isLive()) {

            m_pGrabber->startLive(false);

        }

        pSink->snapImages(1,(DWORD)-1);

        return true;

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
            newFrame.mFormat = mImgDepth;
            newFrame.mSaturatedValue = mSaturateVal;

            newFrame.mFrameNumber = mFrameCounter;
            mFrameCounter++;

            return true;

        }

        return false;

    }

    bool CameraGigeTis::setSize(int width, int height, bool customSize) {

        if(customSize){

        }else{

        }

        return true;

    }

    void CameraGigeTis::acqStop() {

        m_pGrabber->stopLive();
        m_pGrabber->closeDev();

    }

    void CameraGigeTis::grabCleanse() {

        if(m_pGrabber!=NULL)
            m_pGrabber->closeDev();

    }

    double CameraGigeTis::getExposureTime() {

        DShowLib::tIVCDAbsoluteValuePropertyPtr pExposureRange;

        pExposureRange = NULL;

        DShowLib::tIVCDPropertyItemsPtr pItems = m_pGrabber->getAvailableVCDProperties();

        if( pItems != 0 ) {

            // Try to find the exposure item.

            DShowLib::tIVCDPropertyItemPtr pExposureItem = pItems->findItem( DShowLib::VCDID_Exposure );
            if( pExposureItem != 0 ) {

                // Try to find the value and auto elements
                DShowLib::tIVCDPropertyElementPtr pExposureValueElement = pExposureItem->findElement( DShowLib::VCDElement_Value );

                // If a value element exists, try to acquire a range interface
                if( pExposureValueElement != 0 ) {

                    pExposureValueElement->getInterfacePtr( pExposureRange );
                    return (pExposureRange->getValue()/1000000.0);

                }
            }
        }

        return 0.0;

    }

    bool CameraGigeTis::getPixelFormat(CamPixFmt &format) {

        if(m_pGrabber->getVideoFormat().getBitsPerPixel() == 8) {

            format = MONO8;

        }else if(m_pGrabber->getVideoFormat().getBitsPerPixel() == 16 || m_pGrabber->getVideoFormat().getBitsPerPixel() == 12) {

            format = MONO12;

        }else {

            return false;

        }

        return true;

    }

    bool CameraGigeTis::grabSingleImage(Frame &frame, int camID) {

        if(!createDevice(camID))
            return false;

        if(!setPixelFormat(frame.mFormat))
            return false;

        // Set lower fps value.
        if(!setFpsToLowerValue())
            return false;

        if(!setExposureTime(frame.mExposure))
            return false;

        if(!setGain(frame.mGain))
            return false;

        cout << ">> Acquisition in progress... (Please wait)" << endl;

        // We use snap mode.
        pSink->setSnapMode(true);

        // Set the sink.
        m_pGrabber->setSinkType(pSink);

        // Disable live mode.
        m_pGrabber->prepareLive(false);

        // Retrieve the output type and dimension of the handler sink.
        DShowLib::FrameTypeInfo info;
        pSink->getOutputFrameType(info);

        Mat newImg;
        DShowLib ::Error e;

        //Timestamping.
        boost::posix_time::ptime time;

        switch(info.getBitsPerPixel()){

            case 8 :

                {

                    newImg = Mat(info.dim.cy, info.dim.cx, CV_8UC1, Scalar(0));
                    BYTE* pBuf[1];
                    // Allocate image buffers of the above calculate buffer size.
                    pBuf[0] = new BYTE[info.buffersize];

                    // Create a new MemBuffer collection that uses our own image buffers.
                    pCollection = DShowLib::MemBufferCollection::create( info, 1, pBuf );

                    if( pCollection == 0 || !pSink->setMemBufferCollection(pCollection)){

                        BOOST_LOG_SEV(logger,critical) << "Could not set the new MemBufferCollection.";

                    }else {

                        m_pGrabber->startLive();

                        e = pSink->snapImages(1);

                        if( !e.isError()) {
                            time = boost::posix_time::microsec_clock::universal_time();
                            memcpy(newImg.ptr(), pBuf[0], info.buffersize);
                        }
                    }
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

                        BOOST_LOG_SEV(logger,critical) << "Could not set the new MemBufferCollection.";

                    }else {

                        m_pGrabber->startLive(false);

                        e = pSink->snapImages(1);

                        if( !e.isError()) {

                            time = boost::posix_time::microsec_clock::universal_time();
                            memcpy(newImg.ptr(), pBuf[0], info.buffersize);

                            // Shift.
                            unsigned short * ptr;
                            for(int i = 0; i < newImg.rows; i++){
                                ptr = newImg.ptr<unsigned short>(i);
                                for(int j = 0; j < newImg.cols; j++){
                                    ptr[j] = ptr[j] >> 4;
                                }
                            }
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

        if( !e.isError()) {

            newImg.copyTo(frame.mImg);
            frame.mDate = TimeDate::splitIsoExtendedDate(to_iso_extended_string(time));
            frame.mFps = 0;

            return true;
        }

        return false;
    }

    CameraGigeTis::~CameraGigeTis(){

        DShowLib::ExitLibrary();
        if(m_pGrabber != NULL)
            delete m_pGrabber;

    }

#endif
