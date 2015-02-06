/*
								FreeTure.cpp

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
*	Last modified:		04/12/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    FreeTure.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    19/06/2014
* \brief   FreeTure'parameters.
*/

#include "FreeTure.h"

bool FreeTure::loadParameters(){

    boost::filesystem::path p(cfgFile);

    if(boost::filesystem::exists(p)){

        Configuration cfg;
        cfg.Load(cfgFile);

		/// Input parameters

        string camera_type;
		cfg.Get("CAMERA_TYPE", camera_type);

		CamType id_cam_type = cam_type.parseEnum("CAMERA_TYPE", camera_type);
        CAMERA_TYPE = id_cam_type;

		cfg.Get("CAMERA_NAME", 					CAMERA_NAME);
		cfg.Get("CAMERA_ID", 					CAMERA_ID);
		cfg.Get("VIDEO_PATH", 					VIDEO_PATH);
		cfg.Get("FRAMES_PATH", 					FRAMES_PATH);
		cfg.Get("FRAMES_SEPARATOR", 			FRAMES_SEPARATOR);
		cfg.Get("FRAMES_SEPARATOR_POSITION", 	FRAMES_SEPARATOR_POSITION);

		/// Acquisition parameters

		cfg.Get("ACQ_FPS",						ACQ_FPS);

		string acq_bit_depth;
		cfg.Get("ACQ_BIT_DEPTH", acq_bit_depth);
		EParser<CamBitDepth> cam_bit_depth;
		CamBitDepth id_cam_bit_depth = cam_bit_depth.parseEnum("ACQ_BIT_DEPTH", acq_bit_depth);
        ACQ_BIT_DEPTH = id_cam_bit_depth;

		cfg.Get("ACQ_EXPOSURE", ACQ_EXPOSURE);
		cfg.Get("ACQ_GAIN",						ACQ_GAIN);
		cfg.Get("ACQ_AUTO_EXPOSURE_ENABLED",	ACQ_AUTO_EXPOSURE_ENABLED);
		cfg.Get("ACQ_MASK_ENABLED",				ACQ_MASK_ENABLED);
		cfg.Get("ACQ_MASK_PATH",				ACQ_MASK_PATH);

		cfg.Get("ACQ_BUFFER_SIZE",              ACQ_BUFFER_SIZE);

		/// Detection parameters

		cfg.Get("DET_ENABLED", 					DET_ENABLED);

		string det_method;
		cfg.Get("DET_METHOD", det_method);

		DetMeth id_det_mth = det_mth.parseEnum("DET_METHOD", det_method);
        DET_METHOD = id_det_mth;

		cfg.Get("DET_SAVE_FITS3D", 			    DET_SAVE_FITS3D);
		cfg.Get("DET_SAVE_FITS2D", 				DET_SAVE_FITS2D);
		cfg.Get("DET_SAVE_BMP", 				DET_SAVE_BMP);
		cfg.Get("DET_SAVE_SUM", 				DET_SAVE_SUM);
		cfg.Get("DET_SAVE_AVI", 				DET_SAVE_AVI);
		cfg.Get("DET_SAVE_GEMAP", 				DET_SAVE_GEMAP);
		cfg.Get("DET_SAVE_TRAIL", 				DET_SAVE_TRAIL);
		cfg.Get("DET_SAVE_POS", 				DET_SAVE_POS);
		cfg.Get("DET_DOWNSAMPLE_ENABLED", 		DET_DOWNSAMPLE_ENABLED);
		cfg.Get("DET_TIME_BEFORE", 				DET_TIME_BEFORE);
		cfg.Get("DET_TIME_AFTER", 				DET_TIME_AFTER);
		cfg.Get("DET_GE_MAX", 					DET_GE_MAX);
		cfg.Get("DET_TIME_MAX", 				DET_TIME_MAX);

		/// Stack parameters

		cfg.Get("STACK_ENABLED", 				STACK_ENABLED);
		cfg.Get("STACK_TIME", 					STACK_TIME);
		cfg.Get("STACK_INTERVAL", 				STACK_INTERVAL);

		string stack_method;
		cfg.Get("STACK_MTHD", stack_method);

		StackMeth id_stack_mth = stack_mth.parseEnum("STACK_MTHD", stack_method);
        STACK_MTHD = id_stack_mth;

        cfg.Get("STACK_REDUCTION", 				STACK_REDUCTION);

        /// Mail notifications

        string mailRecipients;
        cfg.Get("MAIL_RECIPIENT",               mailRecipients);
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(";");
        tokenizer tokens(mailRecipients, sep);

        for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
            MAIL_RECIPIENT.push_back(*tok_iter);

        }

        for(int ii = 0; ii< MAIL_RECIPIENT.size(); ii++)
            cout << ii << " : " << MAIL_RECIPIENT.at(ii)<< endl;

        cfg.Get("MAIL_DETECTION_ENABLED",       MAIL_DETECTION_ENABLED);
        cfg.Get("MAIL_SMTP_SERVER",             MAIL_SMTP_SERVER);
        cfg.Get("MAIL_SMTP_HOSTNAME",           MAIL_SMTP_HOSTNAME);

		/// Others parameters

		cfg.Get("DEBUG_ENABLED", 				DEBUG_ENABLED);
		cfg.Get("DEBUG_PATH", 					DEBUG_PATH);
		cfg.Get("STATION_NAME", 				STATION_NAME);
		cfg.Get("CFG_FILECOPY_ENABLED", 		CFG_FILECOPY_ENABLED);
		cfg.Get("DATA_PATH", 					DATA_PATH);
		cfg.Get("LOG_PATH", 					LOG_PATH);

		string log_severity;
		cfg.Get("LOG_SEVERITY", 				log_severity);

		LogSeverityLevel id_log_sev = log_sev.parseEnum("LOG_SEVERITY", log_severity);
        LOG_SEVERITY = id_log_sev;

		cfg.Get("SITELONG", 					LONGITUDE);

		return true;

    }else{

        std::cout << " The path of the configuration file (" << cfgFile << ") doesn't exists" << endl;

        return false;

    }

}

