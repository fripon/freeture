/*
								DetThread.cpp

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
*	Last modified:		20/10/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    DetThread.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Detection thread.
*/

#include "DetThread.h"

boost::log::sources::severity_logger< LogSeverityLevel >  DetThread::logger;
DetThread::_Init DetThread::_initializer;

DetThread::DetThread(	boost::mutex				   *cfg_m,
						string						   cfg_p,
						DetMeth						   m,
						boost::circular_buffer<Frame>  *fb,
						boost::mutex                   *fb_m,
						boost::condition_variable      *fb_c,
						bool                           *dSignal,
						boost::mutex                   *dSignal_m,
						boost::condition_variable      *dSignal_c){

	cfg_path				= cfg_p;
	cfg_mutex				= cfg_m;
	frameBuffer				= fb;
    frameBuffer_mutex		= fb_m;
    frameBuffer_condition	= fb_c;
    detSignal				= dSignal;
    detSignal_mutex			= dSignal_m;
    detSignal_condition		= dSignal_c;
	m_thread				= NULL;
	mustStop				= false;
	eventPath				= "";
	eventDate				= "";
	detmthd					= m;

	isRunning               = false;
	nbDetection             = 0;

	waitFramesToCompleteEvent = false;
	nbWaitFrames = 0;
	firstFrameGrabbed = false;
	interruptionStatus = false;

}

DetThread::~DetThread(void){

	if(detTech != NULL){

		BOOST_LOG_SEV(logger, notification) << "Remove detTech instance.";
		delete detTech;

	}

	if (m_thread!=NULL){

		BOOST_LOG_SEV(logger, notification) << "Remove detThread instance.";
		delete m_thread;

	}
}

void DetThread::join(){

	m_thread->join();

}

bool DetThread::startThread(){

	BOOST_LOG_SEV(logger, notification) << "Starting detThread...";

	boost::mutex::scoped_lock lock(*cfg_mutex);
	if(!loadDetThreadParameters()){
		lock.unlock();
		BOOST_LOG_SEV(logger, fail) << "Fail to initialize detThread.";
		return false;
	}
	lock.unlock();

	BOOST_LOG_SEV(logger, notification) << "Success to initialize detThreads.";
	BOOST_LOG_SEV(logger, notification) << "Create detThread.";
	m_thread = new boost::thread(boost::ref(*this));
	return true;
}

void DetThread::stopThread(){

	BOOST_LOG_SEV(logger, notification) << "Stopping detThread...";

	// Signal the thread to stop (thread-safe)
	mustStopMutex.lock();
	mustStop=true;
	mustStopMutex.unlock();

	// Wait for the thread to finish.

    while(m_thread->timed_join(boost::posix_time::seconds(2)) == false){

		BOOST_LOG_SEV(logger, notification) << "DetThread interrupted.";
        m_thread->interrupt();

    }
}

Detection* DetThread::getDetMethod(){

    return detTech;

}

