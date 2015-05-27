/*
							CameraGigeSdkIc.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau -- FRIPON-GEOPS-UPSUD
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
*	Last modified:		21/01/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    CameraGigeSdkIc.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    21/01/2015
* \brief   Use Imaging source sdk to pilot GigE Cameras.
*          https://wiki.gnome.org/action/show/Projects/Aravis?action=show&redirect=Aravis
*/

#include "CameraGigeSdkIc.h"

#ifdef WINDOWS

	boost::log::sources::severity_logger< LogSeverityLevel >  CameraGigeSdkIc::logger;

	CameraGigeSdkIc::Init CameraGigeSdkIc::initializer;

	CameraGigeSdkIc::CameraGigeSdkIc(){

		if(!DShowLib::InitLibrary())
			throw "Fail DShowLib::InitLibrary().";
		
		m_pGrabber = new DShowLib::Grabber();
		mFrameCounter = 0;
		mGain = 0;
		mExposure = 0;
		mFPS = 30;
		mImgDepth = MONO_8;
		mSaturateVal = 0;
	
	}

	void CameraGigeSdkIc::listGigeCameras(){

		// Retrieve a list with the video capture devices connected to the computer.
		pVidCapDevList = m_pGrabber->getAvailableVideoCaptureDevices();

		cout << "******** DETECTED CAMERAS WITH IMAGING SOURCE ********** " << endl;
		cout << "*" << endl;

		// Print available devices.
		for(int i = 0; i < pVidCapDevList->size(); i++)
			cout << "* -> [" << i << "] " << pVidCapDevList->at(0).c_str() << endl;
		
		cout << "*" << endl;
		cout << "******************************************************** " << endl;

	}

	// https://valelab.ucsf.edu/svn/micromanager2/branches/micromanager1.3/DeviceAdapters/TISCam/SimplePropertyAccess.cpp
	DShowLib::tIVCDRangePropertyPtr	CameraGigeSdkIc::getPropertyRangeInterface( _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr& pItems, const GUID& id ){

		GUID itemID = id;
		GUID elemID = DShowLib::VCDElement_Value;

		DShowLib::tIVCDPropertyElementPtr pFoundElement = pItems->findElement( itemID, elemID );
		if( pFoundElement != 0 )
		{
			DShowLib::tIVCDRangePropertyPtr pRange;
			if( pFoundElement->getInterfacePtr( pRange ) != 0 )
			{
				return pRange;
			}
		}
		return 0;
	}

	bool CameraGigeSdkIc::propertyIsAvailable( const GUID& id, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer ){

		return m_pItemContainer->findItem( id ) != 0;
		
	}

	long CameraGigeSdkIc::getPropertyValue( const GUID& id, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer ){
	
		long rval = 0;
		DShowLib::tIVCDRangePropertyPtr pRange = getPropertyRangeInterface( m_pItemContainer, id );
		if( pRange != 0 )
		{
			rval = pRange->getValue();
		}
		return rval;
	}

	void CameraGigeSdkIc::setPropertyValue( const GUID& id, long val, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer ){

		DShowLib::tIVCDRangePropertyPtr pRange = getPropertyRangeInterface( m_pItemContainer, id );
		if( pRange != 0 )
		{
			pRange->setValue( val );
		}
	}

	long CameraGigeSdkIc::getPropertyRangeMin( const GUID& id, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer ){

		long rval = 0;
		DShowLib::tIVCDRangePropertyPtr pRange = getPropertyRangeInterface( m_pItemContainer, id );
		if( pRange != 0 )
		{
			rval = pRange->getRangeMin();
		}
		return rval;
	}

	long CameraGigeSdkIc::getPropertyRangeMax(const GUID& id, _DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr m_pItemContainer){

		long rval = 0;
		DShowLib:: tIVCDRangePropertyPtr pRange = getPropertyRangeInterface( m_pItemContainer, id );
		if( pRange != 0 )
		{
			rval = pRange->getRangeMax();
		}
		return rval;
	}
	
	/*bool CameraGigeSdkIc::getAvailableFormat(){

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

	bool CameraGigeSdkIc::setFPS(int value) {

		return true;

	}

	bool CameraGigeSdkIc::createDevice(int id){
	
		// Open the selected video capture device.
		m_pGrabber->openDev(pVidCapDevList->at(id));

		if(pVidCapDevList == 0 || pVidCapDevList->empty()){

			cout << "No device available." << endl;
			return false;

		}

		return true;
	}

	bool CameraGigeSdkIc::setPixelFormat(CamBitDepth format) {

		mImgDepth = format;

		switch(format){
					
			case MONO_8 :

				cout << "Y8 (1280x960-1280x960)" << endl;

				m_pGrabber->setVideoFormat("Y8 (1280x960-1280x960)");

				// Set the image buffer format to eY800. eY800 means monochrome, 8 bits (1 byte) per pixel.
				// Let the sink create a matching MemBufferCollection with 1 buffer.
				pSink = DShowLib::FrameHandlerSink::create( DShowLib::eY800, NUMBER_OF_BUFFERS );

				break;

			case MONO_12 :
				
				cout << "Y16 (1280x960-1280x960)" << endl;
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

	void CameraGigeSdkIc::getExposureBounds(int &eMin, int &eMax) {

		// Get properties.
		_DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr pItems = m_pGrabber->getAvailableVCDProperties();
				
		eMin  = (int)getPropertyRangeMin(DShowLib::VCDID_Exposure, pItems);
		eMax  = (int)getPropertyRangeMax(DShowLib::VCDID_Exposure, pItems);

		cout << "eMin : " << eMin << " eMax : " << eMax << endl;

	}

	void CameraGigeSdkIc::getGainBounds(int &gMin, int &gMax) {

		// Get properties.
		_DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr pItems = m_pGrabber->getAvailableVCDProperties();
									
		gMin  = (int)getPropertyRangeMin(DShowLib::VCDID_Gain, pItems);
		gMax  = (int)getPropertyRangeMax(DShowLib::VCDID_Gain, pItems);

		cout << "gMin : " << gMin << " gMax : " << gMax << endl;

	
	}

	bool CameraGigeSdkIc::setExposureTime(int value) {

		_DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr pItems = m_pGrabber->getAvailableVCDProperties();

		setPropertyValue(DShowLib::VCDID_Exposure, (long)value, pItems);
		mExposure = value;

		cout << "setExposureTime : " << value <<  endl;

		return true;
	}

	bool CameraGigeSdkIc::setGain(int value) {

		_DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr pItems = m_pGrabber->getAvailableVCDProperties();

		setPropertyValue(DShowLib::VCDID_Gain, (long)value, pItems);
		mGain = value;

		cout << "setGain : " << mGain <<  endl;
		
		return true;
	}

	bool CameraGigeSdkIc::grabInitialization() {

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
		for (int ii = 0; ii < NUMBER_OF_BUFFERS; ++ii)
		{
			pBuf[ii] = new BYTE[info.buffersize];
			assert(pBuf[ii]);
		}

		// Create a new MemBuffer collection that uses our own image buffers.
		pCollection = DShowLib::MemBufferCollection::create(info, NUMBER_OF_BUFFERS, pBuf);
		if (pCollection == 0) return false;
		if (!pSink->setMemBufferCollection(pCollection)) return false;
 
		if (!m_pGrabber->startLive(false)) return false;


	}

	void CameraGigeSdkIc::acqStart() {

		if (!m_pGrabber->isLive()) {

			m_pGrabber->startLive(false);

		}

		pSink->snapImages(1,(DWORD)-1);

	}

	bool CameraGigeSdkIc::grabImage(Frame &newFrame) {

		Mat newImg;

		// Retrieve the output type and dimension of the handler sink.
		// The dimension of the sink could be different from the VideoFormat, when
		// you use filters.
		DShowLib::FrameTypeInfo info;
		pSink->getOutputFrameType(info);

		//Timestamping.
        string acquisitionDate = TimeDate::localDateTime(microsec_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");
        //BOOST_LOG_SEV(logger, normal) << "Date : " << acquisitionDate;
        boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
        string acqDateInMicrosec = to_iso_extended_string(time);

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

			//BOOST_LOG_SEV(logger, normal) << "Creating frame object ...";
			newFrame = Frame(newImg, mGain, mExposure, acquisitionDate);
			//BOOST_LOG_SEV(logger, normal) << "Setting date of frame ...";
			newFrame.setAcqDateMicro(acqDateInMicrosec);
			//BOOST_LOG_SEV(logger, normal) << "Setting fps of frame ...";
			newFrame.setFPS(mFPS);
			newFrame.setBitDepth(mImgDepth);
			//BOOST_LOG_SEV(logger, normal) << "Setting saturated value of frame ...";
			newFrame.setSaturatedValue(mSaturateVal);

			newFrame.setNumFrame(mFrameCounter);
			mFrameCounter++;
			string nn = "test-" + Conversion::intToString(mFrameCounter);
			//pCollection->save(nn);

			SaveImg::saveBMP(newImg,nn);

			if(mFrameCounter > 100) {

				m_pGrabber->stopLive();

				m_pGrabber->closeDev();

				
				return false;
			}

			return true;

		}

		return false;
		
	}

	bool CameraGigeSdkIc::grabSingleImage(Frame &frame, int camID) {

		// Retrieve a list with the video capture devices connected to the computer.
		DShowLib::Grabber::tVidCapDevListPtr pVidCapDevList = m_pGrabber->getAvailableVideoCaptureDevices();
		
		if(pVidCapDevList == 0 || pVidCapDevList->empty()){

			cout << "No device available." << endl;
			return false;

		}else{

			// Print available devices.
			int numCam = -1;
			for(int i = 0; i < pVidCapDevList->size(); i++){
				
				cout << "(" << i << ") " << pVidCapDevList->at(i).c_str() << endl;

				if(camID == i){
					numCam = i; 
					break;
				}

			}

			if(numCam == -1){

				return false;

			}else{

				// Open the selected video capture device.
				m_pGrabber->openDev(pVidCapDevList->at(numCam));

				/*cout << "Available video formats : " << endl;

				DShowLib::Grabber::tVidFmtDescListPtr DecriptionList;

				DecriptionList = m_pGrabber->getAvailableVideoFormatDescs();

				for( DShowLib::Grabber::tVidFmtDescList::iterator pDescription = DecriptionList->begin(); pDescription != DecriptionList->end(); pDescription++ )
				{
					printf("%s\n", (*pDescription)->toString().c_str());
				}*/
				
				

				cout << "Current bits per pixel : " << m_pGrabber->getVideoFormat().getBitsPerPixel() << endl;

				

				switch(frame.getBitDepth()){
					
					case MONO_8 :

						m_pGrabber->setVideoFormat("Y8 (1280x960-1280x960)");

						// Set the image buffer format to eY800. eY800 means monochrome, 8 bits (1 byte) per pixel.
						// Let the sink create a matching MemBufferCollection with 1 buffer.
						pSink = DShowLib::FrameHandlerSink::create( DShowLib::eY800, 1 );

						break;

					case MONO_12 :
						
						m_pGrabber->setVideoFormat("Y16 (1280x960-1280x960)");

						// Disable overlay.
						// http://www.theimagingsourceforums.com/archive/index.php/t-319880.html
						m_pGrabber->setOverlayBitmapPathPosition(DShowLib::ePP_NONE);

						// Set the image buffer format to eY16. eY16 means monochrome, 16 bits (2 byte) per pixel.
						// Let the sink create a matching MemBufferCollection with 1 buffer.
						pSink = DShowLib::FrameHandlerSink::create( DShowLib::eY16, 1 );

						break;
						
					default:
												
						return false;

						break;
				}

				cout << "New video format : " << m_pGrabber->getVideoFormat().getBitsPerPixel() << endl;
 
				// Get properties.
				_DSHOWLIB_NAMESPACE::tIVCDPropertyItemsPtr pItems = m_pGrabber->getAvailableVCDProperties();

				// Set Exposure time.
				int exposure = frame.getExposure();
								
				long eMin  = getPropertyRangeMin(DShowLib::VCDID_Exposure, pItems);
				long eMax  = getPropertyRangeMax(DShowLib::VCDID_Exposure, pItems);
				long e = getPropertyValue(DShowLib::VCDID_Exposure, pItems);

				cout << "Previous exposure time value : " << e << endl;

				if(exposure <= eMax && exposure >= eMin){

					setPropertyValue(DShowLib::VCDID_Exposure, (long)exposure, pItems);
					cout << "New exposure time value : " << getPropertyValue(DShowLib::VCDID_Exposure, pItems) << endl;

				}else{

					cout << "Fail to set exposure. Available range value is " << eMin << " to " << eMax << endl;
					return false;
				}

				// Set Gain (db)
				int gain = frame.getGain();
								
				long gMin  = getPropertyRangeMin(DShowLib::VCDID_Gain, pItems);
				long gMax  = getPropertyRangeMax(DShowLib::VCDID_Gain, pItems);
				long g = getPropertyValue(DShowLib::VCDID_Gain, pItems);

				cout << "Previous gain value : " << g << endl;

				if(gain <= gMax && gain >= gMin){

					setPropertyValue(DShowLib::VCDID_Gain, (long)gain, pItems);
					cout << "New gain value : " << getPropertyValue(DShowLib::VCDID_Gain, pItems) << endl;

				}else{

					cout << "Fail to set gain. Available range value is " << gMin << " to " << gMax << endl;
					return false;
				}

				
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
				cout << info.getBitsPerPixel() << endl;

				Mat newImg;
				//DShowLib::Grabber::tMemBufferCollectionPtr pCollection;

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
				string acquisitionDate = TimeDate::localDateTime(microsec_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");
				boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
				string acqDateInMicrosec = to_iso_extended_string(time);

                frame = Frame(newImg, 0, 0, acquisitionDate);
                frame.setAcqDateMicro(acqDateInMicrosec);
                frame.setFPS(0);

				//cout << "save " << endl;
				//pCollection->save( "yio*.bmp" );

				//double minVal, maxVal;
				//minMaxLoc(newImg, &minVal, &maxVal);

				//cout << "minVal :" << minVal << endl;
				//cout << "maxVal :" << maxVal << endl;
				
				//t = (((double)getTickCount() - t )/getTickFrequency())*1000;

				//cout << "time decalage : " << t << endl;
				
				//minMaxLoc(newImg, &minVal, &maxVal);

				//cout << "minVal :" << minVal << endl;
				//cout << "maxVal :" << maxVal << endl;

			}
		}

		return true;
	}

	CameraGigeSdkIc::~CameraGigeSdkIc(){

		 delete m_pGrabber;

	}

#endif