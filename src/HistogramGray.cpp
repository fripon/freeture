/*
                            HistogramGray.cpp

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
* \file    HistogramGray.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Create/Analyse histogram of a grey image.
*/

#include "HistogramGray.h"

HistogramGray::HistogramGray(){

		bins = Mat(1, 256, CV_32F, Scalar(0.f));

}

int HistogramGray::calculate(Mat& image){

    if( image.channels() != 1 || image.type() != CV_8U ) { return 1; }
    this->clear();
    Mat_< uchar >::iterator it = image.begin< uchar >();
    Mat_< uchar >::iterator itend = image.end< uchar >();

    for( ; it != itend; ++it ) {
        bins.at< float >( 0, *it ) += 1.f;
    }

    return 0;

}

void HistogramGray::normalize(void){

    Mat_< float >::iterator it = bins.begin< float >();
    Mat_< float >::iterator itend = bins.end< float >();
    float max = 1.f;

    for( ; it != itend; ++it ) {
        if( *it > max ) max = *it;
    }

    bins /= max;
}

Mat HistogramGray::render(void){

    Mat result( 100, 256, CV_8U, Scalar( 0 ) );
    Point start( 0, 0 ), end( 0, 0 );

    for( int i = 0; i < 256; i++ ) {
        start.x = end.x = i;
        end.y = cvRound( 100.f * bins.at< float >( i ) );
        line( result, start, end, Scalar( 255 ) );
    }

    flip( result, result, 0 );

    return result;
}

Mat HistogramGray::renderHistogramOnImage(Mat image){

    Mat h = render();

    h.copyTo(image(Rect(0, image.rows - h.rows,h.cols,h.rows)));

    return image;

}
