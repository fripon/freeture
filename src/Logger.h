/*
                                Logger.h

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
* \file    Logger.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
*/

#pragma once

#include <boost/filesystem.hpp>
#include <string>
#include "TimeDate.h"
#include <boost/date_time/gregorian/greg_date.hpp>

class Logger {

    private :

        string mLogPath;
        // The archives timestamped from now to this nummber of day in the past are kept. The others are removed.
        int mTimeLimit;
        vector<int> mDate;

    public :

        /**
        * Constructor.
        * 
        */
        Logger(string logPath, int timeLimit):mLogPath(logPath), mTimeLimit(timeLimit) {

        }

        /**
        * Archive log files. 
        * 
        */
        void archiveLog() {

            // Get current date.
            mDate = TimeDate::splitStringToIntVector(TimeDate::localDateTime(microsec_clock::universal_time(),"%Y:%m:%d:%H:%M:%S"));

            path pp(mLogPath);

            bool logStackThread = false;
            bool logAcqThread = false;
            bool logDetThread = false;
            bool logMainThread = false;

            vector<string> logFile;
            vector<path> logToCopy;

            /// Search logs files in the log directory.
            for(directory_iterator file(pp); file!= directory_iterator(); ++file) {

                path curr(file->path());

                if(is_regular_file(curr)) {

                    // Get file name.
	                string fileName = curr.filename().string();
    
                    if(fileName == "STACK_THREAD.log"){

                        cout << "STACK_THREAD.log found" << endl;
                        logStackThread = true;
                        logToCopy.push_back(curr);
                        logFile.push_back("STACK_THREAD.log");

                    }

                    if(fileName == "MAIN_THREAD.log"){

                        cout << "MAIN_THREAD.log found" << endl;
                        logMainThread = true;
                        logToCopy.push_back(curr);
                        logFile.push_back("MAIN_THREAD.log");

                    }

                    if(fileName == "DET_THREAD.log"){

                        cout << "DET_THREAD.log found" << endl;
                        logDetThread = true;
                        logToCopy.push_back(curr);
                        logFile.push_back("DET_THREAD.log");
                    }

                    if(fileName == "ACQ_THREAD.log"){

                        cout << "ACQ_THREAD.log found" << endl;
                        logAcqThread = true;
                        logToCopy.push_back(curr);
                        logFile.push_back("ACQ_THREAD.log");

                    }

                }

            }

            if(logToCopy.size() != 0 && mDate.size() == 6) {
                 
                // Create archive name with the following pattern : LOG_YYYYMMDDhhmmss.log
                string newArchive = mLogPath + "LOG_" +
                                    Conversion::numbering(2,mDate.at(0)) + Conversion::intToString(mDate.at(0)) + 
                                    Conversion::numbering(2,mDate.at(1)) + Conversion::intToString(mDate.at(1)) + 
                                    Conversion::numbering(2,mDate.at(2)) + Conversion::intToString(mDate.at(2)) + 
                                    Conversion::numbering(2,mDate.at(3)) + Conversion::intToString(mDate.at(3)) +
                                    Conversion::numbering(2,mDate.at(4)) + Conversion::intToString(mDate.at(4)) +
                                    Conversion::numbering(2,mDate.at(5)) + Conversion::intToString(mDate.at(5)) + "/";

                path ppp(newArchive);

                // Create a directory with the name of the archive.
                if(boost::filesystem::create_directory(ppp)) {

                    // For each found log, copy it in the new directory and remove it.
                    for(int i = 0; i<logToCopy.size(); i++) {

                        path pppp(newArchive + logFile.at(i));
                        boost::filesystem::copy_file(logToCopy.at(i), pppp);
                        boost::filesystem::remove(logToCopy.at(i));

                    }
                }

                // Compress the new directory to create an archive.

            }

        }

        /**
        * Delete too old log archives. 
        * 
        */
        void cleanLogArchives() {

            path pp(mLogPath);

            vector<string> fileToRemove;

            int timeLimit = 5;

            // Search old logs archives in the log directory.
            for(directory_iterator file(pp); file!= directory_iterator(); ++file) {

                path curr(file->path());

                int year, month, day;

                if(is_directory(curr)) {

                    string dirName = file->path().filename().string();
           
                    // Extract YYYYMMDDhhmmss from LOG_YYYYMMDDhhmmss
                    list<string> ch;
                    Conversion::stringTok(ch, dirName.c_str(), "_");

                    year = atoi(ch.back().substr(0,4).c_str());

                    month = atoi(ch.back().substr(4,2).c_str());
 
                    day = atoi(ch.back().substr(6,2).c_str());

                    using boost::gregorian::date;

                    date a( year, month, day );
                    date b(mDate.at(0), mDate.at(1), mDate.at(2));

                    if((b-a).days() > timeLimit) {

                        fileToRemove.push_back(dirName);

                    }
                                
                }
            }

            // Remove old archives.
            for(int i = 0; i < fileToRemove.size(); i++) {

                cout << fileToRemove.at(i) << endl;
                path p(mLogPath + fileToRemove.at(i));
                boost::filesystem::remove_all(p);
                            
            }

        }

};