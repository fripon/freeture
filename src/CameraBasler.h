/*
								CameraBasler.h

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
 * @file    CameraBasler.h
 * @author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
 * @version 1.0
 * @date    03/06/2014
 */

#pragma once

#include "includes.h"
#include "Camera.h"
#include "CameraSDK.h"
#include "Fifo.h"
#include "Frame.h"
#include "TimeDate.h"
#include "Conversion.h"
#include "SaveImg.h"
#include "Fits2D.h"
#include "ManageFiles.h"
#include "Conversion.h"
#include "EnumLog.h"
//#include "serialize.h"
#include <boost/filesystem.hpp>
#include <iterator>
#include <algorithm>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/zlib.hpp>

using namespace boost::filesystem;

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

using boost::shared_ptr;

using namespace cv;
using namespace std;

using namespace logenum;

//! Thread class for acquisition with Basler cameras
class CameraBasler : public Camera{

	private:

        //! A logger
        /*!
          Logger used to manage messages added to the log file
        */
		src::severity_logger< severity_level > log;

        //! Pointer on the shared queue
        /*!
          The grabbed images by this acquisition thread will be push in this shared queue
        */
		Fifo<Frame> * framesQueue;

		//! Stop flag of the thread
		bool mustStop;

		//! Mutex on the stop flag
		boost::mutex mustStopMutex;

		//! Mutex on the shared queue
		boost::mutex				*mutexQueue;

		//! Condition if the shared queue is full
		boost::condition_variable	*condQueueFill;

		//! Condition if the shared queue has a new element
		boost::condition_variable	*condQueueNewElement;

        //! Pointer on the acquisition thread
		boost::thread *m_thread;

        //! Location on the buffer on the disk
		string bufferDiskPath;

		//! Size of the buffer on the disk
		int bufferDiskSize;

		CameraSDK *camera;

		bool threadStopped;

		unsigned int cpt;

		int initialExpValue;
        int initialGainValue;


        boost::mutex				*m_BufferDiskNames;
        vector<Frame>               *bufferDiskNames;

        bool saveFits;
        bool saveImg;
        int format;
        Mat mask;
        bool maskEnable;
        string configFile;
        string savePath;


	public:

        //! Constructor
        /*!
          \param queue pointer on the shared queue
          \param m_mutex_queue pointer on a mutex used for the shared queue
          \param m_cond_queue_fill pointer on a condition used to notify when the shared queue is full
          \param m_cond_queue_new_element pointer on a condition used to notify when the shared queue has received a new frame
        */
		CameraBasler										(   Fifo<Frame> *queueRam,
                                                                boost::mutex *m_queue,
                                                                boost::condition_variable *c_queueFull,
                                                                boost::condition_variable *c_queueNew,
                                                                int initialExposure,
                                                                int initialGain,
                                                                Mat frameMask,
                                                                bool enableMask);


        CameraBasler( int exposure,
                      int gain,
                      bool saveFits2D,
                      bool saveBmp,
                      int acqFormat,
                      string configPath,
                      string saveLocation);


		//! Constructor
		CameraBasler();

		//! Destructor
		~CameraBasler(void);

        //! Get width
		int		getCameraWidth();

		//! Get height
		int		getCameraHeight();

        //! List connected cameras
		void	getListCameras();

		//! Select a device by id or by name
		bool	setSelectedDevice(int id, string name);

		//! Wait the end of the acquisition thread
		void	join();

        //! Set the pixel format : 8 or 12
		void	setCameraPixelFormat(int depth);

        //! Acquisition thread operations
		void	operator()();

		//! Stop the thread
		void	stopThread();

		//! Start the acquisition thread
		void	startThread();

		//! Set the exposure time
		void	setCameraExposureTime(double value);

		//! Set the gain
		void	setCameraGain(int);

		void    startGrab();

		void    stopGrab();

		void    grabOne();
};





