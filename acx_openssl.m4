dnl
dnl				acx_openssl.m4
dnl
dnl Figure out if the openssl package is installed.
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
dnl*	Last modified:		13/06/2015
dnl*
dnl*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

dnl @synopsis ACX_OPENSSL([OPENSSL_LIBSDIR, OPENSSL_INCDIR,[ACTION-IF-FOUND
dnl                         [, ACTION-IF-NOT-FOUND]]])
dnl This macro figures out if the OPENSSL libraries are installed.
dnl ACTION-IF-FOUND is a list of shell commands to run if OPENSSL
dnl is found (HAVE_OPENSSL is defined first), and ACTION-IF-NOT-FOUND
dnl is a list of commands to run it if it is not found.

AC_DEFUN([ACX_OPENSSL], [
AC_REQUIRE([AC_CANONICAL_HOST])
acx_openssl_ok=no

dnl --------------------
dnl Search include files
dnl --------------------
if test x$2 = x; then
  if test x$1 = x; then
    AC_CHECK_HEADERS([openssl/ssl.h],[acx_openssl_ok=yes])
    if test x$acx_openssl_ok = xyes; then
      AC_DEFINE(OPENSSL_H, "openssl/ssl.h", [OPENSSL header filename.])
    else
      OPENSSL_ERROR="OpenSSL include files not found!"
    fi
  else
    AC_CHECK_HEADERS([$1/include/openssl/ssl.h], [acx_openssl_ok=yes])
    if test x$acx_openssl_ok = xyes; then
      AC_DEFINE_UNQUOTED(OPENSSL_H, "$1/include/openssl/ssl.h", [OPENSSL header filename.])
    else
      OPENSSL_ERROR="OpensSSL include files not found!"
    fi
  fi
else
  AC_CHECK_HEADERS([$2/openssl/ssl.h], [acx_openssl_ok=yes])
  if test x$acx_openssl_ok = xyes; then
    AC_DEFINE_UNQUOTED(OPENSSL_H, "$2/openssl/ssl.h", [OPENSSL header filename.])
  else
    OPENSSL_ERROR="OpenSSL include files not found in $2!"
  fi
fi


dnl --------------------
dnl Search library files
dnl --------------------
if test x$acx_OPENSSL_ok = xyes; then
  OLIBS="$LIBS"
  LIBS=""
  if test x$1 = x; then
    AC_CHECK_LIB(ssl, main,, [acx_openssl_ok=no],
		[-lssl])
    if test x$acx_openssl_ok = xyes; then
      OPENSSL_LIBPATH=""
    else
      OPENSSL_ERROR="OpenSSL library files not found at usual locations!"
    fi
  else
dnl -------------------------
dnl Specific libdir specified
dnl -------------------------
    AC_CHECK_LIB(ssl, main,, [acx_openssl_ok=no],
		[-L$1 -lssl])
    if test x$acx_openssl_ok = xyes; then
      OPENSSL_LIBPATH="-L$1"
    else
      OPENSSL_ERROR="OpenSSL library files not found in $1!"
    fi
  fi
fi

AC_SUBST(OPENSSL_LIBPATH)
AC_SUBST(OPENSSL_CPPFLAGS)

dnl -------------------------------------------------------------------------
dnl Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND
dnl -------------------------------------------------------------------------
if test x"$acx_openssl_ok" = xyes; then
  AC_DEFINE(HAVE_OPENSSL,1,
	[Define if you have the OpenSSL libraries and header files.])
  OPENSSL_LIBS="$OPENSSL_LIBPATH -lssl"
  LIBS="$OLIBS"
  AC_SUBST(OPENSSL_LIBS)
  $3
else
  AC_SUBST(OPENSSL_ERROR)
  $4
fi

])dnl ACX_OPENSSL
