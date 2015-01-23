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
 * @file    RecEvent.cpp
 * @author  Yoan Audureau
 * @version 1.0
 * @date    03/06/2014
 * @section DESCRIPTION
 *
 * The detection class contains all meteor detection methods
 */


#include "RecEvent.h"

RecEvent::RecEvent( boost::circular_buffer<Frame> *cb,
                    boost::mutex *m_cb,
                    string path,
                    string station,
                    CamBitDepth  bitdepth,
                    bool avi,
                    bool fits3D,
                    bool fits2D,
                    bool sum,
                    bool pos,
                    bool bmp,
                    bool mapGE,
                    Fits fitsHead,
                    int tBefore,
                    int tAfter,
                    int bufferSize){

    frameBuffer             = cb;
    m_frameBuffer           = m_cb;
    fitsHeader              = fitsHead;
    pixelFormat             = bitdepth;
    eventPath               = path;
    recAvi                  = avi;
    recFits3D               = fits3D;
    recFits2D               = fits2D;
	recPos                  = pos;
	recSum                  = sum;
	recBmp                  = bmp;
	recMapGE                = mapGE;
	timeAfter               = tAfter;
	timeBefore              = tBefore;
	frameBufferMaxSize      = bufferSize;

	if(station == "")
        stationName = "station";
    else
        stationName = station;

}

RecEvent::~RecEvent(){
    //dtor
}

bool RecEvent::buildEventLocation(vector<string> eventDate){

    namespace fs = boost::filesystem;

    //STATION_AAMMDD
    string root = eventPath + stationName + "_" + eventDate.at(0) + eventDate.at(1) + eventDate.at(2) +"/";

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

                // If DataLocation/STATION_AAMMDD/events/STATION_AAAAMMDD_HHMMSS_UT/ exists.
                if(!fs::exists(p2)){

                    if(!fs::create_directory(p2)){

                        return false;

                    }else{

                        return true;
                    }

                }

            }else{

                // Create the destination directory
                if(!fs::create_directory(p1)){

                    return false;

                }else{

                    // Create the destination directory
                    if(!fs::create_directory(p2)){

                        return false;

                    }else{

                        return true;

                    }

                }
            }

        }else{

            // Create the destination directory
            if(!fs::create_directory(p0)){

                return false;

            }else{

                if(!fs::create_directory(p1)){

                    return false;

                }else{

                    if(!fs::create_directory(p2)){

                        return false;

                    }else{

                        return true;

                    }

                }
            }
        }

    }else{

        if(!fs::create_directory(p)){

            return false;

        }else{

            // Create the destination directory
            if(!fs::create_directory(p0)){

                return false;

            }else{

                if(!fs::create_directory(p1)){

                    return false;

                }else{

                    if(!fs::create_directory(p2)){

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

bool RecEvent::saveGE(vector<GlobalEvent> &GEList, vector<GlobalEvent>::iterator itGE){

    namespace fs = boost::filesystem;

    /// POSITIONS FILE

    if(recPos){

        ofstream posFile;
        string posFilePath = currentEventPath + "positions.txt";
        posFile.open(posFilePath.c_str());
        posFile << "num_frame (x;y)\n";

        // infos

        posFile.close();

    }

    /// SAVE mapGE

    if(recMapGE){

        SaveImg::saveBMP((*itGE).getMapEvent(), currentEventPath + "GEMap");

    }

    int numFirstFrameEvent = (*itGE).getNumFirstFrame();
    int numLastFrameEvent = (*itGE).getNumLastFrame();

    int numFirstFrameToSave = numFirstFrameEvent - timeBefore;
    int numLastFrameToSave = numLastFrameEvent - timeAfter;

    int c = 1;

    boost::mutex::scoped_lock lock(*m_frameBuffer);

    if(frameBuffer->front().getNumFrame() > numFirstFrameToSave)
        numFirstFrameToSave = frameBuffer->front().getNumFrame();

    if(frameBuffer->back().getNumFrame() < numLastFrameToSave)
        numLastFrameToSave = frameBuffer->back().getNumFrame();

    /// SAVE AVI

    VideoWriter oVideoWriter;

    if(recAvi){

        Size frameSize(static_cast<int>(frameBuffer->front().getImg().cols), static_cast<int>(frameBuffer->front().getImg().cols));
        oVideoWriter = VideoWriter(currentEventPath + "video_" +".avi", CV_FOURCC('D', 'I', 'V', 'X'), 15, frameSize, false);

    }

    /// SAVE FITS3D

    cout << " recFits3D : " << recFits3D << endl;

    Fits3D fits3d;

    if(recFits3D)
        fits3d = Fits3D(pixelFormat, frameBuffer->front().getImg().rows, frameBuffer->front().getImg().cols, (numLastFrameToSave - numFirstFrameToSave +1), fitsHeader);

    cout << ">> " << numLastFrameToSave - numFirstFrameToSave << endl;


    Mat stackEvent = Mat::zeros(frameBuffer->front().getImg().rows, frameBuffer->front().getImg().cols, CV_32FC1);

    boost::circular_buffer<Frame>::iterator it;

    for(it = frameBuffer->begin(); it != frameBuffer->end(); ++it){

        if((*it).getNumFrame() >= numFirstFrameToSave && (*it).getNumFrame() <= numLastFrameToSave){

            Mat currFrame;
            (*it).getImg().copyTo(currFrame);

            Mat temp;

            if(pixelFormat == MONO_12)
                currFrame.convertTo(temp, CV_8UC1, 255, 0);


            /// SAVE BMP

            if(recBmp){

                path p(currentEventPath + "BMP/");

                if(!fs::exists(p))
                    fs::create_directory(p);

                imwrite(currentEventPath + "BMP/frame_" + Conversion::intToString(c) + ".bmp", temp);

            }

            /// SAVE AVI

            if(recAvi){

                if(oVideoWriter.isOpened())
                    oVideoWriter << temp;

            }

            /// SAVE FITS2D

            if(recFits2D){

                string fits2DPath = currentEventPath + "fits2D/";
                vector<string> DD;

                path p(fits2DPath);

                if(!fs::exists(p))
                    fs::create_directory(p);

                if(fs::exists(p)){

                    if(pixelFormat == MONO_8){

                        Fits2D newFits(fits2DPath, fitsHeader);
                        newFits.writeFits((*it).getImg(), UC8, DD, false,"frame_" + Conversion::intToString(c));

                    }else{

                        Fits2D newFits(fits2DPath + "frame_" + Conversion::intToString(c) + "_",fitsHeader);
                        newFits.writeFits((*it).getImg(), US16, DD, false,"frame_" + Conversion::intToString(c));
                    }
                }


            }

            /// SAVE FITS3D

            if(recFits3D){

                cout << "addData" << endl;

                fits3d.addImageToFits3D((*it).getImg());

            }

            c++;

            accumulate((*it).getImg(), stackEvent);

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

        Fits2D newFits(currentEventPath + "sum",fitsHeader);

        vector<string> DD;

        if(stackReduction){

            Mat newMat ;

            float bzero     = 0.0;
            float bscale    = 1.0;

            ImgReduction::dynamicReductionByFactorDivision(stackEvent, pixelFormat, c, bzero, bscale).copyTo(newMat);

            newFits.setBzero(bzero);
            newFits.setBscale(bscale);

            newFits.writeFits(newMat, S16, DD, false,"sum");


        }else{

            // Save fits in 32 bits.
            newFits.writeFits(stackEvent, F32 , DD, false,"sum");

        }

    }

    lock.unlock();

    return true;

}

