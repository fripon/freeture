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

AUTOMAKE_OPTIONS	= foreign no-dependencies
SUBDIRS			= doc src
dist_pkgdata_DATA	= share/configuration.cfg
EXTRA_DIST		= README NEWS INSTALL acx_boost.m4 acx_cfitsio.m4 \
			acx_opencv.m4 acx_pylon.m4 acx_urbi_resolve_dir.m4 \
			bootstrap.sh 
#RPM_ROOTDIR		= `rpmbuild --nobuild -E %_topdir`
#RPM_SRCDIR		= $(RPM_ROOTDIR)/SOURCES
#dist-hook:
#	rm -rf `find $(distdir) -name .svn`

#rpm:	dist
#	cp -f $(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.gz $(RPM_SRCDIR)
#	rpmbuild -ba --clean $(PACKAGE_NAME).spec

#rpm-icc:	dist
#	cp -f $(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.gz $(RPM_SRCDIR)
#	USE_ICC="1" rpmbuild -ba --clean --nodeps $(PACKAGE_NAME).spec

#rpm-best:	dist
#	cp -f $(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.gz $(RPM_SRCDIR)
#	USE_BEST="1" rpmbuild -ba --clean --nodeps $(PACKAGE_NAME).spec


