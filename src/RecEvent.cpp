/*
								RecEvent.cpp

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
* \file    RecEvent.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Save a detected event.
*/

#include "RecEvent.h"

bool RecEvent::buildEventLocation(vector<string> eventDate, string eventPath, string stationName, string &currentEventPath){

    namespace fs = boost::filesystem;
    cout << "start buildEventLocation" << endl;
    //STATION_AAMMDD
    string root = eventPath + stationName + "_" + eventDate.at(0) + eventDate.at(1) + eventDate.at(2) +"/";
    cout << root << endl;
    //events
    string sub0 = "events/";

    //STATION_AAAAMMDD_HHMMSS_UT
    string sub1 = stationName + "_" + eventDate.at(0)
                                    + eventDate.at(1)
                                    + eventDate.at(2) + "_"
                                    + eventDate.at(3)
                                    + eventDate.at(4)
                                    + eventDate.at(5) + "_UT/";



    currentEventPath = root + sub0 + sub1;
    cout << currentEventPath << endl;
    // DataLocation/
    path p(eventPath);

    // DataLocation/STATION_AAMMDD/
    path p0(root);

    // DataLocation/STATION_AAMMDD/events/
    string path1 = root + sub0;
    path p1(path1);

    // DataLocation/STATION_AAMMDD/events/STATION_AAAAMMDD_HHMMSS_UT/
    string path2 = root + sub0 + sub1;
    path p2(path2);

    // If DataLocation/ exists.
    if(fs::exists(p)){

        // If DataLocation/STATION_AAMMDD/ exists.
        if(fs::exists(p0)){

            // If DataLocation/STATION_AAMMDD/events/ exists.
            if(fs::exists(p1)){

                // If DataLocation/STATION_AAMMDD/events/STATION_AAAAMMDD_HHMMSS_UT/ not exists.
                if(!fs::exists(p2)){

                    if(!fs::create_directory(p2)){
                        cout << "Can't create : " << path2 << endl;
                        return false;

                    }else{
                        cout << path2 << " created " << endl;
                        return true;
                    }

                }

            }else{

                // Create the destination directory
                if(!fs::create_directory(p1)){
                    cout << "Can't create : " << path1 << endl;
                    return false;

                }else{
                    cout << path1 << " created " << endl;
                    // Create the destination directory
                    if(!fs::create_directory(p2)){
                        cout << "Can't create : " << path2 << endl;
                        return false;

                    }else{
                        cout << path2 << " created " << endl;
                        return true;

                    }

                }
            }

        }else{

            // Create the destination directory
            if(!fs::create_directory(p0)){
                cout << "Can't create : " << root << endl;
                return false;

            }else{

                if(!fs::create_directory(p1)){
                    cout << "Can't create : " << path1 << endl;
                    return false;

                }else{

                    if(!fs::create_directory(p2)){
                        cout << "Can't create : " << path2 << endl;
                        return false;

                    }else{

                        return true;

                    }

                }
            }
        }

    }else{

        if(!fs::create_directory(p)){
            cout << "Can't create : " << eventPath << endl;
            return false;

        }else{

            // Create the destination directory
            if(!fs::create_directory(p0)){
                cout << "Can't create : " << root << endl;
                return false;

            }else{

                if(!fs::create_directory(p1)){
                    cout << "Can't create : " << path1 << endl;
                    return false;

                }else{

                    if(!fs::create_directory(p2)){
                        cout << "Can't create : " << path2 << endl;
                        return false;

                    }else{

                        return true;

                    }
                }
            }
        }
    }

    return false;
}

