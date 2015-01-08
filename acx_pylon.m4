dnl
dnl				acx_pylon.m4
dnl
dnl Figure out if the Pylon package is installed.
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
dnl*	Last modified:		17/06/2014
dnl*
dnl*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

dnl @synopsis ACX_PYLON([PYLON_LIBSDIR, PYLON_INCDIR,[ACTION-IF-FOUND
dnl                         [, ACTION-IF-NOT-FOUND]]])
dnl This macro figures out if the PYLON libraries are installed.
dnl ACTION-IF-FOUND is a list of shell commands to run if PYLON
dnl is found (HAVE_PYLON is defined first), and ACTION-IF-NOT-FOUND
dnl is a list of commands to run it if it is not found.

AC_DEFUN([ACX_PYLON], [
AC_REQUIRE([AC_CANONICAL_HOST])
acx_pylon_ok=no

dnl --------------------
dnl Search include files
dnl --------------------
def_pylon=/opt/pylon3
def_inc=$def_pylon/include
geni_inc=$def_pylon/genicam/library/CPP/include
if test x$2 = x; then
  oldCPPFLAGS=$CPPFLAGS
  CPPFLAGS="-I$def_inc -I$geni_inc"
  AC_CHECK_HEADERS([$def_inc/pylon/PylonIncludes.h],[acx_pylon_ok=yes])
  if test x$acx_pylon_ok = xyes; then
    AC_DEFINE_UNQUOTED(PYLON_H, "$def_inc/pylon/PylonIncludes.h", [PYLON header filename.])
    PYLON_CPPFLAGS="$CPPFLAGS"
  else
    PYLON_ERROR="Pylon include files not found!"
  fi
else
  def_inc=$2/include
  geni_inc=$2/genicam/library/CPP/include
  oldCPPFLAGS=$CPPFLAGS
  CPPFLAGS="-I$def_inc -I$geni_inc"
  AC_CHECK_HEADERS([$def_inc/pylon/PylonIncludes.h], [acx_pylon_ok=yes])
  if test x$acx_pylon_ok = xyes; then
    AC_DEFINE_UNQUOTED(PYLON_H, "$def_inc/pylon/PylonIncludes.h", [PYLON header filename.])
    PYLON_CPPFLAGS="$CPPFLAGS"
  else
    PYLON_ERROR="Pylon include files not found in $2!"
  fi
fi


dnl --------------------
dnl Search library files
dnl --------------------
def_lib=/opt/pylon3
genicam_lib=genicam/bin/Linux64_x64
lib=lib64
if test x$acx_pylon_ok = xyes; then
  OLIBS="$LIBS"
  LIBS=""
  if test x$1 = x; then
    oldCPPFLAGS=$CPPFLAGS
    CPPFLAGS="-L$def_lib/$lib -L$def_lib/$genicam_lib -lpylonbase -lXerces-C_gcc40_v2_7 -lGCBase_gcc40_v2_3 -lGenApi_gcc40_v2_3 -llog4cpp_gcc40_v2_3 -lLog_gcc40_v2_3 -lMathParser_gcc40_v2_3"
  
    AC_CHECK_LIB(pylonbase, main,, [acx_pylon_ok=no],
		[$CPPFLAGS])
    if test x$acx_pylon_ok = xyes; then
      PYLON_LIBPATH="$CPPFLAGS"
    else
      PYLON_ERROR="Pylon library files not found at usual locations!"
    fi
  else
dnl -------------------------
dnl Specific libdir specified
dnl -------------------------
    oldCPPFLAGS=$CPPFLAGS
    CPPFLAGS="-L$1/$lib -L$1/$genicam_lib -lpylonbase -lXerces-C_gcc40_v2_7 -lGCBase_gcc40_v2_3 -lGenApi_gcc40_v2_3 -llog4cpp_gcc40_v2_3 -lLog_gcc40_v2_3 -lMathParser_gcc40_v2_3"
    AC_CHECK_LIB(pylonbase, main,, [acx_pylon_ok=no],
		[$CPPFLAGS])
    if test x$acx_pylon_ok = xyes; then
      PYLON_LIBPATH="$CPPFLAGS"
    else
      PYLON_ERROR="Pylon library files not found in $1!"
    fi
  fi
fi

AC_SUBST(PYLON_LIBPATH)
AC_SUBST(PYLON_CPPFLAGS)


dnl -------------------------------------------------------------------------
dnl Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND
dnl -------------------------------------------------------------------------
if test x"$acx_pylon_ok" = xyes; then
  AC_DEFINE(HAVE_PYLON,1,
	[Define if you have the Pylon libraries and header files.])
  PYLON_LIBS="$PYLON_LIBPATH -lpylonbase -lXerces-C_gcc40_v2_7 -lGCBase_gcc40_v2_3 -lGenApi_gcc40_v2_3 -llog4cpp_gcc40_v2_3 -lLog_gcc40_v2_3 -lMathParser_gcc40_v2_3"
  LIBS="$OLIBS"
  AC_SUBST(PYLON_LIBS)
  $3
else
  AC_SUBST(PYLON_ERROR)
  $4
fi

])dnl ACX_PYLON
