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

DetThread::Init DetThread::initializer;

DetThread::DetThread(   string						   cfg_p,
						DetMeth						   m,
						boost::circular_buffer<Frame>  *fb,
						boost::mutex                   *fb_m,
						boost::condition_variable      *fb_c,
						bool                           *dSignal,
						boost::mutex                   *dSignal_m,
						boost::condition_variable      *dSignal_c){

	mCfgPath				= cfg_p;
	frameBuffer				= fb;
    frameBuffer_mutex		= fb_m;
    frameBuffer_condition	= fb_c;
    detSignal				= dSignal;
    detSignal_mutex			= dSignal_m;
    detSignal_condition		= dSignal_c;
	pThread				= NULL;
	mMustStop				= false;
	mEventPath				= "";
	mEventDate				= "";
	mDetMthd					= m;

	mIsRunning               = false;
	mNbDetection             = 0;

	mWaitFramesToCompleteEvent = false;
	mNbWaitFrames = 0;

	mInterruptionStatus = false;

}

DetThread::~DetThread(void){

	if(pDetMthd != NULL){

		BOOST_LOG_SEV(logger, notification) << "Remove pDetMthd instance.";
		delete pDetMthd;

	}

	if (pThread!=NULL){

		BOOST_LOG_SEV(logger, notification) << "Remove detThread instance.";
		delete pThread;

	}
}

bool DetThread::startThread(){

	BOOST_LOG_SEV(logger, notification) << "Starting detThread...";


	if(!loadDetThreadParameters()){
		BOOST_LOG_SEV(logger, fail) << "Fail to initialize detThread.";
		return false;
	}


	BOOST_LOG_SEV(logger, notification) << "Success to initialize detThreads.";
	BOOST_LOG_SEV(logger, notification) << "Create detThread.";
	pThread = new boost::thread(boost::ref(*this));
	return true;
}

void DetThread::stopThread(){

	BOOST_LOG_SEV(logger, notification) << "Stopping detThread...";

	// Signal the thread to stop (thread-safe)
	mMustStopMutex.lock();
	mMustStop=true;
	mMustStopMutex.unlock();

	// Wait for the thread to finish.

    while(pThread->timed_join(boost::posix_time::seconds(2)) == false){

		BOOST_LOG_SEV(logger, notification) << "DetThread interrupted.";
        pThread->interrupt();

    }
}

Detection* DetThread::getDetMethod(){

    return pDetMthd;

}