bool DetThread::loadDetThreadParameters(){

	try{

		Configuration cfg;
		cfg.Load(cfg_path);

		// Get Acquisition frequency.
		int ACQ_FPS; cfg.Get("ACQ_FPS", ACQ_FPS);
		BOOST_LOG_SEV(logger, notification) << "ACQ_FPS : " << ACQ_FPS;

		// Get Acquisition format.
		string acq_bit_depth; cfg.Get("ACQ_BIT_DEPTH", acq_bit_depth);
		EParser<CamBitDepth> cam_bit_depth;
		ACQ_BIT_DEPTH = cam_bit_depth.parseEnum("ACQ_BIT_DEPTH", acq_bit_depth);
		BOOST_LOG_SEV(logger, notification) << "ACQ_BIT_DEPTH : " << acq_bit_depth;

		// Get the name of the station.
		cfg.Get("STATION_NAME", STATION_NAME);
		BOOST_LOG_SEV(logger, notification) << "STATION_NAME : " << STATION_NAME;

		// Get the option to copy configuration.cfg in each day directory.
		cfg.Get("CFG_FILECOPY_ENABLED", CFG_FILECOPY_ENABLED);
		BOOST_LOG_SEV(logger, notification) << "CFG_FILECOPY_ENABLED : " << CFG_FILECOPY_ENABLED;

		// Get the location where to save data.
		cfg.Get("DATA_PATH", DATA_PATH);
		BOOST_LOG_SEV(logger, notification) << "DATA_PATH : " << DATA_PATH;

		// Get the option for saving .avi of detection.
		cfg.Get("DET_SAVE_AVI", DET_SAVE_AVI);
		BOOST_LOG_SEV(logger, notification) << "DET_SAVE_AVI : " << DET_SAVE_AVI;

		// Get the option for saving fits cube.
		cfg.Get("DET_SAVE_FITS3D", DET_SAVE_FITS3D);
		BOOST_LOG_SEV(logger, notification) << "DET_SAVE_FITS3D : " << DET_SAVE_FITS3D;

		// Get the option for saving each frame of the event in fits.
		cfg.Get("DET_SAVE_FITS2D", DET_SAVE_FITS2D);
		BOOST_LOG_SEV(logger, notification) << "DET_SAVE_FITS2D : " << DET_SAVE_FITS2D;

		// Get the option for stacking frame's event.
		cfg.Get("DET_SAVE_SUM", DET_SAVE_SUM);
		BOOST_LOG_SEV(logger, notification) << "DET_SAVE_SUM : " << DET_SAVE_SUM;

		// Get the time to keep before an event.
		cfg.Get("DET_TIME_BEFORE", DET_TIME_BEFORE);
		DET_TIME_BEFORE = DET_TIME_BEFORE * ACQ_FPS;
		BOOST_LOG_SEV(logger, notification) << "DET_TIME_BEFORE (in frames): " << DET_TIME_BEFORE;

		// Get the time to keep after an event.
		cfg.Get("DET_TIME_AFTER", DET_TIME_AFTER);
		DET_TIME_AFTER = DET_TIME_AFTER * ACQ_FPS;
		BOOST_LOG_SEV(logger, notification) << "DET_TIME_AFTER (in frames): " << DET_TIME_AFTER;

		// Get the option to send mail notifications.
		cfg.Get("MAIL_DETECTION_ENABLED", MAIL_DETECTION_ENABLED);
		BOOST_LOG_SEV(logger, notification) << "MAIL_DETECTION_ENABLED : " << MAIL_DETECTION_ENABLED;

		// Get the SMTP server adress.
		cfg.Get("MAIL_SMTP_SERVER", MAIL_SMTP_SERVER);
		BOOST_LOG_SEV(logger, notification) << "MAIL_SMTP_SERVER : " << MAIL_SMTP_SERVER;

		// Get the SMTP server hostname.
		cfg.Get("MAIL_SMTP_HOSTNAME", MAIL_SMTP_HOSTNAME);
		BOOST_LOG_SEV(logger, notification) << "MAIL_SMTP_HOSTNAME : " << MAIL_SMTP_HOSTNAME;

		// Get the option to reduce the stack of frame's event to 16 bits.
		cfg.Get("STACK_REDUCTION", STACK_REDUCTION);
		BOOST_LOG_SEV(logger, notification) << "STACK_REDUCTION : " << STACK_REDUCTION;

		// Get the method to stack frame's event.
		string stack_method;
		cfg.Get("STACK_MTHD", stack_method);
		BOOST_LOG_SEV(logger, notification) << "STACK_MTHD : " << stack_method;
		EParser<StackMeth> stack_mth;
		STACK_MTHD = stack_mth.parseEnum("STACK_MTHD", stack_method);

		// Load settable fits keywords from configuration file.
		fitsHeader.loadKeywordsFromConfigFile(cfg_path);

		// Get the list of mail notifications recipients.
		string mailRecipients;
        cfg.Get("MAIL_RECIPIENT", mailRecipients);
		BOOST_LOG_SEV(logger, notification) << "MAIL_RECIPIENT : " << mailRecipients;
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(",");
        tokenizer tokens(mailRecipients, sep);

        for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
            MAIL_RECIPIENT.push_back(*tok_iter);

        }

		// Select the correct detection method according to DET_METHOD in configuration file.
		switch(detmthd){

			case TEMPORAL_MTHD :

				{

					detTech = new DetectionTemporal();
					if(!detTech->initMethod(cfg_path)){
						BOOST_LOG_SEV(logger, fail) << "Fail to init temporal detection method.";
						throw "Fail to init temporal detection method.";
					}

				}

				break;

            case TEMPORAL_MTHD_ :

				{

					detTech = new DetectionTemporal_();
					if(!detTech->initMethod(cfg_path)){
						BOOST_LOG_SEV(logger, fail) << "Fail to init temporal detection method.";
						throw "Fail to init temporal detection method.";
					}

				}

				break;

			case HOUGH_MTHD:

				{



				}

				break;

            case DAYTIME_MTHD:

				{

				    detTech = new DetectionDayTime();
					if(!detTech->initMethod(cfg_path)){
						BOOST_LOG_SEV(logger, fail) << "Fail to init daytime detection method.";
						throw "Fail to init daytime detection method.";
					}



				}

				break;

		}

	}catch(exception& e){

		cout << e.what() << endl;
		return false;

	}catch(const char * msg){

		cout << msg << endl;
		return false;

	}

	return true;

}

