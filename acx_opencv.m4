dnl
dnl				acx_opencv.m4
dnl
dnl Figure out if the OpenCV package is installed.
dnl
dnl%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
dnl*
dnl*	This file part of:	FreeTure
dnl*
dnl*	Copyright:		(C) 2014-2015 Yoan Audureau, Chiara Marmo
dnl*                            FRIPON-GEOPS-UPSUD-CNRS
dnl*
dnl*	License:		GNU General Public License
dnl*
dnl*	FriponCapture is free software: you can redistribute it and/or modify
dnl*	it under the terms of the GNU General Public License as published by
dnl*	the Free Software Foundation, either version 3 of the License, or
dnl*	(at your option) any later version.
dnl*	FriponCapture is distributed in the hope that it will be useful,
dnl*	but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl*	GNU General Public License for more details.
dnl*	You should have received a copy of the GNU General Public License
dnl*	along with FriponCapture. If not, see <http://www.gnu.org/licenses/>.
dnl*
dnl*	Last modified:		13/06/2014
dnl*
dnl*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

dnl @synopsis ACX_OPENCV([OPENCV_LIBSDIR, OPENCV_INCDIR, [ACTION-IF-FOUND
dnl                         [, ACTION-IF-NOT-FOUND]]])
dnl This macro figures out if the OPENCV libraries are installed.
dnl ACTION-IF-FOUND is a list of shell commands to run if OPENCV
dnl is found (HAVE_OPENCV is defined first), and ACTION-IF-NOT-FOUND
dnl is a list of commands to run it if it is not found.

AC_DEFUN([ACX_OPENCV], [
AC_REQUIRE([AC_CANONICAL_HOST])
acx_opencv_ok=no

dnl --------------------
dnl Search include files
dnl --------------------
if test x$2 = x; then
  if test x$1 = x; then
    AC_CHECK_HEADERS([opencv2/opencv.hpp],[acx_opencv_ok=yes])
    if test x$acx_opencv_ok = xyes; then
      AC_DEFINE(OPENCV_H, "opencv2/opencv.hpp", [OPENCV header filename.])
    else
      OPENCV_ERROR="OpenCV include files not found!"
    fi
  else
    AC_CHECK_HEADERS([$1/include/opencv2/opencv.hpp], [acx_opencv_ok=yes])
    if test x$acx_opencv_ok = xyes; then
      AC_DEFINE_UNQUOTED(OPENCV_H, "$1/include/opencv2/opencv.hpp", [OPENCV header filename.])
    else
      OPENCV_ERROR="OpenCV include files not found!"
    fi
  fi
else
  AC_CHECK_HEADERS([$2/opencv2/opencv.hpp], [acx_opencv_ok=yes])
  if test x$acx_opencv_ok = xyes; then
    AC_DEFINE_UNQUOTED(OPENCV_H, "$2/opencv2/opencv.hpp", [OPENCV header filename.])
  else
    OPENCV_ERROR="OpenCV include files not found in $2!"
  fi
fi

dnl --------------------
dnl Search library files
dnl --------------------
if test x$acx_opencv_ok = xyes; then
  OLIBS="$LIBS"
  LIBS=""
  if test x$1 = x; then
    AC_CHECK_LIB(opencv_calib3d, main,, [acx_opencv_ok=no],
		[-lopencv_calib3d])
    AC_CHECK_LIB(opencv_contrib, main,, [acx_opencv_ok=no],
		[-lopencv_contrib])
    AC_CHECK_LIB(opencv_core, main,, [acx_opencv_ok=no],
		[-lopencv_core])
    AC_CHECK_LIB(opencv_features2d, main,, [acx_opencv_ok=no],
		[-lopencv_features2d])
    AC_CHECK_LIB(opencv_flann, main,, [acx_opencv_ok=no],
		[-lopencv_flann])
    AC_CHECK_LIB(opencv_highgui, main,, [acx_opencv_ok=no],
		[-lopencv_highgui])
    AC_CHECK_LIB(opencv_imgproc, main,, [acx_opencv_ok=no],
		[-lopencv_imgproc])
    AC_CHECK_LIB(opencv_legacy, main,, [acx_opencv_ok=no],
		[-lopencv_legacy])
    AC_CHECK_LIB(opencv_ml, main,, [acx_opencv_ok=no],
		[-lopencv_ml])
    AC_CHECK_LIB(opencv_objdetect, main,, [acx_opencv_ok=no],
		[-lopencv_objdetect])
    AC_CHECK_LIB(opencv_video, main,, [acx_opencv_ok=no],
		[-lopencv_video])
    if test x$acx_opencv_ok = xyes; then
      OPENCV_LIBPATH=""
    else
      OPENCV_ERROR="OpenCV library files not found at usual locations!"
    fi
  else
dnl -------------------------
dnl Specific libdir specified
dnl -------------------------
    AC_CHECK_LIB(opencv_calib3d, main,, [acx_opencv_ok=no],
		[-L$1 -lopencv_calib3d])
    AC_CHECK_LIB(opencv_contrib, main,, [acx_opencv_ok=no],
		[-L$1 -lopencv_contrib])
    AC_CHECK_LIB(opencv_core, main,, [acx_opencv_ok=no],
		[-L$1 -lopencv_core])
    AC_CHECK_LIB(opencv_features2d, main,, [acx_opencv_ok=no],
		[-L$1 -lopencv_features2d])
    AC_CHECK_LIB(opencv_flann, main,, [acx_opencv_ok=no],
		[-L$1 -lopencv_flann])
    AC_CHECK_LIB(opencv_highgui, main,, [acx_opencv_ok=no],
		[-L$1 -lopencv_highgui])
    AC_CHECK_LIB(opencv_imgproc, main,, [acx_opencv_ok=no],
		[-L$1 -lopencv_imgproc])
    AC_CHECK_LIB(opencv_legacy, main,, [acx_opencv_ok=no],
		[-L$1 -lopencv_legacy])
    AC_CHECK_LIB(opencv_ml, main,, [acx_opencv_ok=no],
		[-L$1 -lopencv_ml])
    AC_CHECK_LIB(opencv_objdetect, main,, [acx_opencv_ok=no],
		[-L$1 -lopencv_objdetect])
    AC_CHECK_LIB(opencv_video, main,, [acx_opencv_ok=no],
		[-L$1 -lopencv_video])
    if test x$acx_opencv_ok = xyes; then
      OPENCV_LIBPATH="-L$1"
    else
      OPENCV_ERROR="OpenCV library files not found in $1!"
    fi
  fi
fi

AC_SUBST(OPENCV_LIBPATH)
AC_SUBST(OPENCV_CPPFLAGS)

dnl -------------------------------------------------------------------------
dnl Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND
dnl -------------------------------------------------------------------------
if test x"$acx_opencv_ok" = xyes; then
  AC_DEFINE(HAVE_OPENCV,1,
	[Define if you have the OpenCV libraries and header files.])
  OPENCV_LIBS="$OPENCV_LIBPATH -lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_objdetect -lopencv_video"
  LIBS="$OLIBS"
  AC_SUBST(OPENCV_LIBS)
  $3
else
  AC_SUBST(OPENCV_ERROR)
  $4
fi

])dnl ACX_OPENCV