bool DetThread::loadDetThreadParameters(){

	try{

		Configuration cfg;
		cfg.Load(mCfgPath);

		// Get Acquisition frequency.
		int ACQ_FPS; cfg.Get("ACQ_FPS", ACQ_FPS);
		BOOST_LOG_SEV(logger, notification) << "ACQ_FPS : " << ACQ_FPS;

		// Get Acquisition format.
		string acq_bit_depth; cfg.Get("ACQ_BIT_DEPTH", acq_bit_depth);
		EParser<CamBitDepth> cam_bit_depth;
		mBitDepth = cam_bit_depth.parseEnum("ACQ_BIT_DEPTH", acq_bit_depth);
		BOOST_LOG_SEV(logger, notification) << "ACQ_BIT_DEPTH : " << acq_bit_depth;

		// Get the name of the station.
		cfg.Get("STATION_NAME", mStationName);
		BOOST_LOG_SEV(logger, notification) << "STATION_NAME : " << mStationName;

		// Get the location where to save data.
		cfg.Get("DATA_PATH", mDataPath);
		BOOST_LOG_SEV(logger, notification) << "DATA_PATH : " << mDataPath;

		// Get the option for saving .avi of detection.
		cfg.Get("DET_SAVE_AVI", mSaveAvi);
		BOOST_LOG_SEV(logger, notification) << "DET_SAVE_AVI : " << mSaveAvi;

		// Get the option for saving fits cube.
		cfg.Get("DET_SAVE_FITS3D", mSaveFits3D);
		BOOST_LOG_SEV(logger, notification) << "DET_SAVE_FITS3D : " << mSaveFits3D;

		// Get the option for saving each frame of the event in fits.
		cfg.Get("DET_SAVE_FITS2D", mSaveFits2D);
		BOOST_LOG_SEV(logger, notification) << "DET_SAVE_FITS2D : " << mSaveFits2D;

		// Get the option for stacking frame's event.
		cfg.Get("DET_SAVE_SUM", mSaveSum);
		BOOST_LOG_SEV(logger, notification) << "DET_SAVE_SUM : " << mSaveSum;

		// Get the time to keep before an event.
		cfg.Get("DET_TIME_BEFORE", mTimeBeforeEvent);
		mTimeBeforeEvent = mTimeBeforeEvent * ACQ_FPS;
		BOOST_LOG_SEV(logger, notification) << "DET_TIME_BEFORE (in frames): " << mTimeBeforeEvent;

		// Get the time to keep after an event.
		cfg.Get("DET_TIME_AFTER", mTimeAfterEvent);
		mTimeAfterEvent = mTimeAfterEvent * ACQ_FPS;
		BOOST_LOG_SEV(logger, notification) << "DET_TIME_AFTER (in frames): " << mTimeAfterEvent;

		// Get the option to send mail notifications.
		cfg.Get("MAIL_DETECTION_ENABLED", mMailAlertEnabled);
		BOOST_LOG_SEV(logger, notification) << "MAIL_DETECTION_ENABLED : " << mMailAlertEnabled;

		// Get the SMTP server adress.
		cfg.Get("MAIL_SMTP_SERVER", mMailSmtpServer);
		BOOST_LOG_SEV(logger, notification) << "MAIL_SMTP_SERVER : " << mMailSmtpServer;

        // Get the SMTP login.
		cfg.Get("MAIL_SMTP_LOGIN", mMailSmtpLogin);
		
        // Get the SMTP server adress.
		cfg.Get("MAIL_SMTP_PASSWORD", mMailSmtpPassword);

        // Get connection type to the SMTP server.
		string smtp_connection_type; cfg.Get("MAIL_CONNECTION_TYPE", smtp_connection_type);
		EParser<SmtpSecurity> smtp_security;
		mSmtpSecurity = smtp_security.parseEnum("MAIL_CONNECTION_TYPE", smtp_connection_type);

		// Get the option to reduce the stack of frame's event to 16 bits.
		cfg.Get("STACK_REDUCTION", mStackReduction);
		BOOST_LOG_SEV(logger, notification) << "STACK_REDUCTION : " << mStackReduction;

		// Get the method to stack frame's event.
		string stack_method;
		cfg.Get("STACK_MTHD", stack_method);
		BOOST_LOG_SEV(logger, notification) << "STACK_MTHD : " << stack_method;
		EParser<StackMeth> stack_mth;
		mStackMthd = stack_mth.parseEnum("STACK_MTHD", stack_method);

		// Load settable fits keywords from configuration file.
		mFitsHeader.loadKeywordsFromConfigFile(mCfgPath);

		// Get the list of mail notifications recipients.
		string mailRecipients;
        cfg.Get("MAIL_RECIPIENT", mailRecipients);
		BOOST_LOG_SEV(logger, notification) << "MAIL_RECIPIENT : " << mailRecipients;
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(",");
        tokenizer tokens(mailRecipients, sep);

        for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
            mMailRecipients.push_back(*tok_iter);

        }

		// Select the correct detection method according to DET_METHOD in configuration file.
		switch(mDetMthd){

            case TEMPORAL_MTHD :

				{

					pDetMthd = new DetectionTemporal();
					if(!pDetMthd->initMethod(mCfgPath)){
						BOOST_LOG_SEV(logger, fail) << "Fail to init temporal detection method.";
						throw "Fail to init temporal detection method.";
					}

				}

				break;

			case TEMPLATE_MTHD:

				{



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

    mInterruptionStatusMutex.lock();
    mInterruptionStatus = true;
    cout << "interruptionStatus in detection process : " << mInterruptionStatus << endl;
    mInterruptionStatusMutex.unlock();

}

void DetThread::operator ()(){

    bool stopThread = false;
    mIsRunning = true;

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
                mInterruptionStatusMutex.lock();
                if(mInterruptionStatus) forceToReset = true;
                mInterruptionStatusMutex.unlock();

                if(!forceToReset){

                    // Fetch the two last frames grabbed.
                    BOOST_LOG_SEV(logger, normal) << "Fetch the two last frames grabbed.";
                    Frame currentFrame, previousFrame;
                    boost::mutex::scoped_lock lock2(*frameBuffer_mutex);
                    if(frameBuffer->size() > 2){
                        currentFrame    = frameBuffer->back();
                    }
                    lock2.unlock();

                    double t = (double)getTickCount();

                    if(currentFrame.getImg().data){

                        BOOST_LOG_SEV(logger, normal) << "Start detection process on frames : " << currentFrame.getNumFrame();

                        // Run detection.
                        if(pDetMthd->run(currentFrame) && !mWaitFramesToCompleteEvent){

                            // Event detected.
                            BOOST_LOG_SEV(logger, notification) << "Event detected ! Waiting frames to complete the event..." << endl;
                            mWaitFramesToCompleteEvent = true;
                            mNbDetection++;

                        }

                        // Wait frames to complete the detection.
                        if(mWaitFramesToCompleteEvent){

                            if(mNbWaitFrames >= mTimeAfterEvent){

                                BOOST_LOG_SEV(logger, notification) << "Event completed." << endl;

                                // Build event directory.
                                mEventDate = pDetMthd->getDateEvent();
                                BOOST_LOG_SEV(logger, notification) << "Building event directory..." << endl;

                                if(buildEventDataDirectory())
                                    BOOST_LOG_SEV(logger, fail) << "Fail to build event directory !" << endl;
                                else
                                    BOOST_LOG_SEV(logger, notification) << "Success to build event directory !" << endl;

                                // Save event.
                                BOOST_LOG_SEV(logger, notification) << "Start saving event..." << endl;
                                pDetMthd->saveDetectionInfos(mEventPath);
                                boost::mutex::scoped_lock lock(*frameBuffer_mutex);
                                if(!saveEventData(pDetMthd->getNumFirstEventFrame(), pDetMthd->getNumLastEventFrame())){
                                    lock.unlock();
                                    BOOST_LOG_SEV(logger,critical) << "Error saving event data.";
                                    throw "Error saving event data.";
                                }else{
                                    BOOST_LOG_SEV(logger, notification) << "Success to save event !" << endl;
                                }
                                lock.unlock();

                                // Reset detection.
                                BOOST_LOG_SEV(logger, notification) << "Reset detection process." << endl;
                                pDetMthd->resetDetection();
                                mWaitFramesToCompleteEvent = false;
                                mNbWaitFrames = 0;

                            }else{

                                mNbWaitFrames++;

                            }
                        }
                    }

                    t = (((double)getTickCount() - t)/getTickFrequency())*1000;
                    cout << " [-DET TIME-] : " << std::setprecision(3) << std::fixed << t << " ms " << endl;
                    BOOST_LOG_SEV(logger,normal) << " [-DET TIME-] : " << std::setprecision(3) << std::fixed << t << " ms ";

                }else{

                    // reset method
                    if(pDetMthd != NULL)
                        pDetMthd->resetDetection();

                    mWaitFramesToCompleteEvent = false;
                    mNbWaitFrames = 0;

                    mInterruptionStatusMutex.lock();
                    mInterruptionStatus = false;
                    cout << "interruptionStatus in detection process : " << mInterruptionStatus << endl;
                    mInterruptionStatusMutex.unlock();

                }

            }catch(const boost::thread_interrupted&){

                BOOST_LOG_SEV(logger,notification) << "Detection Thread INTERRUPTED";
                cout << "Detection Thread INTERRUPTED" <<endl;

            }

            mMustStopMutex.lock();
            stopThread = mMustStop;
            mMustStopMutex.unlock();

        }while(!stopThread);

        cout << "--> NUMBER OF DETECTED EVENTS : " << mNbDetection << endl;

    }catch(const char * msg){

        cout << msg << endl;
        BOOST_LOG_SEV(logger,critical) << msg;

    }catch(exception& e){

        cout << "An exception occured : " << e.what() << endl;
        BOOST_LOG_SEV(logger, critical) << e.what();

    }

    mIsRunning = false;

	cout << "Detection Thread finished." << endl;
	BOOST_LOG_SEV(logger,notification) << "Detection Thread terminated.";

}

