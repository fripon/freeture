# - Try to find TIS
# Once done this will define
#  TIS_FOUND - System has TIS
#  TIS_INCLUDE_DIRS - The TIS include directories
#  TIS_LIBRARIES - The libraries needed to use TIS

FIND_PATH(	TIS_INCLUDE_DIR tisudshl.h MemBufferCollection.h
			PATHS
			"$ENV{TIS_INCLUDEDIR}"
)

FIND_LIBRARY(	TIS_LIBRARY 
				NAMES 
				TIS_UDSHL11d_x64 TIS_UDSHL10d_x64
				PATHS
				$ENV{TIS_LIBRARYDIR}
)

set(TIS_LIBRARIES  ${TIS_LIBRARY})
set(TIS_INCLUDE_DIRS ${TIS_INCLUDE_DIR})

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(TIS DEFAULT_MSG
  TIS_INCLUDE_DIR
  TIS_LIBRARY)

mark_as_advanced(TIS_INCLUDE_DIR TIS_LIBRARIES)
 