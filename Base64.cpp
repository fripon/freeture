/*
                                Base64.cpp

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
* \file    Base64.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    26/11/2014
* \brief   Handle Base64 encryption.
*/

#include "Base64.h"

string Base64::encodeBase64(string data){

    stringstream os;

    typedef boost::archive::iterators::base64_from_binary<                      // Convert binary values to base64 characters.
                boost::archive::iterators::transform_width<const char *, 6, 8>  // Retrieve 6 bit integers from a sequence of 8 bit bytes.
            >base64_text;                                                       // Compose all the above operations in to a new iterator.

    copy(
        base64_text(data.c_str()),
        base64_text(data.c_str() + data.size()),
        boost::archive::iterators::ostream_iterator<char>(os)
    );

    return os.str();

}