bool DetThread::getRunStatus(){

    return mIsRunning;

}

bool DetThread::buildEventDataDirectory(){

    namespace fs = boost::filesystem;

	// eventDate is the date of the first frame attached to the event.
	string YYYYMMDD = TimeDate::getYYYYMMDDfromDateString(mEventDate);

	// Data location.
	path p(mDataPath);

	// Create data directory for the current day.
	string fp = mDataPath + mStationName + "_" + YYYYMMDD +"/";
	path p0(fp);

    // Events directory.
    string fp1 = "events/";
	path p1(fp + fp1);

    // Current event directory with the format : STATION_AAAAMMDDThhmmss_UT
	string fp2 = mStationName + "_" + TimeDate::getYYYYMMDDThhmmss(mEventDate) + "_UT/";
	path p2(fp + fp1 + fp2);

	// Final path used by an other function to save event data.
    mEventPath = fp + fp1 + fp2;

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
    int numFirstFrameToSave = firstEvPosInFB - mTimeBeforeEvent;

    // Number of the last frame to save.
	// It depends of how many frames we want to keep after the event.
    int numLastFrameToSave = lastEvPosInFB + mTimeAfterEvent;

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
    cout << "> Time to keep before  : " << mTimeBeforeEvent		<< endl;
    cout << "> Time to keep after   : " << mTimeAfterEvent		<< endl;
    cout << "> Total frames to save : " << nbTotalFramesToSave << endl;
    cout << "> Total digit          : " << nbDigitOnNbTotalFramesToSave << endl;

	BOOST_LOG_SEV(logger,notification) << "> First frame to save  : " << numFirstFrameToSave;
	BOOST_LOG_SEV(logger,notification) << "> Lst frame to save    : " << numLastFrameToSave;
	BOOST_LOG_SEV(logger,notification) << "> First event frame    : " << firstEvPosInFB;
	BOOST_LOG_SEV(logger,notification) << "> Last event frame     : " << lastEvPosInFB;
	BOOST_LOG_SEV(logger,notification) << "> Time to keep before  : " << mTimeBeforeEvent;
	BOOST_LOG_SEV(logger,notification) << "> Time to keep after   : " << mTimeAfterEvent;
	BOOST_LOG_SEV(logger,notification) << "> Total frames to save : " << nbTotalFramesToSave;
	BOOST_LOG_SEV(logger,notification) << "> Total digit          : " << nbDigitOnNbTotalFramesToSave;

    vector<int> dateFirstFrame;
    float dateSecFirstFrame = 0.0;
	int c = 0;

	// Init fits 3D.
    Fits3D fits3d;

    if(mSaveFits3D){

        fits3d = Fits3D(mBitDepth, frameBuffer->front().getImg().rows, frameBuffer->front().getImg().cols, (numLastFrameToSave - numFirstFrameToSave +1), mFitsHeader);
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
        if((*it).getNumFrame() == numFirstFrameToSave && mSaveFits3D){

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
            double  julianDate      = TimeDate::gregorianToJulian((*it).getDate());
            double  julianCentury   = TimeDate::julianCentury(julianDate);
            double  sideralT        = TimeDate::localSideralTime_2(julianCentury, (*it).getDate().at(3), (*it).getDate().at(4), (*it).getDateSeconds(), mFitsHeader.getSitelong());
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
        if((*it).getNumFrame() == numLastFrameToSave && mSaveFits3D){
            cout << "DATE first : " << dateFirstFrame.at(3) << " H " << dateFirstFrame.at(4) << " M " << dateSecFirstFrame << " S" << endl;
            cout << "DATE last : " << (*it).getDate().at(3) << " H " << (*it).getDate().at(4) << " M " << (*it).getDateSeconds() << " S" << endl;
            fits3d.setElaptime(((*it).getDate().at(3)*3600 + (*it).getDate().at(4)*60 + (*it).getDateSeconds()) - (dateFirstFrame.at(3)*3600 + dateFirstFrame.at(4)*60 + dateSecFirstFrame));

        }

        // If the current frame read from the framebuffer has to be saved.
        if((*it).getNumFrame() >= numFirstFrameToSave && (*it).getNumFrame() <= numLastFrameToSave){

            // Save fits2D.
			if(mSaveFits2D){

                string fits2DPath = mEventPath + "fits2D/";
                string fits2DName = "frame_" + Conversion::numbering(nbDigitOnNbTotalFramesToSave, c) + Conversion::intToString(c);
                vector<string> DD;

                cout << "Save fits 2D  : " << fits2DName << endl;

                path p(fits2DPath);

                Fits2D newFits(fits2DPath, mFitsHeader);
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
                double  julianDate      = TimeDate::gregorianToJulian((*it).getDate());
                double  julianCentury   = TimeDate::julianCentury(julianDate);
                double  sideralT        = TimeDate::localSideralTime_2(julianCentury, (*it).getDate().at(3), (*it).getDate().at(4), (*it).getDateSeconds(), mFitsHeader.getSitelong());
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

				if(mBitDepth == MONO_8){

                    newFits.writeFits((*it).getImg(), UC8, fits2DName);

                }else{

                    newFits.writeFits((*it).getImg(), S16, fits2DName);
                }
            }

			// Add a frame to fits cube.
            if(mSaveFits3D) fits3d.addImageToFits3D((*it).getImg());

            // Add frame to the event's stack.
			if(mSaveSum && (*it).getNumFrame() >= firstEvPosInFB && (*it).getNumFrame() <= lastEvPosInFB){

				stack.addFrame((*it));

			}

			c++;

        }
    }

	// Write fits cube.
    if(mSaveFits3D) fits3d.writeFits3D(mEventPath + "fits3D");

	// Save stack of the event.
    if(mSaveSum) stack.saveStack(mFitsHeader, mEventPath, mStackMthd, mStationName, mStackReduction);

	// Send mail notification.
	if(mMailAlertEnabled){

		BOOST_LOG_SEV(logger,notification) << "Sending mail...";

		mailAttachments.push_back(mEventPath + "DirMap.bmp");

		mailAttachments.push_back(mEventPath + "GeMap.bmp");

        SMTPClient::sendMail(   mMailSmtpServer, 
                                mMailSmtpLogin,
                                mMailSmtpPassword, 
                                "freeture@" + mStationName +".fr", 
                                mMailRecipients, 
                                "Detection by " + mStationName  + "'s station - " + TimeDate::getYYYYMMDDThhmmss(mEventDate), 
                                mStationName + "\n" + mEventPath, 
                                mailAttachments,
                                mSmtpSecurity);


		BOOST_LOG_SEV(logger,notification) << "Mail sent.";

    }

    return true;

}
