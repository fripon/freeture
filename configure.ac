#
#				configure.ac
#
# Process this file with autoconf to generate a configure script.
#
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
#*	Last modified:		27/09/2016
#*
#*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

# First, disable the annoying config.cache
define([AC_CACHE_LOAD],)
define([AC_CACHE_SAVE],)

AC_PREREQ([2.69])
AC_INIT([freeture], [1.1-0], [yoan.audureau@u-psud.fr])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE
date=`date +%Y-%m-%d`
date2=`date +"%a %b %d %Y"`
date3=`date +"%B %Y"`
AC_DEFINE_UNQUOTED(DATE, "$date", [Archive creation date])
AC_SUBST(PACKAGER, "Yoan Audureau")
AC_SUBST(DATE2, "$date2")
AC_SUBST(DATE3, "$date3")

AC_DEFINE(LINUX, 1, [Linux plateform])

# Include macros
sinclude(acx_urbi_resolve_dir.m4)
sinclude(acx_boost.m4)
sinclude(acx_opencv.m4)
sinclude(acx_cfitsio.m4)
sinclude(acx_pylon.m4)

# Display package and version number
AC_MSG_RESULT([******** Configuring:  $PACKAGE_NAME $VERSION ($date) ********])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for languages.
AC_LANG(C)
AC_LANG(C++)

### Adding C++11 compatibility compilation flag
CPPFLAGS="$CPPFLAGS -std=c++11 -std=c++0x"

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor memset modf pow sqrt])

# Checks for libraries.
PKG_CHECK_MODULES(GLIB_2_0, glib-2.0, [LIBS="$GLIB_2_0_LIBS $LIBS"]
				      [AM_CPPFLAGS="$AM_CPPFLAGS $GLIB_2_0_CFLAGS"],
				      [AC_MSG_ERROR([$GLIB_2_0_ERROR Exiting.])])
PKG_CHECK_MODULES(GOBJ_2_0, gobject-2.0, [LIBS="$GOBJ_2_0_LIBS $LIBS"]
				      [AM_CPPFLAGS="$AM_CPPFLAGS $GOBJ_2_0_CFLAGS"],
				      [AC_MSG_ERROR([$GOBJ_2_0_ERROR Exiting.])])
PKG_CHECK_MODULES(UDEV, libudev, [LIBS="$UDEV_LIBS $LIBS"]
				      [AM_CPPFLAGS="$AM_CPPFLAGS $UDEV_CFLAGS"],
				      [AC_MSG_ERROR([$UDEV_ERROR Exiting.])])
PKG_CHECK_MODULES(CRYPTO, libcrypto, [LIBS="$CRYPTO_LIBS $LIBS"]
				      [AM_CPPFLAGS="$AM_CPPFLAGS $CRYPTO_CFLAGS"],
				      [AC_MSG_ERROR([$CRYPTO_ERROR Exiting.])])
PKG_CHECK_MODULES(V4L2, libv4l2, [LIBS="$V4L2_LIBS $LIBS"]
				      [AM_CPPFLAGS="$AM_CPPFLAGS $V4L2_CFLAGS"],
				      [AC_MSG_ERROR([$V4L2_ERROR Exiting.])])

# Provide special options for PYLON SDK (Basler Camera)
AC_MSG_CHECKING([whether we should use the Pylon SDK for Basler Camera])
AC_ARG_ENABLE(pylon,
	[AS_HELP_STRING([--enable-pylon],
	[Use the Pylon SDK (off by default)])],
	[AC_MSG_RESULT([yes])],
	AC_MSG_RESULT([no]))
AC_ARG_WITH(pylon-libdir,
	[AS_HELP_STRING([--with-pylon-libdir=<Pylon library path>],
	[Provide an alternative path to the Pylon library])])
AC_ARG_WITH(pylon-incdir,
	[AS_HELP_STRING([--with-pylon-incdir=<Pylon include dir>],
	[Provide an alternative path to the Pylon include directory])])

# Provide special options for BOOST
AC_ARG_WITH(boost-libdir,
        [AS_HELP_STRING([--with-boost-libdir=<BOOST library path>],
	[Alternative directory for the BOOST library])])
AC_ARG_WITH(boost-incdir,
        [AS_HELP_STRING([--with-boost-incdir=<BOOST include dir>],
	[Alternative directory for BOOST include files])])

# Provide special options for OPENCV
AC_ARG_WITH(opencv-libdir,
        [AS_HELP_STRING([--with-opencv-libdir=<OPENCV library path>],
	[Alternative directory for the OPENCV library])])
AC_ARG_WITH(opencv-incdir,
        [AS_HELP_STRING([--with-opencv-incdir=<OPENCV include dir>],
	[Alternative directory for OPENCV include files])])

# Provide special options for OPENSSL
AC_ARG_WITH(openssl-libdir,
        [AS_HELP_STRING([--with-openssl-libdir=<OPENSSL library path>],
        [Alternative directory for the OPENSSL library])])
AC_ARG_WITH(openssl-incdir,
        [AS_HELP_STRING([--with-openssl-incdir=<OPENSSL include dir>],
        [Alternative directory for OPENSSL include files])])

