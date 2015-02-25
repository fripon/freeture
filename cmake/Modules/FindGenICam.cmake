# - Try to find Genicam
# Once done this will define
#  GENICAM_FOUND - System has genicam
#  GENICAM_INCLUDE_DIRS - The genicam include directories
#  GENICAM_LIBRARIES - The libraries needed to use genicam
#C:\Program Files\Basler\pylon 3.2\genicam\library\cpp\include
#C:\Program Files\Basler\pylon 3.2\genicam\library\cpp\lib\win64_x64

IF(${OperatingSystem} MATCHES "Windows")
	if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		set( GENICAM_LIBRARY "$ENV{PYLON_GENICAM_ROOT}/library/cpp/lib/win64_x64" )
	else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		set( GENICAM_LIBRARY "$ENV{PYLON_GENICAM_ROOT}/library/cpp/lib/win32_i86" )
	endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )
ELSE(${OperatingSystem} MATCHES "Windows")
	IF(${OperatingSystem} MATCHES "Linux")
		if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
			set( GENICAM_LIBRARY "/opt/pylon3/genicam/bin/Linux64_x64" )
		else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
			set( GENICAM_LIBRARY "/opt/pylon3/genicam/bin/Linux86_x86" )
		endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	ENDIF(${OperatingSystem} MATCHES "Linux")
ENDIF(${OperatingSystem} MATCHES "Windows")

FIND_PATH(	GENICAM_INCLUDE_DIR GenICam.h
			PATHS
			"$ENV{PYLON_GENICAM_ROOT}/library/cpp/include"
			)
message("GENICAM_INCLUDE_DIR :"  ${GENICAM_INCLUDE_DIR})

FIND_LIBRARY(	GENAPI_LIBRARY 
				NAMES 
				GenApi_gcc40_v2_3 GenApi_MD_VC100_v2_3
				PATHS
				${GENICAM_LIBRARY}
)

FIND_LIBRARY(	GCBASE_LIBRARY 
				NAMES 
				GCBase_gcc40_v2_3 GCBase_MD_VC100_v2_3
				PATHS
				${GENICAM_LIBRARY}
)

FIND_LIBRARY(	LOG4CPP_LIBRARY 
				NAMES 
				log4cpp_gcc40_v2_3 log4cpp_MD_VC100_v2_3
				PATHS
				${GENICAM_LIBRARY}
)

FIND_LIBRARY(	LOG_GCC_LIBRARY 
				NAMES 
				Log_gcc40_v2_3 Log_MD_VC100_v2_3
				PATHS
				${GENICAM_LIBRARY}
)

FIND_LIBRARY(	MATHPARSER_LIBRARY 
				NAMES 
				MathParser_gcc40_v2_3 MathParser_MD_VC100_v2_3
				PATHS
				${GENICAM_LIBRARY}
)

set(GENICAM_LIBRARIES ${GENAPI_LIBRARY} ${GCBASE_LIBRARY} ${LOG4CPP_LIBRARY} ${LOG_GCC_LIBRARY} ${MATHPARSER_LIBRARY})
set(GENICAM_INCLUDE_DIRS ${GENICAM_INCLUDE_DIR} )

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GENICAM DEFAULT_MSG
  GENICAM_INCLUDE_DIR
  GENICAM_LIBRARY)

mark_as_advanced(GENICAM_INCLUDE_DIR GENICAM_LIBRARIES)
