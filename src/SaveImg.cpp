/*
                                SaveImg.cpp

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
* \file    SaveImg.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
*/

#include "SaveImg.h"

bool SaveImg::saveJPEG(Mat img, string name){

    // Vector that stores the compression parameters of the image.
    vector<int> compression_params;

    // Specify the compression technique.
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);

    // Specify the compression quality.
    compression_params.push_back(98);

    // Write the image to file.
    return imwrite(name + ".jpg", img, compression_params);

}

bool SaveImg::saveBMP(Mat img, string name){

    return imwrite(name+".bmp", img);

}

bool SaveImg::savePNG(Mat img, string name){

    vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(98);
    return imwrite(name+".png", img, compression_params);

}
/*
void SaveImg::saveMat(Mat& m, string filename){

    namespace io = boost::iostreams;

    ofstream ofs(filename.c_str(),ios::out | ios::binary);

    {

        io::filtering_streambuf<io::output> out;
       // out.push(io::zlib_compressor(io::zlib::best_speed));

        out.push(ofs);

        boost::archive::binary_oarchive oa(out);

        oa << m;

    }

    //ofs.close;

}

void SaveImg::loadMat(Mat& m, string filename){

    namespace io = boost::iostreams;

    std::ifstream ifs(filename.c_str(), ios::in | ios::binary);

    {

        io::filtering_streambuf<io::input> in;

        in.push(io::zlib_decompressor());

        in.push(ifs);

        boost::archive::binary_iarchive ia(in);

        ia >> m;

    }
}
*/
