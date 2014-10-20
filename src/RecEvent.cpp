/*
								RecEvent.cpp

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
 * @file    RecEvent.cpp
 * @author  Yoan Audureau
 * @version 1.0
 * @date    03/06/2014
 * @section DESCRIPTION
 *
 * The detection class contains all meteor detection methods
 */


#include "RecEvent.h"

RecEvent::RecEvent(){


    //ctor
}

RecEvent::~RecEvent(){

cout << "destructor recEvent"<<endl;

    //dtor
}

void RecEvent::setMapEvent(Mat mapE){

    mapE.copyTo(mapEvent);

}

Mat RecEvent::getMapEvent(){

    return mapEvent;

}

void RecEvent::setListMetPos(vector<Point> l){

    meteorPos = l;

}

void RecEvent::setBuffer(vector<Mat> b){

    buffer = b;

}

void RecEvent::setPrevFrames(vector<Frame> prev){

    previousFrames = prev;

}

void RecEvent::setFramesDisk(vector<int> f){

    diskFrames = f;

}

void RecEvent::setFrameBufferLocation(string path){

    bufferPath = path;

}

void RecEvent::setPositionInBuffer(vector<int> l){

    posInBuffer = l;

}

vector<int> RecEvent::getPositionInBuffer(){

    return posInBuffer;

}

vector<Point> RecEvent::getListMetPos(){

    return meteorPos;

}

void RecEvent::setBufferFileName(vector<Frame> l){

     bufferFileName = l;

}

void RecEvent::setPathOfFrames(string newPath){

     pathOfFrames = newPath;

}

string RecEvent::getPathOfFrames(){

     return pathOfFrames;

}

string RecEvent::getPath(){

     return path;

}

void RecEvent::setPath(string newPath){

     path = newPath;

}

vector<Frame>  RecEvent::getBufferFileName(){

    return bufferFileName;

}

bool RecEvent::copyFromRecEvent(RecEvent *ev){

    meteorPos       = ev->getListMetPos();
    posInBuffer     = ev->getPositionInBuffer();
    path            = ev->getPath();
    mapEvent        = ev->getMapEvent();
    dateEv          = ev->getDateEvent();
    buffer          = ev->getBuffer();

}

vector<Frame> RecEvent::getPrevFrames(){

    return previousFrames;

}

vector<int> RecEvent::getFramesDisk(){

    return diskFrames;

}

vector<Mat> RecEvent::getBuffer(){

    return buffer;

}

string RecEvent::getFrameBufferLocation(){

    return bufferPath;

}

void RecEvent::setDateEvent(vector<string> date){

    dateEv =  date;

}

vector<string> RecEvent::getDateEvent(){

    return dateEv;

}


/*
  int numEvent = 0;

    //Get the event's date
    vector<int> eventDate = gEvent.getDate();

    //Directory of day event : ../ORSAY_DD-MM-AA/
    string root = recPath + stationName + Conversion::intToString(eventDate.at(2)) + "-" + Conversion::intToString(eventDate.at(1)) + "-" + Conversion::intToString(eventDate.at(0)) +"/";

    //Directory of hour's event: ../HH_UT/
    string sub1 = Conversion::intToString(eventDate.at(3)) + "_UT/";

    //Directory of an event: ../ev_ORSAY_DDMMAA_HHMMSS_num_UT/
    string sub2 = "ev_"+ stationName + "_" + Conversion::intToString(eventDate.at(2))
                                           + Conversion::intToString(eventDate.at(1))
                                           + Conversion::intToString(eventDate.at(0)) + "_"
                                           + Conversion::intToString(eventDate.at(3))
                                           + Conversion::intToString(eventDate.at(4))
                                           + Conversion::intToString(eventDate.at(5)) + "_"
                                           + Conversion::intToString(0) + "_UT/";

    namespace fs = boost::filesystem;

    path p1(root);

    string path2 = root + sub1;
    path p2(path2);

    string path3 = root + sub1 + sub2;
    path p3(path3);

    if(fs::exists(p1)){

        std::cerr << "Destination directory " << p1.string() << " already exists." << '\n';
        //return false;

        if(fs::exists(p2)){

            std::cerr << "Destination directory " << p2.string() << " already exists." << '\n';


            if(fs::exists(p3)){

                std::cerr << "Destination directory " << p3.string() << " already exists." << '\n';

                //copy of the frame buffer
                string path4 = path3 + "buffer/";
                path p4(path4);

                if(!fs::create_directory(p4)){
                    std::cerr << "Unable to create destination directory" << p3.string() << '\n';
                    //return false;

                }else{

                    /// SAVE

                    boost::unique_lock<boost::mutex> lock_eventToRec(mutex_listEventToRecord);
                    lock_eventToRec.lock();

                    //copy of the frame buffer
                    path sourceBufferDisk(bufferDiskPath);
                    ManageFiles::copyDirectory( sourceBufferDisk ,p4 );






                    lock_eventToRec.unlock();






                    //copie de la liste des noms de frames du buffer sur le disque


                    rec->setBufferFileName(imageOfBufferOnDisk);

                    lock_eventToRec.unlock();

                    //Ajout du chemin des frames du buffer
                    rec->setPathOfFrames(path4);

                    //Ajout du chemin où les images et vidéos seront enregistrées
                    rec->setPath(path3);

                    //copie du buffer dans le chemin path4

                    path ps("./framebuffer/");
                    path pd(p4);

                    ManageFiles::copyDirectory( ps, pd);

                    //Ajout du recEvent à la liste des évènement à enregistrer
                    boost::unique_lock<boost::mutex> lock(mutex_imageOfBufferOnDisk);
                    lock.lock();

                    listEventToRecord.push_back(*rec);

                    lock.unlock();

                    //Envoie d'une notification au thread d'enregistrement
                    //condNewElemOn_ListEventToRec->notify_all();
                    returnStatus = true;

                    //delete rec;

                    //cout << "save"<<endl;
                   // saveBMP(metImg, path3+"met_"+Conversion::intToString(moveObjCpt));


                }

            }else{

                // Create the destination directory
                if(!fs::create_directory(p3)){
                    std::cerr << "Unable to create destination directory" << p3.string() << '\n';
                    //return false;


                }else{

                     /// SAVE
                    cout << "save"<<endl;
                    //saveBMP(metImg, path3+"met_"+Conversion::intToString(moveObjCpt));


                }

            }


        }else{

            // Create the destination directory
            if(!fs::create_directory(p2)){
                std::cerr << "Unable to create destination directory" << p2.string() << '\n';
                //return false;

            }else{


                // Create the destination directory
                if(!fs::create_directory(p3)){
                    std::cerr << "Unable to create destination directory" << p3.string() << '\n';
                    //return false;

                }else{

                    /// SAVE
                    cout << "save"<<endl;
                    //saveBMP(metImg, path3+"met_"+Conversion::intToString(moveObjCpt));

                }
            }
        }

    }else{

        // Create the destination directory
        if(!fs::create_directory(p1)){

            std::cerr << "Unable to create destination directory" << p1.string() << '\n';
            //return false;


        }else{

            if(!fs::create_directory(p2)){
                std::cerr << "Unable to create destination directory" << p2.string() << '\n';
                //return false;

            }else{


                // Create the destination directory
                if(!fs::create_directory(p3)){
                    std::cerr << "Unable to create destination directory" << p3.string() << '\n';
                    //return false;


                }else{

                    /// SAVE
                    cout << "save"<<endl;
                   // saveBMP(metImg, path3+"met_"+Conversion::intToString(moveObjCpt));

                }
            }
        }
    }


*/
