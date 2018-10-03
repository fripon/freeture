/*
                                    Mask.h

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
*   Last modified:      26/11/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    Mask.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    26/11/2014
*/

#pragma once
#include <boost/circular_buffer.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include "ImgProcessing.h"
#include "SaveImg.h"
#include "Conversion.h"
#include "ECamPixFmt.h"
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;

class Mask {

    public :

        Mat mCurrentMask;

    private :

        Mat mOriginalMask;
        int mUpdateInterval;
        bool mUpdateMask;
        bool mMaskToCreate;
        string refDate;
        bool updateStatus;
        int saturatedValue;
        boost::circular_buffer<Mat> satMap;

    public :

        Mask(int timeInterval, bool customMask, string customMaskPath, bool downsampleMask, CamPixFmt format, bool updateMask);

        bool applyMask(Mat &currFrame);

        void resetMask();

};


