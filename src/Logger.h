/*
                                Logger.h

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
*   Last modified:      20/07/2015
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
#include <numeric>
#include "TimeDate.h"
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/bind.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include "TimeDate.h"
#include "Conversion.h"

class Logger {

    private :

        string mLogPath;
        int mTimeLimit;
        int mSizeLimit;
        vector<string> mLogFiles;
        vector<int> mRefDate;

    public :

        /**
        * Constructor.
        *
        */
        Logger(string logPath, int timeLimit, int sizeLimit, vector<string> logFiles):
        mLogPath(logPath), mTimeLimit(timeLimit), mSizeLimit(sizeLimit), mLogFiles(logFiles) {

            mRefDate = TimeDate::splitStringToInt(TimeDate::localDateTime(microsec_clock::universal_time(),"%Y:%m:%d:%H:%M:%S"));

        }

        void monitorLog() {

            vector<int> currDate = TimeDate::splitStringToInt(TimeDate::localDateTime(microsec_clock::universal_time(),"%Y:%m:%d:%H:%M:%S"));

            //cout << "REFDATE : " << mRefDate.at(0) << mRefDate.at(1) << mRefDate.at(2) << endl;
            //cout << "CURDATE : " << currDate.at(0) << currDate.at(1) << currDate.at(2) << endl;

            // Create log date directories when date changes.
            if(mRefDate.at(0) != currDate.at(0) || mRefDate.at(1) != currDate.at(1) || mRefDate.at(2) != currDate.at(2)) {

                string rDate = Conversion::numbering(2, mRefDate.at(0)) + Conversion::intToString(mRefDate.at(0)) + Conversion::numbering(2, mRefDate.at(1)) + Conversion::intToString(mRefDate.at(1)) + Conversion::numbering(2, mRefDate.at(2)) + Conversion::intToString(mRefDate.at(2));
                //cout << rDate << endl;
                if(create_directory(path(mLogPath + "/LOG_" + rDate)) || exists(path(mLogPath + "/LOG_" + rDate))) {

                    //cout << mLogPath << "/LOG_" << rDate << " created." << endl;

                    for(int i = 0; i < mLogFiles.size(); i++) {

                        try {

                            rename(path(mLogPath + "/" + mLogFiles.at(i)), path(mLogPath + "/LOG_" + rDate + "/" + mLogFiles.at(i)));
                            //cout << "RENAME : " << mLogPath << "/" << mLogFiles.at(i) << " TO " << mLogPath << "/LOG_" << rDate << "/" + mLogFiles.at(i) << endl;

                        }catch(boost::filesystem::filesystem_error e) {

                            cout <<"filesystem error" << endl;

                        }
                    }

                    // Clean logs directories

                    boost::posix_time::ptime t1(boost::posix_time::from_iso_string(rDate + "T000000"));
                    int dirNb = 0;
                    vector<string> dirToRemove;
                    path pDir(mLogPath);
                    //cout << ">> LOOP DIR :  "<< endl;
                    for(directory_iterator file(pDir);file!= directory_iterator(); ++file) {

                        path curr(file->path());
                        //cout << "-> " << curr << endl;

                        if(is_directory(curr)) {

                            string dirName = curr.filename().string();

                            vector<string> output;
                            typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                            boost::char_separator<char> sep("_");
                            tokenizer tokens(dirName, sep);

                            for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter) {
                                output.push_back(*tok_iter);
                            }

                            if(output.size() == 2 && output.back().size() == 8) {

                                boost::posix_time::ptime t2(boost::posix_time::from_iso_string(output.back() + "T000000"));

                                boost::posix_time::time_duration td = t1 - t2;
                                long secTime = td.total_seconds();
                                //cout << secTime << endl;

                                if(abs(secTime) > mTimeLimit * 24 * 3600) {
                                    dirToRemove.push_back(curr.string());
                                }

                            }else{
                                dirToRemove.push_back(curr.string());
                                //cout << "remove : " << curr.string() << endl;
                            }
                        }
                    }

                    //cout << ">> SIZE dirToRemove : " << dirToRemove.size() << endl;

                    for(int i = 0; i < dirToRemove.size(); i++) {

                        //cout << ">> REMOVE : " << dirToRemove.at(i) << endl;
                        boost::filesystem::remove_all(path(dirToRemove.at(i)));

                    }

                    mRefDate = currDate;

                }/*else
                    cout << "DIR not exists" << endl;*/

            }

            // Check log size.
            unsigned long long logSize = 0;
            getFoldersize(mLogPath, logSize);
            //cout << "LOG SIZE : " <<  logSize << endl;
            if((logSize/1024.0)/1024.0 > mSizeLimit)
                boost::filesystem::remove_all(path(mLogPath));

        }

    private :

        void  getFoldersize(string rootFolder,unsigned long long & f_size) {

           path folderPath(rootFolder);

           if (exists(folderPath)) {

                directory_iterator end_itr;

                for (directory_iterator dirIte(rootFolder); dirIte != end_itr; ++dirIte ) {

                    path filePath(complete (dirIte->path(), folderPath));

                    try{

                        if (!is_directory(dirIte->status()) ){
                            f_size = f_size + file_size(filePath);
                        }else{
                            getFoldersize(filePath.string(),f_size);
                        }

                    }catch(exception& e){  cout << e.what() << endl; }
                }
            }
        }
};