void FreeTure::printParameters(){

    std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"    << endl;
    std::cout << "%%%%%%%%%%%%%%%%%% FreeTure Configuration file %%%%%%%%%%%%%%%%%%"    << endl;
    std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"    << endl
                                                                                        << endl;
    std::cout << "Location : " << cfgFile   << endl
                                            << endl;

    std::cout << "========================== Input ================================"    << endl
                                                                                        << endl;
    std::cout << "=> CAMERA_TYPE                : " << cam_type.getStringEnum(CAMERA_TYPE) << endl;

    std::cout << "=> CAMERA_NAME                : " << CAMERA_NAME                      << endl;
    std::cout << "=> CAMERA_ID                  : " << CAMERA_ID                        << endl;
    std::cout << "=> VIDEO_PATH                 : " << VIDEO_PATH                       << endl;
    std::cout << "=> FRAMES_PATH                : " << FRAMES_PATH                      << endl;
    std::cout << "=> FRAMES_SEPARATOR           : " << FRAMES_SEPARATOR                 << endl;
    std::cout << "=> FRAMES_SEPARATOR_POSITION  : " << FRAMES_SEPARATOR_POSITION        << endl;

    std::cout << endl
              << "======================= Acquisition ============================="    << endl
                                                                                        << endl;

    std::cout <<"=> ACQ_FPS                     : " << ACQ_FPS                          << endl;
    std::cout <<"=> ACQ_BIT_DEPTH               : " << ACQ_BIT_DEPTH                    << endl;
    std::cout <<"=> ACQ_EXPOSURE                : " << ACQ_EXPOSURE                     << endl;
    std::cout <<"=> ACQ_GAIN                    : " << ACQ_GAIN                         << endl;
    std::cout <<"=> ACQ_MASK_ENABLED            : " << ACQ_MASK_ENABLED                 << endl;
    std::cout <<"=> ACQ_MASK_PATH               : " << ACQ_MASK_PATH                    << endl;
    std::cout <<"=> ACQ_BUFFER_SIZE             : " << ACQ_BUFFER_SIZE                  << endl;

    std::cout << endl
              << "======================== Detection =============================="    << endl
                                                                                        << endl;

    std::cout <<"=> DET_ENABLED                 : " << DET_ENABLED                      << endl;
    std::cout <<"=> DET_METHOD                  : " << det_mth.getStringEnum(DET_METHOD) << endl;
    std::cout <<"=> DET_SAVE_FITS3D             : " << DET_SAVE_FITS3D                  << endl;
    std::cout <<"=> DET_SAVE_FITS2D             : " << DET_SAVE_FITS2D                  << endl;
    std::cout <<"=> DET_SAVE_BMP                : " << DET_SAVE_BMP                     << endl;
    std::cout <<"=> DET_SAVE_SUM                : " << DET_SAVE_SUM                     << endl;
    std::cout <<"=> DET_SAVE_AVI                : " << DET_SAVE_AVI                     << endl;
    std::cout <<"=> DET_SAVE_GEMAP              : " << DET_SAVE_GEMAP                   << endl;
    std::cout <<"=> DET_SAVE_TRAIL              : " << DET_SAVE_TRAIL                   << endl;
    std::cout <<"=> DET_SAVE_POS                : " << DET_SAVE_POS                     << endl;
    std::cout <<"=> DET_DOWNSAMPLE_ENABLED      : " << DET_DOWNSAMPLE_ENABLED           << endl;
    std::cout <<"=> DET_TIME_BEFORE             : " << DET_TIME_BEFORE                  << endl;
    std::cout <<"=> DET_TIME_AFTER              : " << DET_TIME_AFTER                   << endl;
    std::cout <<"=> DET_GE_MAX                  : " << DET_GE_MAX                       << endl;
    std::cout <<"=> DET_TIME_MAX                : " << DET_TIME_MAX                     << endl;

    std::cout << endl
              << "=========================== Stack ==============================="    << endl
                                                                                        << endl;

    std::cout <<"=> STACK_ENABLED               : " << STACK_ENABLED                    << endl;
    std::cout <<"=> STACK_TIME                  : " << STACK_TIME                       << endl;
    std::cout <<"=> STACK_INTERVAL              : " << STACK_INTERVAL                   << endl;
    std::cout <<"=> STACK_MTHD                  : " << stack_mth.getStringEnum(STACK_MTHD) << endl;
    std::cout <<"=> STACK_REDUCTION             : " << STACK_REDUCTION                  << endl;

    std::cout << endl
              << "=========================== Others =============================="    << endl
                                                                                        << endl;

    std::cout <<"=> DEBUG_ENABLED               : " << DEBUG_ENABLED                    << endl;
    std::cout <<"=> DEBUG_PATH                  : " << DEBUG_PATH                       << endl;

    std::cout <<"=> STATION_NAME                : " << STATION_NAME                     << endl;
    std::cout <<"=> CFG_FILECOPY_ENABLED        : " << CFG_FILECOPY_ENABLED             << endl;
    std::cout <<"=> DATA_PATH                   : " << DATA_PATH                        << endl;
    std::cout <<"=> LOG_PATH                    : " << LOG_PATH                         << endl;
    std::cout <<"=> LOG_SEVERITY                : " << log_sev.getStringEnum(LOG_SEVERITY) << endl;
    std::cout <<"=> LONGITUDE                   : " << LONGITUDE                        << endl;

}