void DetThread::interruptThread(){

    interruptionStatusMutex.lock();
    interruptionStatus = true;
    cout << "interruptionStatus in detection process : " << interruptionStatus << endl;
    interruptionStatusMutex.unlock();

}

void DetThread::operator ()(){

    bool stopThread = false;
    isRunning = true;

	BOOST_LOG_SCOPED_THREAD_TAG("LogName", "DET_THREAD");
	BOOST_LOG_SEV(logger,notification) << "\n";
    BOOST_LOG_SEV(logger,notification) << "==============================================";
	BOOST_LOG_SEV(logger,notification) << "=========== Start detection thread ===========";
	BOOST_LOG_SEV(logger,notification) << "==============================================";

    /// Thread loop.
    try{
        do{

            try{

                /// Wait new frame from AcqThread.
                BOOST_LOG_SEV(logger, normal) << "Waiting new frame from AcqThread.";
                boost::mutex::scoped_lock lock(*detSignal_mutex);
                while (!(*detSignal)) detSignal_condition->wait(lock);
                *detSignal = false;
                lock.unlock();
                BOOST_LOG_SEV(logger, normal) << "End to wait new frame from AcqThread.";

                // Check interruption signal from AcqThread.
                bool forceToReset = false;
                interruptionStatusMutex.lock();
                if(interruptionStatus) forceToReset = true;
                interruptionStatusMutex.unlock();

                if(!forceToReset){

                    // Fetch the two last frames grabbed.
                    BOOST_LOG_SEV(logger, normal) << "Fetch the two last frames grabbed.";
                    Frame currentFrame, previousFrame;
                    boost::mutex::scoped_lock lock2(*frameBuffer_mutex);
                    if(frameBuffer->size() > 2){
                        currentFrame    = frameBuffer->back();
                        previousFrame   = frameBuffer->at(frameBuffer->size()-2);
                    }
                    lock2.unlock();

                    double t = (double)getTickCount();

                    if(currentFrame.getImg().data && previousFrame.getImg().data){

                        BOOST_LOG_SEV(logger, normal) << "Start detection process on frames : " << currentFrame.getNumFrame() << " and " << previousFrame.getNumFrame();

                        // Run detection.
                        if(detTech->run(currentFrame, previousFrame) && !waitFramesToCompleteEvent){

                            // Event detected.
                            BOOST_LOG_SEV(logger, notification) << "Event detected ! Waiting frames to complete the event..." << endl;
                            waitFramesToCompleteEvent = true;
                            nbDetection++;

                        }

                        // Wait frames to complete the detection.
                        if(waitFramesToCompleteEvent){

                            if(nbWaitFrames >= DET_TIME_AFTER){

                                BOOST_LOG_SEV(logger, notification) << "Event completed." << endl;

                                // Build event directory.
                                eventDate = detTech->getDateEvent();
                                BOOST_LOG_SEV(logger, notification) << "Building event directory..." << endl;

                                if(buildEventDataDirectory(eventDate))
                                    BOOST_LOG_SEV(logger, fail) << "Fail to build event directory !" << endl;
                                else
                                    BOOST_LOG_SEV(logger, notification) << "Success to build event directory !" << endl;

                                // Save event.
                                BOOST_LOG_SEV(logger, notification) << "Start saving event..." << endl;
                                detTech->saveDetectionInfos(eventPath);
                                boost::mutex::scoped_lock lock(*frameBuffer_mutex);
                                if(!saveEventData(detTech->getNumFirstEventFrame(), detTech->getNumLastEventFrame())){
                                    lock.unlock();
                                    BOOST_LOG_SEV(logger,critical) << "Error saving event data.";
                                    throw "Error saving event data.";
                                }else{
                                    BOOST_LOG_SEV(logger, notification) << "Success to save event !" << endl;
                                }
                                lock.unlock();

                                // Reset detection.
                                BOOST_LOG_SEV(logger, notification) << "Reset detection process." << endl;
                                detTech->resetDetection();
                                waitFramesToCompleteEvent = false;
                                nbWaitFrames = 0;

                            }else{

                                nbWaitFrames++;

                            }
                        }
                    }

                    t = (((double)getTickCount() - t)/getTickFrequency())*1000;
                    cout << " [-DET TIME-] : " << std::setprecision(3) << std::fixed << t << " ms " << endl;
                    BOOST_LOG_SEV(logger,normal) << " [-DET TIME-] : " << std::setprecision(3) << std::fixed << t << " ms ";

                }else{

                    // reset method
                    if(detTech != NULL)
                        detTech->resetDetection();

                    waitFramesToCompleteEvent = false;
                    nbWaitFrames = 0;

                    interruptionStatusMutex.lock();
                    interruptionStatus = false;
                    cout << "interruptionStatus in detection process : " << interruptionStatus << endl;
                    interruptionStatusMutex.unlock();

                }

            }catch(const boost::thread_interrupted&){

                BOOST_LOG_SEV(logger,notification) << "Detection Thread INTERRUPTED";
                cout << "Detection Thread INTERRUPTED" <<endl;

            }

            mustStopMutex.lock();
            stopThread = mustStop;
            mustStopMutex.unlock();

        }while(!stopThread);

        cout << "--> NUMBER OF DETECTED EVENTS : " << nbDetection << endl;

    }catch(const char * msg){

        cout << msg << endl;
        BOOST_LOG_SEV(logger,critical) << msg;

    }catch(exception& e){

        cout << "An exception occured : " << e.what() << endl;
        BOOST_LOG_SEV(logger, critical) << e.what();

    }

    isRunning = false;

	cout << "Detection Thread terminated." << endl;
	BOOST_LOG_SEV(logger,notification) << "Detection Thread terminated.";

}