# Provide special options for V4L2
AC_ARG_WITH(v4l2-libdir,
        [AS_HELP_STRING([--with-v4l2-libdir=<V4L2 library path>],
        [Alternative directory for the V4L2 library])])
AC_ARG_WITH(v4l2-incdir,
        [AS_HELP_STRING([--with-v4l2-incdir=<V4L2 include dir>],
        [Alternative directory for V4L2 include files])])

# Provide special options for CFITSIO
AC_ARG_WITH(cfitsio-libdir,
        [AS_HELP_STRING([--with-cfitsio-libdir=<CFITSIO library path>],
	[Alternative directory for the CFITSIO library])])
AC_ARG_WITH(cfitsio-incdir,
        [AS_HELP_STRING([--with-cfitsio-incdir=<CFITSIO include dir>],
	[Alternative directory for CFITSIO include files])])

# Provide a special option for the default config file path
with_cfg_path=$(URBI_RESOLVE_DIR([$datadir]))"/$PACKAGE_NAME/"
AC_ARG_WITH(cfg_path,
        [AS_HELP_STRING([--with-cfg_path=<default path for conf file>],
        [Provide an alternative default path for the configuration file])])

AC_DEFINE_UNQUOTED([CFG_PATH], "$with_cfg_path",[Default path for the configuration file])

################# handle the Pylon library ##################
if test "$enable_pylon" = "yes"; then
  AC_DEFINE(USE_PYLON, 1, [Pylon activated for Basler cameras])
  ACX_PYLON($with_pylon_libdir,$with_pylon_incdir,
	[use_pylon="yes"]
    	[AM_CPPFLAGS="$AM_CPPFLAGS $PYLON_CPPFLAGS"]
    	[LIBS="$LIBS $PYLON_LIBS"]
    	AC_MSG_WARN([Pylon activated for Basler cameras!]),
    	[use_pylon="no"]
    	AC_MSG_ERROR([$PYLON_ERROR Exiting.]))
fi
AM_CONDITIONAL(USE_PYLON, test "$use_pylon" = "yes")

###################### handle the BOOST library ######################
ACX_BOOST($with_boost_libdir,$with_boost_incdir,
    	[AM_CPPFLAGS="$AM_CPPFLAGS $BOOST_CPPFLAGS"]
	[LIBS="$BOOST_LIBS $LIBS"]
	if test "$BOOST_WARN" != ""; then
	  AC_MSG_WARN([$BOOST_WARN])
	fi,
	AC_MSG_ERROR([$BOOST_ERROR Exiting.]))						

###################### handle the OPENCV library ######################
ACX_OPENCV($with_opencv_libdir,$with_opencv_incdir,
    	[AM_CPPFLAGS="$AM_CPPFLAGS $OPENCV_CPPFLAGS"]
	[LIBS="$OPENCV_LIBS $LIBS"]
	if test "$OPENCV_WARN" != ""; then
	  AC_MSG_WARN([$OPENCV_WARN])
	fi,
	AC_MSG_ERROR([$OPENCV_ERROR Exiting.]))						

###################### handle the CFITSIO library ######################
ACX_CFITSIO($with_cfitsio_libdir,$with_cfitsio_incdir,
    	[AM_CPPFLAGS="$AM_CPPFLAGS $CFITSIO_CFLAGS"]
	[LIBS="$CFITSIO_LIBS $LIBS"]
	if test "$CFITSIO_WARN" != ""; then
	  AC_MSG_WARN([$CFITSIO_WARN])
	fi,
	AC_MSG_ERROR([$CFITSIO_ERROR Exiting.]))						

###################### handle the OPENSSL library ######################
PKG_CHECK_MODULES(OPENSSL, openssl,
	[LIBS="$OPENSSL_LIBS $LIBS"]
	[AM_CPPFLAGS="$AM_CPPFLAGS $OPENSSL_CFLAGS"],
	[AC_MSG_ERROR([$OPENSSL_ERROR Exiting.])])

###################### handle the GSTREAMER library ######################
PKG_CHECK_MODULES(GST01, gstreamer-0.10,
	[LIBS="$GST01_LIBS $LIBS"]
	[AM_CPPFLAGS="$AM_CPPFLAGS $GST01_CFLAGS"],
	[AC_MSG_ERROR([$GST_ERROR Exiting.])])
	
###################### handle the ARAVIS library ######################
PKG_CHECK_MODULES(ARV, aravis-0.6,
	[LIBS="$ARV_LIBS $LIBS"]
	[AM_CPPFLAGS="$AM_CPPFLAGS $ARV_CFLAGS"],
	[AC_MSG_ERROR([$ARV_ERROR Exiting.])])

AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_LDFLAGS)

# Display compiler and linker flags
AC_MSG_RESULT([***************************************************************])
AC_MSG_RESULT([Compile cmdline:  $CC $AM_CPPFLAGS $CPPFLAGS $AM_CFLAGS $CFLAGS])
AC_MSG_RESULT([Link    cmdline:  $CC $AM_LDFLAGS $LDFLAGS $LIBS])
AC_MSG_RESULT([***************************************************************])

AC_CONFIG_FILES([Makefile
		 README
		 INSTALL
                 doc/Makefile
		 doc/freeture.1
                 src/Makefile
                 ])
AC_OUTPUT
