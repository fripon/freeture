#
#				Makefile.am
#
# src Makefile.am. Process this file with automake to generate a Makefile
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#*
#*	This file part of:	FreeTure
#*
#*	Copyright:		(C) 2014-2015 Yoan Audureau, Chiara Marmo
#*                              FRIPON-GEOPS-UPSUD-CNRS
#*
#*	License:		GNU General Public License
#*
#*	FreeTure is free software: you can redistribute it and/or modify
#*	it under the terms of the GNU General Public License as published by
#*	the Free Software Foundation, either version 3 of the License, or
#*	(at your option) any later version.
#*	FreeTure is distributed in the hope that it will be useful,
#*	but WITHOUT ANY WARRANTY; without even the implied warranty of
#*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#*	GNU General Public License for more details.
#*	You should have received a copy of the GNU General Public License
#*	along with FreeTure. If not, see <http://www.gnu.org/licenses/>.
#*
#*	Last modified:		20/01/2015
#*
#*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

AUTOMAKE_OPTIONS = foreign no-dependencies subdir-objects
bin_PROGRAMS = FreeTure
SRC_DIR = ../src

FreeTure_SOURCES = $(SRC_DIR)/EImgBitDepth.h $(SRC_DIR)/ECamType.h $(SRC_DIR)/EParser.h $(SRC_DIR)/EParser.cpp \
			$(SRC_DIR)/EDetMeth.h $(SRC_DIR)/Base64.h $(SRC_DIR)/Camera.h $(SRC_DIR)/AcqThread.h $(SRC_DIR)/AcqThread.cpp \
			$(SRC_DIR)/CameraVideo.h $(SRC_DIR)/CameraGigeAravis.h $(SRC_DIR)/CameraGigeAravis.cpp \
			$(SRC_DIR)/CameraFrames.h $(SRC_DIR)/CameraGigePylon.h $(SRC_DIR)/CameraGigePylon.cpp $(SRC_DIR)/CameraV4l2.cpp $(SRC_DIR)/CameraV4l2.h \
			$(SRC_DIR)/SMTPClient.h ../src/CameraGigeTis.h ../src/CameraGigeTis.cpp ../src/Circle.h \
			$(SRC_DIR)/Configuration.h $(SRC_DIR)/Conversion.h $(SRC_DIR)/DetThread.h $(SRC_DIR)/Detection.h \
			$(SRC_DIR)/Fits.h $(SRC_DIR)/Fits2D.h $(SRC_DIR)/Fits3D.h $(SRC_DIR)/Frame.h $(SRC_DIR)/GlobalEvent.h $(SRC_DIR)/Histogram.h $(SRC_DIR)/DetectionTemplate.h \
			$(SRC_DIR)/LocalEvent.h $(SRC_DIR)/EStackMeth.h $(SRC_DIR)/DetectionTemplate.cpp $(SRC_DIR)/DetectionTemporal.h $(SRC_DIR)/DetectionTemporal.cpp \
			$(SRC_DIR)/TimeDate.h $(SRC_DIR)/Device.h $(SRC_DIR)/Device.cpp $(SRC_DIR)/Stack.h $(SRC_DIR)/Stack.cpp $(SRC_DIR)/Ephemeris.cpp \
			$(SRC_DIR)/SaveImg.h $(SRC_DIR)/ECamBitDepth.h $(SRC_DIR)/StackThread.h $(SRC_DIR)/StackThread.cpp \
			$(SRC_DIR)/Base64.cpp $(SRC_DIR)/ESmtpSecurity.h $(SRC_DIR)/OpenSSL.h $(SRC_DIR)/OpenSSL.cpp $(SRC_DIR)/Socket.h \
			$(SRC_DIR)/CameraFrames.cpp $(SRC_DIR)/SMTPClient.cpp $(SRC_DIR)/HistogramGray.h $(SRC_DIR)/HistogramGray.cpp \
			$(SRC_DIR)/CameraVideo.cpp $(SRC_DIR)/Configuration.cpp $(SRC_DIR)/Conversion.cpp $(SRC_DIR)/Ephemeris.h \
			$(SRC_DIR)/DetThread.cpp $(SRC_DIR)/Fits.cpp $(SRC_DIR)/Fits2D.cpp $(SRC_DIR)/Fits3D.cpp $(SRC_DIR)/Logger.h \
			$(SRC_DIR)/Frame.cpp $(SRC_DIR)/GlobalEvent.cpp $(SRC_DIR)/Histogram.cpp $(SRC_DIR)/AcqSchedule.h $(SRC_DIR)/AcqSchedule.cpp \
			$(SRC_DIR)/LocalEvent.cpp $(SRC_DIR)/main.cpp $(SRC_DIR)/HistogramRGB.h $(SRC_DIR)/HistogramRGB.cpp $(SRC_DIR)/EImgFormat.h \
			$(SRC_DIR)/TimeDate.cpp $(SRC_DIR)/ExposureControl.cpp $(SRC_DIR)/ExposureControl.h $(SRC_DIR)/ETimeMode.h \
			$(SRC_DIR)/SaveImg.cpp $(SRC_DIR)/ELogSeverityLevel.h $(SRC_DIR)/ImgProcessing.cpp $(SRC_DIR)/ImgProcessing.h

dist_pkgdata_DATA	= ../share/configuration.cfg
EXTRA_DIST		= README NEWS INSTALL acx_boost.m4 acx_cfitsio.m4 \
			acx_opencv.m4 acx_pylon.m4 acx_urbi_resolve_dir.m4 \
			bootstrap.sh THANKS


