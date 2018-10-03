/*
                            HistogramRGB.h

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
* \file    HistogramRGB.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
* \brief   Create/Analyse histogram of a rgb image.
*/

#pragma once

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include "Histogram.h"

using namespace std;
using namespace cv;

class HistogramRGB : public Histogram {

    /**
    * Clear bins.
    *
    */
    void clear() {

        bins.setTo(Scalar( 0.f ));

    };

    public :

        /**
        * Constructor.
        *
        */
        HistogramRGB();

        /**
        * Calculate number of pixels in each bins.
        *
        * @param image Opencv mat image to analysis.
        * @return Success status.
        */
        int calculate(Mat& image);

        /**
        * Normalize bins.
        *
        */
        void normalize(void);

        /**
        * Build histogram image.
        *
        * @return Histogram.
        */
        Mat render(void);
};
