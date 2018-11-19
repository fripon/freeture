/*
                                    SaveImg.h

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
* \file    SaveImg.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/06/2014
*/

#pragma once

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

class SaveImg{

    public:


        /**
        * Save JPEG.
        *
        * @param img Opencv mat image to save.
        * @param name Path and name of the file to save.
        * @return Success status to save image.
        */
        static bool saveJPEG(Mat img, string name);


        /**
        * Save BMP.
        *
        * @param img Opencv mat image to save.
        * @param name Path and name of the file to save.
        * @return Success status to save image.
        */
        static bool saveBMP(Mat img, string name);


        /**
        * Save PNG.
        *
        * @param img Opencv mat image to save.
        * @param name Path and name of the file to save.
        * @return Success status to save image.
        */
        static bool savePNG(Mat img, string name);

};