bool DetThread::getRunStatus(){

    return isRunning;

}

bool DetThread::buildEventDataDirectory(string eventDate){

    namespace fs = boost::filesystem;

	// eventDate is the date of the first frame attached to the event.
	string YYYYMMDD = TimeDate::get_YYYYMMDD_fromDateString(eventDate);

	// Data location.
	path p(DATA_PATH);

	// Create data directory for the current day.
	string fp = DATA_PATH + STATION_NAME + "_" + YYYYMMDD +"/";
	path p0(fp);

    // Events directory.
    string fp1 = "events/";
	path p1(fp + fp1);

    // Current event directory with the format : STATION_AAAAMMDDThhmmss_UT
	string fp2 = STATION_NAME + "_" + TimeDate::get_YYYYMMDDThhmmss(eventDate) + "_UT/";
	path p2(fp + fp1 + fp2);

	// Final path used by an other function to save event data.
    eventPath = fp + fp1 + fp2;

	// Check if data path specified in the configuration file exists.
    if(fs::exists(p)){

        // Check DataLocation/STATION_AAMMDD/
        if(fs::exists(p0)){

            // Check DataLocation/STATION_AAMMDD/events/
            if(fs::exists(p1)){

                // Check DataLocation/STATION_AAMMDD/events/STATION_AAAAMMDDThhmmss_UT/
                if(!fs::exists(p2)){

					// Create DataLocation/STATION_AAMMDD/events/STATION_AAAAMMDDThhmmss_UT/
                    if(!fs::create_directory(p2)){

						BOOST_LOG_SEV(logger,fail) << "Fail to create : " << p2;
                        return false;

                    }else{

						BOOST_LOG_SEV(logger,notification) << "Success to create : " << p2;
                        return true;
                    }

                }

            }else{

                // Create DataLocation/STATION_AAMMDD/events/
                if(!fs::create_directory(p1)){

					BOOST_LOG_SEV(logger,fail) << "Fail to create : " << p1;
                    return false;

                }else{

                    // Create DataLocation/STATION_AAMMDD/events/STATION_AAAAMMDDThhmmss_UT/
                    if(!fs::create_directory(p2)){

						BOOST_LOG_SEV(logger,fail) << "Fail to create : " << p2;
                        return false;

                    }else{

						BOOST_LOG_SEV(logger,notification) << "Success to create : " << p2;
                        return true;

                    }
                }
            }

        }else{

            // Create DataLocation/STATION_AAMMDD/
            if(!fs::create_directory(p0)){

				BOOST_LOG_SEV(logger,fail) << "Fail to create : " << p0;
                return false;

            }else{

				// Create DataLocation/STATION_AAMMDD/events/
                if(!fs::create_directory(p1)){

					BOOST_LOG_SEV(logger,fail) << "Fail to create : " << p1;
                    return false;

                }else{

					// Create DataLocation/STATION_AAMMDD/events/STATION_AAAAMMDDThhmmss_UT/
                    if(!fs::create_directory(p2)){

						BOOST_LOG_SEV(logger,fail) << "Fail to create : " << p2;
                        return false;

                    }else{

						BOOST_LOG_SEV(logger,notification) << "Success to create : " << p2;
                        return true;

                    }
                }
            }
        }

    }else{

		// Create DataLocation/
        if(!fs::create_directory(p)){

			BOOST_LOG_SEV(logger,fail) << "Fail to create : " << p;
            return false;

        }else{

            // Create DataLocation/STATION_AAMMDD/
            if(!fs::create_directory(p0)){

				BOOST_LOG_SEV(logger,fail) << "Fail to create : " << p0;
                return false;

            }else{

				//Create DataLocation/STATION_AAMMDD/events/
                if(!fs::create_directory(p1)){

					BOOST_LOG_SEV(logger,fail) << "Fail to create : " << p1;
                    return false;

                }else{

					// Create DataLocation/STATION_AAMMDD/events/STATION_AAAAMMDDThhmmss_UT/
                    if(!fs::create_directory(p2)){

						BOOST_LOG_SEV(logger,fail) << "Fail to create : " << p2;
                        return false;

                    }else{

						BOOST_LOG_SEV(logger,notification) << "Success to create : " << p1;
                        return true;

                    }
                }
            }
        }
    }

	return false;
}