/*
// pylon api
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
	#include <pylon/PylonGUI.h>
#endif
#include <pylon/gige/BaslerGigEInstantCamera.h>
#include <pylon/gige/BaslerGigECamera.h>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

using boost::shared_ptr;

using namespace Pylon;
using namespace GenApi;
using namespace cv;
using namespace std;

using namespace Basler_GigECameraParams;

// buffer's number used for grabbing
static const uint32_t nbBuffers = 20;


//! Thread class for acquisition with Basler cameras
/*!
  This class uses Pylon SDK
*/
/*class CameraBasler : public Camera{

    public:

        /** Severity level enumeration for log file.
         */
     /*   enum severity_level{
                normal,         /**< enum normal */
        //        notification,   /**< enum notification */
        //        warning,        /**< enum warning */
        //        fail,           /**< enum fail */
        //        critical        /**< enum critical */
       // };
/*
	private:

        //! A logger
        /*!
          Logger used to manage messages added to the log file
        */
		/*src::severity_logger< severity_level > log;

		Pylon::PylonAutoInitTerm autoInitTerm;

        //! Pointer on the shared queue
        /*!
          The grabbed images by this acquisition thread will be push in this shared queue
        */
	/*	Fifo<Frame> * framesQueue;

		//! Stop flag of the thread
		bool mustStop;

		//! Mutex on the stop flag
		boost::mutex mustStopMutex;

		//! Mutex on the shared queue
		boost::mutex				*mutexQueue;

		//! Condition if the shared queue is full
		boost::condition_variable	*condQueueFill;

		//! Condition if the shared queue has a new element
		boost::condition_variable	*condQueueNewElement;

        //! Pointer on the acquisition thread
		boost::thread *m_thread;

        //! Buffer for the grabbed images in 8 bits format
		uint8_t* ppBuffersUC[nbBuffers];

		//! Buffer for the grabbed images in 8 bits format
		uint16_t* ppBuffersUS[nbBuffers];

		StreamBufferHandle handles[nbBuffers];

        //! Pointer on the transport layer
		CTlFactory			*pTlFactory;

		//! Pointer on basler camera
		CBaslerGigECamera	*pCamera;

		//! Pointer on device
		IPylonDevice		*pDevice;

		//! List of basler connected cameras
		DeviceInfoList_t	 devices;

		CBaslerGigECamera::EventGrabber_t	* pEventGrabber;

		IEventAdapter						* pEventAdapter;

		CBaslerGigECamera::StreamGrabber_t	* pStreamGrabber;

		int nbEventBuffers;

        //! Location on the buffer on the disk
		string bufferDiskPath;

		//! Size of the buffer on the disk
		int bufferDiskSize;

	public:

        //! Constructor
        /*!
          \param queue pointer on the shared queue
          \param m_mutex_queue pointer on a mutex used for the shared queue
          \param m_cond_queue_fill pointer on a condition used to notify when the shared queue is full
          \param m_cond_queue_new_element pointer on a condition used to notify when the shared queue has received a new frame
        */
	/*	CameraBasler										(   Fifo<Frame> *queue,
                                                                boost::mutex *m_mutex_queue,
                                                                boost::condition_variable *m_cond_queue_fill,
                                                                boost::condition_variable *m_cond_queue_new_element,
                                                                string frameBufferPath,
                                                                int frameBufferSize);

		//! Constructor
		CameraBasler										();

		//! Destructor
		~CameraBasler(void);

        //! Get width
        /*!
          \return width
        */
	/*	int			getWidth                                ();

		//! Get height
        /*!
          \return height
        */
		/*int			getHeight                               ();

        //! List connected cameras
		void		listCameras								();

		//! Choose a device
        /*!
          \param id id of the selected device
          \return success to choose the selected device
        */
	/*	bool		chooseDevice							(int id);

		//! Start grabbing
		int			grabStart								();

		//! Stop grabbing
		void		grabStop								();

		//! Wait the end of the acquisition thread
		void		join									();

        //! Set the pixel format
        /*!
          \param format 8 or 12 bits
        */
	/*	void		setPixelFormat	                        (int depth);

        //! Acquisition thread operations
		void		operator								()();

		//! Stop the thread
		void		stopThread();

		//! Set the exposure time
        /*!
          \param exp
        */
		/*void		setExposureTime							(double value);

		//! Set the gain
        /*!
          \param gain
        */
	/*	void		setGain					(int)			;
};*/

