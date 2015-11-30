/*
                            PixFmtConv.cpp

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
* \file    PixFmtConv.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    26/11/2015
*/

#include "PixFmtConv.h"


// https://github.com/gjasny/v4l-utils/blob/master/lib/libv4lconvert/rgbyuv.c
void PixFmtConv::UYVY_to_BGR24(const unsigned char *src, unsigned char *dest, int width, int height, int stride) {

    int j;

    while (--height >= 0) {
        for (j = 0; j + 1 < width; j += 2) {
            int u = src[0];
            int v = src[2];
            int u1 = (((u - 128) << 7) +  (u - 128)) >> 6;
            int rg = (((u - 128) << 1) +  (u - 128) +
                    ((v - 128) << 2) + ((v - 128) << 1)) >> 3;
            int v1 = (((v - 128) << 1) +  (v - 128)) >> 1;

            *dest++ = CLIP(src[1] + u1);
            *dest++ = CLIP(src[1] - rg);
            *dest++ = CLIP(src[1] + v1);

            *dest++ = CLIP(src[3] + u1);
            *dest++ = CLIP(src[3] - rg);
            *dest++ = CLIP(src[3] + v1);
            src += 4;
        }
        src += stride - width * 2;
    }

}

void PixFmtConv::YUYV_to_BGR24(const unsigned char *src, unsigned char *dest, int width, int height, int stride) {

    int j;

    while (--height >= 0) {
        for (j = 0; j + 1 < width; j += 2) {
            int u = src[1];
            int v = src[3];
            int u1 = (((u - 128) << 7) +  (u - 128)) >> 6;
            int rg = (((u - 128) << 1) +  (u - 128) +
                    ((v - 128) << 2) + ((v - 128) << 1)) >> 3;
            int v1 = (((v - 128) << 1) +  (v - 128)) >> 1;

            *dest++ = CLIP(src[0] + u1);
            *dest++ = CLIP(src[0] - rg);
            *dest++ = CLIP(src[0] + v1);

            *dest++ = CLIP(src[2] + u1);
            *dest++ = CLIP(src[2] - rg);
            *dest++ = CLIP(src[2] + v1);
            src += 4;
        }
        src += stride - width * 2;
    }
}

void PixFmtConv::RGB565_to_BGR24(const unsigned char *src, unsigned char *dest, int width, int height) {

    int j;
    while (--height >= 0) {
        for (j = 0; j < width; j++) {
            unsigned short tmp = *(unsigned short *)src;

            /* Original format: rrrrrggg gggbbbbb */
            *dest++ = 0xf8 & (tmp << 3);
            *dest++ = 0xfc & (tmp >> 3);
            *dest++ = 0xf8 & (tmp >> 8);

            src += 2;
        }
    }
}
