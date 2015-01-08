dnl
dnl				acx_cfitsio.m4
dnl
dnl Figure out if the CFITSIO package is installed.
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

dnl @synopsis ACX_CFITSIO([CFITSIO_LIBSDIR,CFITSIO_INCDIR,[ACTION-IF-FOUND
dnl                         [, ACTION-IF-NOT-FOUND]]])
dnl This macro figures out if the CFITSIO libraries are installed.
dnl ACTION-IF-FOUND is a list of shell commands to run if CFITSIO
dnl is found (HAVE_CFITSIO is defined first), and ACTION-IF-NOT-FOUND
dnl is a list of commands to run it if it is not found.

AC_DEFUN([ACX_CFITSIO], [
AC_REQUIRE([AC_CANONICAL_HOST])
acx_cfitsio_ok=no

dnl --------------------
dnl Search include files
dnl --------------------
if test x$2 = x; then
  if test x$1 = x; then
dnl Testing standard locations for fitsio.h in different distributions
dnl Debian
    AC_CHECK_HEADERS([fitsio.h],[acx_cfitsio_ok=yes])
    if test x$acx_cfitsio_ok = xyes; then
      AC_DEFINE(CFITSIO_H, "fitsio.h", [CFITSIO header filename.])
    else
dnl Fedora
      AC_CHECK_HEADERS([cfitsio/fitsio.h],[acx_cfitsio_ok=yes])
      if test x$acx_cfitsio_ok = xyes; then
        AC_DEFINE(CFITSIO_H, "cfitsio/fitsio.h", [CFITSIO header filename.])
      else
        CFITSIO_ERROR="CFitsIO include files not found!"
      fi
    fi
  else
    AC_CHECK_HEADERS([$1/include/fitsio.h], [acx_cfitsio_ok=yes])
    if test x$acx_cfitsio_ok = xyes; then
      AC_DEFINE_UNQUOTED(CFITSIO_H, "$1/include/fitsio.h",
      		[CFITSIO header filename.])
    else
      CFITSIO_ERROR="CFitsIO include files not found!"
    fi
  fi
else
  AC_CHECK_HEADERS([$2/fitsio.h], [acx_cfitsio_ok=yes])
  if test x$acx_cfitsio_ok = xyes; then
    AC_DEFINE_UNQUOTED(CFITSIO_H, "$2/fitsio.h", [CFITSIO header filename.])
  else
    CFITSIO_ERROR="CFitsIO include files not found in $2!"
  fi
fi

dnl --------------------
dnl Search library files
dnl --------------------
if test x$acx_cfitsio_ok = xyes; then
  OLIBS="$LIBS"
  LIBS=""
  if test x$1 = x; then
    AC_CHECK_LIB(cfitsio, ffopen,, [acx_cfitsio_ok=no],
		[-lcfitsio])
    if test x$acx_cfitsio_ok = xyes; then
      CFITSIO_LIBPATH=""
    else
      CFITSIO_ERROR="CFITSIO library files not found at usual locations!"
    fi
  else
dnl -------------------------
dnl Specific libdir specified
dnl -------------------------
    AC_CHECK_LIB(cfitsio, ffopen,, [acx_cfitsio_ok=no],
		[-L$1 -lcfitsio])
    if test x$acx_cfitsio_ok = xyes; then
      CFITSIO_LIBPATH="-L$1"
    else
      CFITSIO_ERROR="CFITSIO library files not found in $1!"
    fi
  fi
fi

AC_SUBST(CFITSIO_LIBPATH)
AC_SUBST(CFITSIO_CFLAGS)

dnl -------------------------------------------------------------------------
dnl Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND
dnl -------------------------------------------------------------------------
if test x"$acx_cfitsio_ok" = xyes; then
  AC_DEFINE(HAVE_CFITSIO,1,
	[Define if you have the CFitsIO libraries and header files.])
  CFITSIO_LIBS="$CFITSIO_LIBPATH -lcfitsio"
  LIBS="$OLIBS"
  AC_SUBST(CFITSIO_LIBS)
  $3
else
  AC_SUBST(CFITSIO_ERROR)
  $4
fi

])dnl ACX_CFITSIO
