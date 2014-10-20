/*
								Fits3D.cpp

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
 * @file    Fits3D.cpp
 * @author  Yoan Audureau
 * @version 1.0
 * @date    03/06/2014
 * @section DESCRIPTION
 *
 * The detection class contains all meteor detection methods
 */

#include "Fits3D.h"

Fits3D::Fits3D(int dimT, int dimH, int dimW, vector <Mat> frames){

    //ctor
    imgW = dimW;
    imgH = dimH;
    imgT = dimT;

    buffer = frames;

}

Fits3D::~Fits3D(){
    //dtor
}

//http://www.great08challenge.info/code/c/read_GREAT08_fits.c
bool Fits3D::writeFits3D_UC(string file){

    // Pointer to the FITS file
    fitsfile *fptr;
    int status = 0;
    long naxis = 3;
    long naxes[3] = {imgW,imgH,imgT};
    int size3d = naxes[0] * naxes[1] * naxes[2];
    long fpixel[3]={1,1,1};
    int imgSize = imgH*imgW;

    // 1D array which contains severals images : [ image1(line1, line2 ...) image2(line1, line2...)]
    unsigned char *array3d;

    // 2D array : one image per line
    unsigned char **array2d;

    array2d = (unsigned char**)calloc(imgT, sizeof(unsigned char*));

    if (array2d == NULL){
        cout << "Memory allocation error for array2d"<<endl;
        return false;
    }

    for (int i=0; i<imgT; i++){

        array2d[i] = (unsigned char*)calloc((imgW*imgH), sizeof(unsigned char));

        if (array2d[i] == NULL){

            cout << "Memory allocation error for array2d" << endl;
            return false;

        }
    }

    /// Fill 2D array with frames

    Mat currentImg;

    for(int t=0 ; t<imgT;t++){

        buffer.at(t).copyTo(currentImg);

        for (int j = 0 ; j < naxes[1] ; j++){

            unsigned char *  pt= currentImg.ptr<unsigned char>(j);

            for (int i = 0; i <naxes[0] ; i++){

                array2d[t][(imgH-1-j)*imgW+i] = (int)pt[i];

            }
        }
     }

    array3d = (unsigned char *)calloc(size3d,sizeof(unsigned char ));

    int jj;

    for (int n = 0; n < imgT; n++){

        for (int i=0; i<imgSize; i++){

              jj = n*imgSize + i;
              array3d[jj] = array2d[n][i];

        }
    }

    const char * filename;

    filename = file.c_str();

    remove(filename);

    fits_create_file(&fptr, filename, &status);

    fits_create_img(fptr, BYTE_IMG, naxis, naxes, &status);


    if (fits_write_pix(fptr, TBYTE, fpixel, size3d, array3d, &status)){

        cout << " Error writing pixel data " << endl;
        return false;
    }

    fits_close_file(fptr, &status);     // close the file

    fits_report_error(stderr, status);  // print out any error messages

    return true;

}



bool Fits3D::writeFits3D_US(string file){

    // Pointer to the FITS file
    fitsfile *fptr;
    int status = 0;
    long naxis = 3;
    long naxes[3] = {imgW,imgH,imgT};
    int size3d = naxes[0] * naxes[1] * naxes[2];
    long fpixel[3]={1,1,1};
    int imgSize = imgH*imgW;

    // 1D array which contains severals images : [ image1(line1, line2 ...) image2(line1, line2...)]
    unsigned short *array3d;

    // 2D array : one image per line
    unsigned short **array2d;

    array2d = (unsigned short**)calloc(imgT, sizeof(unsigned short*));

    if (array2d == NULL){
        cout << "Memory allocation error for array2d"<<endl;
        return false;
    }

    for (int i=0; i<imgT; i++){

        array2d[i] = (unsigned short*)calloc((imgW*imgH), sizeof(unsigned short));

        if (array2d[i] == NULL){

            cout << "Memory allocation error for array2d" << endl;
            return false;

        }
    }

    /// Fill 2D array with frames

    Mat currentImg;

    for(int t=0 ; t<imgT;t++){

        buffer.at(t).copyTo(currentImg);

        for (int j = 0 ; j < naxes[1] ; j++){

            unsigned short *  pt= currentImg.ptr<unsigned short>(j);

            for (int i = 0; i <naxes[0] ; i++){

                array2d[t][(imgH-1-j)*imgW+i] = (int)pt[i];

            }
        }
     }

    array3d = (unsigned short *)calloc(size3d,sizeof(unsigned short ));

    int jj;

    for (int n = 0; n < imgT; n++){

        for (int i=0; i<imgSize; i++){

              jj = n*imgSize + i;
              array3d[jj] = array2d[n][i];

        }
    }

    const char * filename;

    filename = file.c_str();

    remove(filename);

    if(fits_create_file(&fptr, filename, &status)){

        cout << " Failed to create fits3D " << endl;

    }

    fits_create_img(fptr, SHORT_IMG, naxis, naxes, &status);


    if (fits_write_pix(fptr, TSHORT, fpixel, size3d, array3d, &status)){

        cout << " Error writing pixel data " << endl;
        return false;
    }

    fits_close_file(fptr, &status);     // close the file

    fits_report_error(stderr, status);  // print out any error messages

    return true;

}





