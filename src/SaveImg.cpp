/*
								SaveImg.cpp

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

#include "SaveImg.h"

SaveImg::SaveImg(){
    //ctor
}

void SaveImg::saveJPEG(Mat img, string name){

	//vector that stores the compression parameters of the image
	vector<int> compression_params;

	//specify the compression technique
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);

	//specify the compression quality
    compression_params.push_back(98);

	//write the image to file
	bool res = imwrite(name + ".jpg", img, compression_params);

	if(res)

		cout << "Save jpg worked." << endl;

	else

		cout << "Save jpg failed." << endl;

}

void SaveImg::saveBMP(Mat img, string name){

	bool res = imwrite(name+".bmp", img);

	if(res)

		cout << "> Save bmp worked." << endl;

	else

		cout << "> Save bmp failed." << endl;

}

bool SaveImg::savePNG(Mat img, string name){

    vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(98);
    bool res = imwrite(name+".png", img, compression_params);

    return res;
}
