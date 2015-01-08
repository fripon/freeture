dnl
dnl				acx_boost.m4
dnl
dnl Figure out if the Boost package is installed.
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

dnl @synopsis ACX_BOOST([BOOST_LIBSDIR, BOOST_INCDIR,[ACTION-IF-FOUND
dnl                         [, ACTION-IF-NOT-FOUND]]])
dnl This macro figures out if the BOOST libraries are installed.
dnl ACTION-IF-FOUND is a list of shell commands to run if BOOST
dnl is found (HAVE_BOOST is defined first), and ACTION-IF-NOT-FOUND
dnl is a list of commands to run it if it is not found.

AC_DEFUN([ACX_BOOST], [
AC_REQUIRE([AC_CANONICAL_HOST])
acx_boost_ok=no

dnl --------------------
dnl Search include files
dnl --------------------
if test x$2 = x; then
  if test x$1 = x; then
    AC_CHECK_HEADERS([boost/date_time.hpp],[acx_boost_ok=yes])
    if test x$acx_boost_ok = xyes; then
      AC_DEFINE(BOOST_H, "boost/date_time.hpp", [BOOST header filename.])
    else
      BOOST_ERROR="Boost include files not found!"
    fi
  else
    AC_CHECK_HEADERS([$1/include/boost/date_time.hpp], [acx_boost_ok=yes])
    if test x$acx_boost_ok = xyes; then
      AC_DEFINE_UNQUOTED(BOOST_H, "$1/include/boost/date_time.hpp", [BOOST header filename.])
    else
      BOOST_ERROR="Boost include files not found!"
    fi
  fi
else
  AC_CHECK_HEADERS([$2/boost/date_time.hpp], [acx_boost_ok=yes])
  if test x$acx_boost_ok = xyes; then
    AC_DEFINE_UNQUOTED(BOOST_H, "$2/boost/date_time.hpp", [BOOST header filename.])
  else
    BOOST_ERROR="Boost include files not found in $2!"
  fi
fi


dnl --------------------
dnl Search library files
dnl --------------------
if test x$acx_boost_ok = xyes; then
  OLIBS="$LIBS"
  LIBS=""
  if test x$1 = x; then
    AC_CHECK_LIB(boost_program_options, main,, [acx_boost_ok=no],
		[-lboost_program_options])
    AC_CHECK_LIB(boost_thread, main,, [acx_boost_ok=no],
		[-lboost_thread])
    AC_CHECK_LIB(boost_system, main,, [acx_boost_ok=no],
		[-lboost_system])
    AC_CHECK_LIB(boost_log_setup, main,, [acx_boost_ok=no],
		[-lboost_log_setup])
    AC_CHECK_LIB(boost_log, main,, [acx_boost_ok=no],
		[-lboost_log])
    AC_CHECK_LIB(boost_filesystem, main,, [acx_boost_ok=no],
		[-lboost_filesystem])
    AC_CHECK_LIB(boost_date_time, main,, [acx_boost_ok=no],
		[-lboost_date_time])
    AC_CHECK_LIB(boost_chrono, main,, [acx_boost_ok=no],
		[-lboost_chrono])
    AC_CHECK_LIB(boost_iostreams, main,, [acx_boost_ok=no],
		[-lboost_iostreams])		
    if test x$acx_boost_ok = xyes; then
      BOOST_LIBPATH=""
    else
      BOOST_ERROR="Boost library files not found at usual locations!"
    fi
  else
dnl -------------------------
dnl Specific libdir specified
dnl -------------------------
    AC_CHECK_LIB(boost_program_options, main,, [acx_boost_ok=no],
		[-L$1 -lboost_program_options])
    AC_CHECK_LIB(boost_thread, main,, [acx_boost_ok=no],
		[-L$1 -lboost_thread])
    AC_CHECK_LIB(boost_system, main,, [acx_boost_ok=no],
		[-L$1 -lboost_system])
    AC_CHECK_LIB(boost_log_setup, main,, [acx_boost_ok=no],
		[-L$1 -lboost_log_setup])
    AC_CHECK_LIB(boost_log, main,, [acx_boost_ok=no],
		[-L$1 -lboost_log])
    AC_CHECK_LIB(boost_filesystem, main,, [acx_boost_ok=no],
		[-L$1 -lboost_filesystem])
    AC_CHECK_LIB(boost_date_time, main,, [acx_boost_ok=no],
		[-L$1 -lboost_date_time])
    AC_CHECK_LIB(boost_chrono, main,, [acx_boost_ok=no],
		[-L$1 -lboost_chrono])
    AC_CHECK_LIB(boost_iostreams, main,, [acx_boost_ok=no],
		[-L$1 -lboost_iostreams])		
    if test x$acx_boost_ok = xyes; then
      BOOST_LIBPATH="-L$1"
    else
      BOOST_ERROR="Boost library files not found in $1!"
    fi
  fi
fi

AC_SUBST(BOOST_LIBPATH)
AC_SUBST(BOOST_CPPFLAGS)

dnl -------------------------------------------------------------------------
dnl Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND
dnl -------------------------------------------------------------------------
if test x"$acx_boost_ok" = xyes; then
  AC_DEFINE(HAVE_BOOST,1,
	[Define if you have the Boost libraries and header files.])
  BOOST_LIBS="$BOOST_LIBPATH -lboost_program_options -lboost_thread -lboost_system -lboost_log_setup -lboost_log -lboost_filesystem -lboost_date_time -lboost_chrono -lboost_iostreams"
  LIBS="$OLIBS"
  AC_SUBST(BOOST_LIBS)
  $3
else
  AC_SUBST(BOOST_ERROR)
  $4
fi

])dnl ACX_BOOST
