/*
                                ECamType.h

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
*   Last modified:      01/12/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#pragma once

/**
 * \brief Possible inputs supported by FreeTure.
 */

enum CamType{

    DMK_GIGE,       /*!< Under Linux DMK GigE cameras use Aravis library.*/
    BASLER_GIGE,    /*!< Support of Basler cameras by using Aravis Library or Pylon.*/
    TYTEA_USB,
    VIDEO,          /*!< Used to support a simple .avi in input of FreeTure.*/
    FRAMES,          /*!< Used to send in input of FreeTure a set of single images (Fits2D, jpeg, bmp).*/
    OTHERS

};