bool RecEvent::saveGE(  boost::circular_buffer<Frame>  *frameBuffer,
                        vector<GlobalEvent> &GEList,
                        vector<GlobalEvent>::iterator itGE,
                        Fits fitsHeader,
                        bool downsample,
                        bool recAvi,
                        bool recFits3D,
                        bool recFits2D,
                        bool recPos,
                        bool recSum,
                        bool recBmp,
                        bool recMapGE,
                        int timeAfter,
                        int timeBefore,
                        int frameBufferMaxSize,
                        bool mailNotification,
                        string SMTPServer,
                        string SMTPHostname,
                        vector<string> mailRecipients,
                        string eventPath,
                        string stationName,
                        string currentEventPath,
                        CamBitDepth pixelFormat){

    namespace fs = boost::filesystem;

     vector<string> mailAttachments;

    /// SAVE mapGE

    if(recMapGE){

        SaveImg::saveBMP((*itGE).getMapEvent(), currentEventPath + "GEMap");
        mailAttachments.push_back(currentEventPath + "GEMap.bmp");

        SaveImg::saveBMP((*itGE).getDirMap(), currentEventPath + "DirMap");

    }

    // Number of the first frame where the event is detected.
    int numFirstFrameEvent = (*itGE).getNumFirstFrame();

    // Number of the last frame where the event is detected.
    int numLastFrameEvent = (*itGE).getNumLastFrame();
    // Number of the first frame to save. It depends of how many frames we want to keep before the event.

    int numFirstFrameToSave = numFirstFrameEvent - timeBefore;
    int shiftPosition = timeBefore;

    // Number of the last frame to save. It depends of how many frames we want to keep after the event.
    int numLastFrameToSave = numLastFrameEvent + timeAfter;

    if(frameBuffer->front().getNumFrame() > numFirstFrameToSave){
        numFirstFrameToSave = frameBuffer->front().getNumFrame();
        shiftPosition = numFirstFrameEvent - frameBuffer->front().getNumFrame();
    }

    if(frameBuffer->back().getNumFrame() < numLastFrameToSave)
           numLastFrameToSave = frameBuffer->back().getNumFrame();

    cout << "> Num first frame in buffer : " << numFirstFrameToSave << endl;
    cout << "> Num last frame in buffer : " << numLastFrameToSave << endl;
    cout << "> Num first frame of event : " << numFirstFrameEvent << endl;
    cout << "> Num last frame of event : " << numLastFrameEvent << endl;
    cout << "> Time to save before : " << timeBefore << endl;
    cout << "> Time to save after : " << timeAfter << endl;

    int c = 0;
    int nbTotalFramesToSave = numLastFrameToSave - numFirstFrameToSave;

    cout << "> nbTotalFramesToSave : " << nbTotalFramesToSave << endl;

    // Count number of digit on nbTotalFramesToSave.
    int n = nbTotalFramesToSave;
    int nbDigitOnNbTotalFramesToSave = 0;
    while(n!=0){
      n/=10;
      ++nbDigitOnNbTotalFramesToSave;
    }

    cout << "> nbDigitOnNbTotalFramesToSave : " << nbDigitOnNbTotalFramesToSave << endl;

    vector<int> dateFirstFrame;
    float dateSecFirstFrame = 0.0;

    /// POSITIONS FILE

    if(recPos){

        ofstream posFile;
        string posFilePath = currentEventPath + "positions.txt";
        posFile.open(posFilePath.c_str());

        vector<LocalEvent>::iterator itLE;

        for(itLE = (*itGE).LEList.begin(); itLE!=(*itGE).LEList.end(); ++itLE){

            Point pos;
            pos = (*itLE).getMassCenter();

            if(downsample){

                pos*=2;

            }

            string line = Conversion::intToString((*itLE).getNumFrame() - numFirstFrameToSave) + "               (" + Conversion::intToString(pos.x)  + ";" + Conversion::intToString(pos.y) + ")\n";
            posFile << line;

        }

        // infos

        posFile.close();

    }


    /// INFOS DET
    bool infos = true;
    if(infos){

        ofstream infFile;
        string infFilePath = currentEventPath + "infos.txt";
        infFile.open(infFilePath.c_str());


        infFile << " * AGE              : " << (*itGE).getAge() << "\n";
        infFile << " * AGE LAST ELEM    : " << (*itGE).getAgeLastElem()<< "\n";
        infFile << " * LINEAR STATE     : " << (*itGE).getLinearStatus()<< "\n";
        infFile << " * BAD POS          : " << (*itGE).getBadPos()<< "\n";
        infFile << " * GOOD POS         : " << (*itGE).getGoodPos()<< "\n";
        infFile << " * Num first frame  : " << (*itGE).getNumFirstFrame()<< "\n";
        infFile << " * Num last frame   : " << (*itGE).getNumLastFrame()<< "\n";
        //infFile << " * Static Test      : " << (*itGE).getStaticTestRes();
        infFile << "\n mainPoint : \n";

        for(int i = 0; i < (*itGE).mainPoints.size(); i++){

            infFile << "(" << (*itGE).mainPoints.at(i).x << ";"<<  (*itGE).mainPoints.at(i).y << ")\n";

        }

        infFile << "\n distance : \n";

         for(int i = 0; i < (*itGE).dist.size(); i++){

            infFile << (*itGE).dist.at(i)<< "\n";

        }

        infFile.close();
/*
        vector<LocalEvent>::iterator itLE;
        int cpt = 0;

        for(itLE = (*itGE).LEList.begin(); itLE!=(*itGE).LEList.end(); ++itLE){

            SaveImg::saveBMP((*itGE).getMapEvent(), currentEventPath + "LEMap_" + Conversion::intToString(cpt));
            cpt++;

        }*/

        SaveImg::saveBMP((*itGE).getDirMap2(), currentEventPath + "DirMap2");

        mailAttachments.push_back(currentEventPath + "DirMap2.bmp");

    }

    /// SAVE AVI

    VideoWriter oVideoWriter;

    if(recAvi){

        Size frameSize(static_cast<int>(frameBuffer->front().getImg().cols), static_cast<int>(frameBuffer->front().getImg().cols));
        oVideoWriter = VideoWriter(currentEventPath + "video_" +".avi", CV_FOURCC('D', 'I', 'V', 'X'), 15, frameSize, false);

    }

    /// SAVE FITS3D

    Fits3D fits3d;

    if(recFits3D){

        fits3d = Fits3D(pixelFormat, frameBuffer->front().getImg().rows, frameBuffer->front().getImg().cols, (numLastFrameToSave - numFirstFrameToSave +1), fitsHeader);
        boost::posix_time::ptime time = boost::posix_time::microsec_clock::universal_time();
        fits3d.setDate(to_iso_extended_string(time));

        // Name of the fits file.
        fits3d.setFilename("fits3d.fit");

    }

    Mat stackEvent = Mat::zeros(frameBuffer->front().getImg().rows, frameBuffer->front().getImg().cols, CV_32FC1);

    cout << "> Loop frame buffer ... " << endl;

    boost::circular_buffer<Frame>::iterator it;
    for(it = frameBuffer->begin(); it != frameBuffer->end(); ++it){

        cout << "> NUM FRAME : " << (*it).getNumFrame() << endl;

        // Get infos about the first frame of the event record for fits 3D.
        if((*it).getNumFrame() == numFirstFrameToSave && recFits3D){

            fits3d.setDateobs((*it).getAcqDateMicro());
            // Exposure time.
            fits3d.setOntime((*it).getExposure());
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
           // fits3d.setExposure((1.0/(*it).getFPS()));
            cout << "EXPOSURE : " << 1.0f/(*it).getFPS() << endl;

            dateFirstFrame = (*it).getDate();
            dateSecFirstFrame = (*it).getDateSeconds();

        }

        // Get infos about the last frame of the event record for fits 3D.
        if((*it).getNumFrame() == numLastFrameToSave && recFits3D){
            cout << "DATE first : " << dateFirstFrame.at(3) << " H " << dateFirstFrame.at(4) << " M " << dateSecFirstFrame << " S" << endl;
            cout << "DATE last : " << (*it).getDate().at(3) << " H " << (*it).getDate().at(4) << " M " << (*it).getDateSeconds() << " S" << endl;
            fits3d.setElaptime(((*it).getDate().at(3)*3600 + (*it).getDate().at(4)*60 + (*it).getDateSeconds()) - (dateFirstFrame.at(3)*3600 + dateFirstFrame.at(4)*60 + dateSecFirstFrame));

        }

        // For each frame to save
        if((*it).getNumFrame() >= numFirstFrameToSave && (*it).getNumFrame() <= numLastFrameToSave){

            Mat currFrame;
            (*it).getImg().copyTo(currFrame);

            Mat temp;

            if(pixelFormat == MONO_12)
                currFrame.convertTo(temp, CV_8UC1, 255, 0);


            /// SAVE BMP

            if(recBmp){

                /*path p(currentEventPath + "BMP/");

                if(!fs::exists(p))
                    fs::create_directory(p);

                imwrite(currentEventPath + "BMP/frame_" + Conversion::intToString(c) + ".bmp", temp);*/

            }

            /// SAVE AVI

            if(recAvi){

                if(oVideoWriter.isOpened())
                    oVideoWriter << temp;

            }

            /// SAVE FITS2D

            if(recFits2D){

                string fits2DPath = currentEventPath + "fits2D/";
                string fits2DName = "frame_" + Conversion::numbering(nbDigitOnNbTotalFramesToSave, c) + Conversion::intToString(c);
                vector<string> DD;

                cout << "Save fits 2D  : " << fits2DName << endl;

                path p(fits2DPath);

                Fits2D newFits(fits2DPath, fitsHeader);
                // Frame's acquisition date.
                newFits.setDateobs((*it).getAcqDateMicro());
                // Fits file creation date.
                boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
                // YYYYMMDDTHHMMSS,fffffffff where T is the date-time separator
                newFits.setDate(to_iso_string(time));
                // Name of the fits file.
                newFits.setFilename(fits2DName);
                // Exposure time.
                newFits.setOntime((*it).getExposure());
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
                newFits.setExposure((1.0f/(*it).getFPS()));
                cout << "EXPOSURE : " << 1.0/(*it).getFPS() << endl;
                // Projection and reference system
                newFits.setCtype1("RA---ARC");
                newFits.setCtype2("DEC--ARC");
                // Equinox
                newFits.setEquinox(2000.0);

                if(!fs::exists(p)) fs::create_directory(p);

                if(pixelFormat == MONO_8){
                    newFits.setBzero(128);
                    newFits.writeFits((*it).getImg(), C8, DD, false, fits2DName);

                }else{
                    newFits.setBzero(32768);
                    newFits.writeFits((*it).getImg(), S16, DD, false, fits2DName);
                }
            }

            /// SAVE FITS3D

            if(recFits3D){

                cout << "addData" << endl;

                fits3d.addImageToFits3D((*it).getImg());

            }

            c++;

            if((*it).getNumFrame() >= numFirstFrameEvent && (*it).getNumFrame() <= numLastFrameEvent){

                accumulate((*it).getImg(), stackEvent);
            }

        }
    }

    /// SAVE FITS3D

    if(recFits3D){

        cout << "save : " << currentEventPath + "fits3D" << endl;

        fits3d.writeFits3D(currentEventPath + "fits3D");

    }


    /// SAVE SUM


    if(recSum){

        bool stackReduction = true;

        Fits2D newFits(currentEventPath, fitsHeader);
        newFits.setObsmode("SUM");
        string sumFileName = "sum";
        mailAttachments.push_back(currentEventPath + sumFileName + ".fit");

        vector<string> DD;

        if(stackReduction){

            Mat newMat ;

            float bzero     = 0.0;
            float bscale    = 1.0;

            ImgReduction::dynamicReductionByFactorDivision(stackEvent, pixelFormat, c, bzero, bscale).copyTo(newMat);

            newFits.setBzero(bzero);
            newFits.setBscale(bscale);

            newFits.writeFits(newMat, S16, DD, false, sumFileName);


        }else{

            // Save fits in 32 bits.
            newFits.writeFits(stackEvent, F32 , DD, false, sumFileName);

        }

    }

    if(mailNotification){
        cout << "Send mail " << endl;

        SMTPClient mailc(SMTPServer, 25, SMTPHostname);

        mailc.send("yoan.audureau@u-psud.fr",
                   mailRecipients,
                   "Detection by " + stationName  + "'s station - " + (*itGE).getDate().at(0) + (*itGE).getDate().at(1) + (*itGE).getDate().at(2) + "_" + (*itGE).getDate().at(3) + (*itGE).getDate().at(4) + (*itGE).getDate().at(5) + "_UT",
                   stationName + "\n" + (*itGE).getDate().at(0) + (*itGE).getDate().at(1) + (*itGE).getDate().at(2) + "_" + (*itGE).getDate().at(3) + (*itGE).getDate().at(4) + (*itGE).getDate().at(5) + "_UT\n" + currentEventPath,
                   mailAttachments,
                   false);


    }



    //lock.unlock();

    return true;

}

