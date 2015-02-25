# - Try to find Aravis
# Once done this will define
#  ARAVIS_FOUND - System has aravis-0.4
#  ARAVIS_INCLUDE_DIRS - The aravis-0.4 include directories
#  ARAVIS_LIBRARIES - The libraries needed to use aravis-0.4

# http://www.cmake.org/Wiki/CMake:How_To_Find_Libraries#Using_LibFindMacros

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(aravis_PKGCONF aravis-0.4)

# Include dir
find_path(aravis_INCLUDE_DIR
	NAMES arv.h
	PATHS ${aravis_PKGCONF_INCLUDE_DIRS} 
)
message("path : "  ${aravis_PKGCONF_INCLUDE_DIRS} )
message("include : " ${INCLUDE_DIR})

# Finally the library itself
find_library(aravis_LIBRARY
	NAMES aravis-0.4
	PATHS 
	${aravis_PKGCONF_LIBRARY_DIRS}
	
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(aravis_PROCESS_INCLUDES aravis_INCLUDE_DIR aravis_INCLUDE_DIRS)
set(aravis_PROCESS_LIBS aravis_LIBRARY aravis_LIBRARIES)
libfind_process(aravis)