bool DetThread::saveEventData(int firstEvPosInFB, int lastEvPosInFB){

    namespace fs = boost::filesystem;

	// List of data path to attach to the mail notification.
    vector<string> mailAttachments;

    // Number of the first frame to save.
	// It depends of how many frames we want to keep before the event.
    int numFirstFrameToSave = firstEvPosInFB - DET_TIME_BEFORE;

    // Number of the last frame to save.
	// It depends of how many frames we want to keep after the event.
    int numLastFrameToSave = lastEvPosInFB + DET_TIME_AFTER;

	// If the number of the first frame to save for the event is not in the framebuffer.
	// The first frame to save become the first frame available in the framebuffer.
    if(frameBuffer->front().getNumFrame() > numFirstFrameToSave)
        numFirstFrameToSave = frameBuffer->front().getNumFrame();


	// Check the number of the last frame to save.
    if(frameBuffer->back().getNumFrame() < numLastFrameToSave)
           numLastFrameToSave = frameBuffer->back().getNumFrame();

	// Total frames to save.
	int nbTotalFramesToSave = numLastFrameToSave - numFirstFrameToSave;

	// Count number of digit on nbTotalFramesToSave.
    int n = nbTotalFramesToSave;
    int nbDigitOnNbTotalFramesToSave = 0;
    while(n!=0){
      n/=10;
      ++nbDigitOnNbTotalFramesToSave;
    }

    cout << "> First frame to save  : " << numFirstFrameToSave	<< endl;
    cout << "> Last frame to save    : " << numLastFrameToSave	<< endl;
    cout << "> First event frame    : " << firstEvPosInFB		<< endl;
    cout << "> Last event frame     : " << lastEvPosInFB		<< endl;
    cout << "> Time to keep before  : " << DET_TIME_BEFORE		<< endl;
    cout << "> Time to keep after   : " << DET_TIME_AFTER		<< endl;
    cout << "> Total frames to save : " << nbTotalFramesToSave << endl;
    cout << "> Total digit          : " << nbDigitOnNbTotalFramesToSave << endl;

	BOOST_LOG_SEV(logger,notification) << "> First frame to save  : " << numFirstFrameToSave;
	BOOST_LOG_SEV(logger,notification) << "> Lst frame to save    : " << numLastFrameToSave;
	BOOST_LOG_SEV(logger,notification) << "> First event frame    : " << firstEvPosInFB;
	BOOST_LOG_SEV(logger,notification) << "> Last event frame     : " << lastEvPosInFB;
	BOOST_LOG_SEV(logger,notification) << "> Time to keep before  : " << DET_TIME_BEFORE;
	BOOST_LOG_SEV(logger,notification) << "> Time to keep after   : " << DET_TIME_AFTER;
	BOOST_LOG_SEV(logger,notification) << "> Total frames to save : " << nbTotalFramesToSave;
	BOOST_LOG_SEV(logger,notification) << "> Total digit          : " << nbDigitOnNbTotalFramesToSave;

    vector<int> dateFirstFrame;
    float dateSecFirstFrame = 0.0;
	int c = 0;

	// Init fits 3D.
    Fits3D fits3d;

    if(DET_SAVE_FITS3D){

        fits3d = Fits3D(ACQ_BIT_DEPTH, frameBuffer->front().getImg().rows, frameBuffer->front().getImg().cols, (numLastFrameToSave - numFirstFrameToSave +1), fitsHeader);
        boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
        fits3d.setDate(to_iso_extended_string(time));

        // Name of the fits file.
        fits3d.setFilename("fits3d.fit");

    }

	// Init sum.
	Stack stack = Stack(lastEvPosInFB - firstEvPosInFB);

	// Loop framebuffer.
    boost::circular_buffer<Frame>::iterator it;
    for(it = frameBuffer->begin(); it != frameBuffer->end(); ++it){

        // Get infos about the first frame of the event for fits 3D.
        if((*it).getNumFrame() == numFirstFrameToSave && DET_SAVE_FITS3D){

            fits3d.setDateobs((*it).getAcqDateMicro());
            // Exposure time.
            fits3d.setOntime((*it).getExposure()/1000000.0);
            // Gain.
            fits3d.setGaindb((*it).getGain());
            // Saturation.
            fits3d.setSaturate((*it).getSaturatedValue());
            // FPS.
            fits3d.setCd3_3((*it).getFPS());
            // CRVAL1 : sideral time.
            double  julianDate      = TimeDate::gregorianToJulian_2((*it).getDate());
            double  julianCentury   = TimeDate::julianCentury(julianDate);
            double  sideralT        = TimeDate::localSideralTime_2(julianCentury, (*it).getDate().at(3), (*it).getDate().at(4), (*it).getDateSeconds(), fitsHeader.getSitelong());
            fits3d.setCrval1(sideralT);
            // Projection and reference system
            fits3d.setCtype1("RA---ARC");
            fits3d.setCtype2("DEC--ARC");
            // Equinox
            fits3d.setEquinox(2000.0);
            // Integration time : 1/fps * nb_frames (sec.)
			if((*it).getFPS()!=0)
            fits3d.setExposure((1.0/(*it).getFPS()));

            dateFirstFrame = (*it).getDate();
            dateSecFirstFrame = (*it).getDateSeconds();

        }

        // Get infos about the last frame of the event record for fits 3D.
        if((*it).getNumFrame() == numLastFrameToSave && DET_SAVE_FITS3D){
            cout << "DATE first : " << dateFirstFrame.at(3) << " H " << dateFirstFrame.at(4) << " M " << dateSecFirstFrame << " S" << endl;
            cout << "DATE last : " << (*it).getDate().at(3) << " H " << (*it).getDate().at(4) << " M " << (*it).getDateSeconds() << " S" << endl;
            fits3d.setElaptime(((*it).getDate().at(3)*3600 + (*it).getDate().at(4)*60 + (*it).getDateSeconds()) - (dateFirstFrame.at(3)*3600 + dateFirstFrame.at(4)*60 + dateSecFirstFrame));

        }

        // If the current frame read from the framebuffer has to be saved.
        if((*it).getNumFrame() >= numFirstFrameToSave && (*it).getNumFrame() <= numLastFrameToSave){

            // Save fits2D.
			if(DET_SAVE_FITS2D){

                string fits2DPath = eventPath + "fits2D/";
                string fits2DName = "frame_" + Conversion::numbering(nbDigitOnNbTotalFramesToSave, c) + Conversion::intToString(c);
                vector<string> DD;

                cout << "Save fits 2D  : " << fits2DName << endl;

                path p(fits2DPath);

                Fits2D newFits(fits2DPath, fitsHeader);
				cout << (*it).getAcqDateMicro() << endl;
                // Frame's acquisition date.
                newFits.setDateobs((*it).getAcqDateMicro());
                // Fits file creation date.
                boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
                // YYYYMMDDTHHMMSS,fffffffff where T is the date-time separator
                newFits.setDate(to_iso_string(time));
				cout << to_iso_string(time) << endl;

                // Name of the fits file.

                newFits.setFilename(fits2DName);
                // Exposure time.

                newFits.setOntime((*it).getExposure()/1000000.0);
                // Gain.

                newFits.setGaindb((*it).getGain());
                // Saturation.

                newFits.setSaturate((*it).getSaturatedValue());
                // FPS.

                newFits.setCd3_3((*it).getFPS());
                // CRVAL1 : sideral time.

                double  julianDate      = TimeDate::gregorianToJulian_2((*it).getDate());

                double  julianCentury   = TimeDate::julianCentury(julianDate);

                double  sideralT        = TimeDate::localSideralTime_2(julianCentury, (*it).getDate().at(3), (*it).getDate().at(4), (*it).getDateSeconds(), fitsHeader.getSitelong());
                newFits.setCrval1(sideralT);
                // Integration time : 1/fps * nb_frames (sec.)
				if((*it).getFPS()!=0)
                newFits.setExposure((1.0f/(*it).getFPS()));
                //cout << "EXPOSURE : " << 1.0/(*it).getFPS() << endl;
                // Projection and reference system
                newFits.setCtype1("RA---ARC");
                newFits.setCtype2("DEC--ARC");
                // Equinox
                newFits.setEquinox(2000.0);

                if(!fs::exists(p)) {

					fs::create_directory(p);

				}

				if(ACQ_BIT_DEPTH == MONO_8){

                    newFits.writeFits((*it).getImg(), UC8, fits2DName);

                }else{

                    newFits.writeFits((*it).getImg(), S16, fits2DName);
                }
            }

			// Add a frame to fits cube.
            if(DET_SAVE_FITS3D) fits3d.addImageToFits3D((*it).getImg());

            // Add frame to the event's stack.
			if(DET_SAVE_SUM && (*it).getNumFrame() >= firstEvPosInFB && (*it).getNumFrame() <= lastEvPosInFB){

				stack.addFrame((*it));

			}

			c++;

        }
    }

	// Write fits cube.
    if(DET_SAVE_FITS3D) fits3d.writeFits3D(eventPath + "fits3D");

	// Save stack of the event.
    if(DET_SAVE_SUM) stack.saveStack(fitsHeader, eventPath, STACK_MTHD, STATION_NAME, STACK_REDUCTION);

	// Send mail notification.
	if(MAIL_DETECTION_ENABLED){

		BOOST_LOG_SEV(logger,notification) << "Sending mail...";

		mailAttachments.push_back(eventPath + "DirMap.bmp");

		mailAttachments.push_back(eventPath + "GeMap.bmp");

        SMTPClient mailc(MAIL_SMTP_SERVER, 25, MAIL_SMTP_HOSTNAME);

        mailc.send("yoan.audureau@u-psud.fr",
					MAIL_RECIPIENT,
					"Detection by " + STATION_NAME  + "'s station - " + TimeDate::get_YYYYMMDDThhmmss(eventDate),
					STATION_NAME + "\n" + eventPath,
					mailAttachments,
					false);

		BOOST_LOG_SEV(logger,notification) << "Mail sent.";

    }

    return true;

}
