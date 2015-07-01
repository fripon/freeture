# - Try to find open ssl
# Once done this will define
#  OPENSSL_FOUND - System has OPENSSL
#  OPENSSL_INCLUDE_DIRS - The OPENSSL include directories
#  OPENSSL_LIBRARIES - The libraries needed to use OPENSSL


FIND_PATH(	OPENSSL_INCLUDE_DIR openssl/ssl.h openssl/err.h
			PATHS
			/usr/include/
			"$ENV{OPENSSL_INCLUDEDIR}"
)

IF(${OperatingSystem} MATCHES "Windows")
	
	set(OPENSSL_LIBRARY "$ENV{OPENSSL_LIBRARYDIR}")

	FIND_LIBRARY(	SSLEAY_LIBRARY 
				NAMES 
				ssleay32MDd ssleay32MD ssleay32 ssleay
				PATHS
				${OPENSSL_LIBRARY}
	)

	FIND_LIBRARY(	LIBEAY_LIBRARY 
					NAMES 
					libeay32MDd libeay32MD libeay32 libeay
					PATHS
					${OPENSSL_LIBRARY}
	)

	set(OPENSSL_LIBRARIES  ${SSLEAY_LIBRARY} ${LIBEAY_LIBRARY})
	
ELSE(${OperatingSystem} MATCHES "Windows")
	IF(${OperatingSystem} MATCHES "Linux")

		set(OPENSSL_LIBRARY "/usr/lib/x86_64-linux-gnu/")

		FIND_LIBRARY(	LIBSSL_LIBRARY 
					NAMES 
					ssl libssl
					PATHS
					${OPENSSL_LIBRARY}
		)

		FIND_LIBRARY(LIBCRYPTO_LIBRARY 
					NAMES 
					crypto libcrypto
					PATHS
					${OPENSSL_LIBRARY}
		)

	set(OPENSSL_LIBRARIES ${LIBSSL_LIBRARY} ${LIBCRYPTO_LIBRARY})		

	ENDIF(${OperatingSystem} MATCHES "Linux")
ENDIF(${OperatingSystem} MATCHES "Windows")

set(OPENSSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR})

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPENSSL DEFAULT_MSG
  OPENSSL_INCLUDE_DIR
  OPENSSL_LIBRARY)

mark_as_advanced(OPENSSL_INCLUDE_DIR OPENSSL_LIBRARIES)
 
