/*
								CameraFrames.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau
*                               FRIPON-GEOPS-UPSUD-CNRS
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
* \file    CameraFrames.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    02/09/2014
* \brief   Acquisition thread with fits individual frames in input.
*/

#include "CameraFrames.h"

boost::log::sources::severity_logger< LogSeverityLevel >  CameraFrames::logger;
CameraFrames::_Init CameraFrames::_initializer;

CameraFrames::CameraFrames(vector<string> dir, int nbPos){

	framesDir			= dir;
    numPosInName		= nbPos;
	endReadDataStatus	= false;
	framesetID = 0;
    currentFramesFir = dir.front();

}

CameraFrames::~CameraFrames(void){}

bool CameraFrames::loadData(){

    if(framesetID !=0 ){

        currentFramesFir = framesDir.at(framesetID);
        endReadDataStatus = false;
        if(!extractFrameNumbers(currentFramesFir))
            return false;

    }

    return true;

}

bool CameraFrames::grabStart(){

	return extractFrameNumbers(currentFramesFir);

}

bool CameraFrames::getDataStatus(){

    framesetID++;

    if(framesetID == framesDir.size())
        return false;
    else
        return true;
}

bool CameraFrames::extractFrameNumbers(string location){

    namespace fs = boost::filesystem;

    path p(location);

    if(fs::exists(p)){

		BOOST_LOG_SEV(logger, normal) << "Frame's directory exists : " << location;

        int firstFrame = -1, lastFrame = 0;
        string filename = "";

        /// Search first and last frames numbers in the directory.
        for(directory_iterator file(p);file!= directory_iterator(); ++file){

            path curr(file->path());
            //cout << file->path() << endl;

            if(is_regular_file(curr)){

				// Get file name.
				string fname = curr.filename().string();

				// Split file name according to the separator "_".
				vector<string> output;
				typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
				boost::char_separator<char> sep("_");
				tokenizer tokens(fname, sep);
				for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter){
					output.push_back(*tok_iter);
				}

				// Search frame number according to the number position known in the file name.

                int i = 0, number = 0;

                for(int j = 0; j < output.size(); j++){

                    if(j == numPosInName && j != output.size() - 1){

                        number = atoi(output.at(j).c_str());
						break;
                    }

					// If the frame number is at the end (before the file extension).
                    if(j == numPosInName && j == output.size() - 1){

						vector<string> output2;
						typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
						boost::char_separator<char> sep2(".");
						tokenizer tokens2(output.back(), sep2);
						for (tokenizer::iterator tok_iter = tokens2.begin();tok_iter != tokens2.end(); ++tok_iter){
							output2.push_back(*tok_iter);
						}

                        number = atoi(output2.front().c_str());
                        break;

                    }

                    i++;

                }

                if(firstFrame == -1){

                    firstFrame = number;

                }else if(number < firstFrame){

                    firstFrame = number;

                }

                if(number > lastFrame){

                    lastFrame = number;

                }

            }

        }

		BOOST_LOG_SEV(logger, normal) << "First frame number in frame's directory : " << firstFrame;
		BOOST_LOG_SEV(logger, normal) << "Last frame number in frame's directory : " << lastFrame;


		lastNumFrame = lastFrame;
		firstNumFrame = firstFrame;

		return true;

	}else{

		BOOST_LOG_SEV(logger, fail) << "Frame's directory not found.";
		cout << "Frame's directory not found." << endl;
		return false;

	}

}

bool CameraFrames::getStopStatus(){
	return endReadDataStatus;
}

bool CameraFrames::grabImage(Frame &img){

    bool fileFound = false;

    string filename = "";

	path p(currentFramesFir);

    /// Search a frame in the directory.
    for(directory_iterator file(p);file!= directory_iterator(); ++file){

        path curr(file->path());

        if(is_regular_file(curr)){

            list<string> ch;
			string fname = curr.filename().string();
			Conversion::stringTok(ch, fname.c_str(), "_");
            list<string>::const_iterator lit(ch.begin()), lend(ch.end());
            int i = 0;
            int number = 0;

            for(; lit != lend; ++lit){

                if(i == numPosInName && i != ch.size() - 1){

                    number = atoi((*lit).c_str()); break;
                }

                if(i == ch.size() - 1){

					list<string> ch_;
					Conversion::stringTok(ch_, (*lit).c_str(), ".");
					number = atoi(ch_.front().c_str());
					break;

				}

                i++;

            }

            if(number == firstNumFrame){

                firstNumFrame++;
                fileFound = true;

                cout << "FILE:" << file->path().string() << endl;
				BOOST_LOG_SEV(logger, normal) <<  "FILE:" << file->path().string();

				filename = file->path().string() ;

                break;

            }
        }
    }


    if(firstNumFrame > lastNumFrame || !fileFound){

		endReadDataStatus = true;
		BOOST_LOG_SEV(logger, normal) <<  "End read frames.";
		return false;

    }else{

		BOOST_LOG_SEV(logger, normal) <<  "Frame found.";

		Fits2D newFits;
        int bitpix;

        if(!newFits.readIntKeyword(filename, "BITPIX", bitpix)){
			BOOST_LOG_SEV(logger, fail) << " Fail to read fits keyword : BITPIX";

			return false;
        }

		/// Read the frame.

		Mat resMat;
		CamBitDepth frameFormat;

		switch(bitpix){

			case 8 :
				frameFormat = MONO_8;
				newFits.readFits8UC(resMat, filename);

				break;

			case 16 :
				frameFormat = MONO_12;

				//newFits.readFits16S(resMat, filename);
				newFits.readFits16US(resMat, filename);

				break;

		}

		string acquisitionDate = TimeDate::localDateTime(microsec_clock::universal_time(),"%Y:%m:%d:%H:%M:%S");
		boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
		string acqDateInMicrosec = to_iso_extended_string(time);

		Frame f = Frame(resMat, 0, 0, acquisitionDate);

		img = f;

		img.setAcqDateMicro(acqDateInMicrosec);
        cout << "set num frame: " << firstNumFrame-1<<  endl;
		img.setNumFrame(firstNumFrame -1 );
		img.setFrameRemaining(lastNumFrame - firstNumFrame-1);
		img.setFPS(1);
		img.setBitDepth(frameFormat);

		waitKey(300);

		return true;

	}

}
