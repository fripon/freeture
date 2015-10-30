cmake_minimum_required(VERSION 2.6)
#------------------------------------------------
# Project name.
project(freeture)
#------------------------------------------------
# The version number.
set(VERSION 1.0.1)
# Configuration file default path
set(CFG_PATH /usr/local/share/freeture/ )
#------------------------------------------------
# Executable name.
set(EXE_NAME FreeTure)
#------------------------------------------------
# Sources.
file(
	GLOB_RECURSE
	SRC_FILES
	../src/*
)
message("> SOURCES : " ${SRC_FILES})
#------------------------------------------------
# To see the command line CMake produces :
set(CMAKE_VERBOSE_MAKEFILE on)
#------------------------------------------------
# Output path.
set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})
#------------------------------------------------
# Get the complete system name.
message("> Complete system name : " ${CMAKE_SYSTEM})
#------------------------------------------------
# Get operating system.
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	OPTION(USE_PYLON "Include Pylon for Basler Cameras on Windows" OFF)
	OPTION(LINUX "Linux system" ON)
	OPTION(WINDOWS "Windows system" OFF)
	#SET(SRC_FILES ${SRC_FILES} src/CameraSDKAravis.h src/CameraSDKAravis.cpp)
	message("> System name : "
			${CMAKE_SYSTEM_NAME} " --> Linux")
    SET(OperatingSystem "Linux")
ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		OPTION(USE_PYLON "Include Pylon for Basler Cameras on Windows" ON)
		OPTION(LINUX "Linux system" OFF)
		OPTION(WINDOWS "Windows system" ON)

		#SET(SRC_FILES ${SRC_FILES} src/CameraSDKPylon.h src/CameraSDKPylon.cpp)
		message("> System name : "
				${CMAKE_SYSTEM_NAME} " --> Windows")
		SET(OperatingSystem "Windows")
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
#------------------------------------------------
# Get processor.
message("> Processor : " ${CMAKE_SYSTEM_PROCESSOR})
#------------------------------------------------
# Executable.
add_executable( ${EXE_NAME} ${SRC_FILES} )
#------------------------------------------------


include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
IF(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ELSEIF(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
ELSE()
     message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
ENDIF()



#------------------------------------------------
# Add path to find cmake modules.
# http://www.vtk.org/Wiki/CMake%3aHow_To_Find_Libraries#How_package_finding_works
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Modules/")
#------------------------------------------------
# OPENCV.
message("> Searching opencv...")
find_package( OpenCV REQUIRED )

if(OpenCV_FOUND)
	message("> Success to find OPENCV.")
	link_directories(${OpenCV_LIBRARY_DIRS})
	include_directories( ${OpenCV_INCLUDE_DIRS} )
	message("OPENCV LIBRARY DIRS : " ${OpenCV_LIBRARY_DIRS})
	message("OPENCV INCLUDE DIRS : " ${OpenCV_INCLUDE_DIRS})
	message("OPENCV LIBS : " ${OpenCV_LIBRARIES})
	target_link_libraries(${EXE_NAME} ${OpenCV_LIBRARIES})
else(NOT OpenCV_FOUND)
	message("> Fail to find opencv.")
endif(OpenCV_FOUND)
#------------------------------------------------
# CFITSIO.
message("> Searching cfitsio...")
find_package(CFITSIO)

if(CFITSIO_FOUND)
	message("> Success to find cfitsio.")
	link_directories(${CFITSIO_LIBRARY_DIRS})
	message("CFITSIO_INCLUDE_DIR : " ${CFITSIO_INCLUDE_DIR} )
	message("CFITSIO_LIBRARIES : " ${CFITSIO_LIBRARIES} )
	include_directories(${CFITSIO_INCLUDE_DIR})
	target_link_libraries( ${EXE_NAME} ${CFITSIO_LIBRARIES} )
else(NOT CFITSIO_FOUND)
	message("> Fail to find cfitsio.")
endif(CFITSIO_FOUND)
#------------------------------------------------
# Find required packages according to the platform.
IF(${OperatingSystem} MATCHES "Windows")
	message("windows!")

	# Boost Library.
	# http://www.cmake.org/cmake/help/v3.0/module/FindBoost.html
	message("> Searching boost...")

	include(FindBoost)
	SET(Boost_USE_STATIC_LIBS ON)
	find_package(	Boost
					1.55.0
					COMPONENTS
					thread
					system
					date_time
					chrono
					filesystem
					log
					log_setup
					program_options
					iostreams
					serialization
					regex
					REQUIRED)

	if(Boost_FOUND)
		message("> Success to find boost.")
		message("> Boost_INCLUDE_DIRS : " ${Boost_INCLUDE_DIRS})
		include_directories(${Boost_INCLUDE_DIRS})
		message("> Boost_LIBRARY_DIRS : " ${Boost_LIBRARY_DIRS})
		LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
		message("> Boost_LIBRARIES : " ${Boost_LIBRARIES})
		target_link_libraries(${EXE_NAME} ${Boost_LIBRARIES})
	else(NOT Boost_FOUND)
		message("> Fail to find boost")
		message("> Boost_INCLUDE_DIRS : " ${Boost_INCLUDE_DIRS})
	endif(Boost_FOUND)

	# Genicam.
	message("> Searching GenICam...")
	find_package( GenICam )
	message ("GENICAM_FOUND : " ${GENICAM_FOUND})
	message ("GENICAM_INCLUDE_DIR : " ${GENICAM_INCLUDE_DIR})
	message ("GENICAM_LIBRARIES : " ${GENICAM_LIBRARIES})
	message ("GENICAM_LIBRARY : " ${GENICAM_LIBRARY})
	include_directories(${GENICAM_INCLUDE_DIR})
	link_directories(${GENICAM_LIBRARY})
	target_link_libraries( ${EXE_NAME} ${GENICAM_LIBRARIES} )

	# Pylon
	message("> Searching pylon...")
	find_package( Pylon )
	message ("PYLON_FOUND : " ${PYLON_FOUND})
	message ("PYLON_INCLUDE_DIR : " ${PYLON_INCLUDE_DIR})
	message ("PYLON_LIBRARIES : " ${PYLON_LIBRARIES})
	message ("PYLON_LIBRARY : " ${PYLON_LIBRARY})
	include_directories(${PYLON_INCLUDE_DIR})
	link_directories(${PYLON_LIBRARY})
	target_link_libraries( ${EXE_NAME} ${PYLON_LIBRARIES} )

	# TIS
	message("> Searching the imaging source...")
	find_package( TIS )
	message ("TIS_FOUND : " ${TIS_FOUND})
	message ("TIS_INCLUDE_DIR : " ${TIS_INCLUDE_DIR})
	message ("TIS_LIBRARIES : " ${TIS_LIBRARIES})
	message ("TIS_LIBRARY : " ${TIS_LIBRARY})
	include_directories(${TIS_INCLUDE_DIR})
	#link_directories(${TIS_LIBRARY})
	target_link_libraries( ${EXE_NAME} ${TIS_LIBRARIES} )

	# OPENSSL
	message("> Searching openssl...")
	find_package( OPENSSL )
	message ("OPENSSL_FOUND : " ${OPENSSL_FOUND})
	message ("OPENSSL_INCLUDE_DIR : " ${OPENSSL_INCLUDE_DIR})
	message ("OPENSSL_LIBRARIES : " ${OPENSSL_LIBRARIES})
	message ("OPENSSL_LIBRARY : " ${OPENSSL_LIBRARY})
	include_directories(${OPENSSL_INCLUDE_DIR})
	link_directories(${OPENSSL_LIBRARY})
	target_link_libraries( ${EXE_NAME} ${OPENSSL_LIBRARIES} )

ELSE(${OperatingSystem} MATCHES "Windows")
	IF(${OperatingSystem} MATCHES "Linux")


	# Not include Pylon for Basler Cameras on Windows.
	#OPTION(USE_PYLON "Include Pylon for Basler Cameras on Windows" OFF)

	# Boost Library.
	# http://www.cmake.org/cmake/help/v3.0/module/FindBoost.html
	message("> Searching boost...")

	include(FindBoost)
	#SET(Boost_USE_STATIC_LIBS ON)
	#ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
	find_package(	Boost
					1.55.0
					COMPONENTS
					thread
					system
					date_time
					chrono
					filesystem
					log
					log_setup
					program_options
					iostreams
					serialization
					REQUIRED)

	if(Boost_FOUND)
		message("> Success to find boost.")
		message("> Boost_INCLUDE_DIRS : " ${Boost_INCLUDE_DIRS})
		include_directories(${Boost_INCLUDE_DIRS})
		message("> Boost_LIBRARY_DIRS : " ${Boost_LIBRARY_DIRS})
		LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
		message("> Boost_LIBRARIES : " ${Boost_LIBRARIES})
		target_link_libraries(${EXE_NAME} ${Boost_LIBRARIES})
	else(NOT Boost_FOUND)
		message("> Fail to find boost")
	endif(Boost_FOUND)

	# Thread library of the system (pthread for linux)
	# http://www.cmake.org/cmake/help/v3.0/module/FindThreads.html
	find_package( Threads REQUIRED )
	message("CMAKE_THREAD_LIBS_INIT : " ${CMAKE_THREAD_LIBS_INIT})
	target_link_libraries(${EXE_NAME} ${CMAKE_THREAD_LIBS_INIT})

	# glib-2.0
	find_package( GLIB2 REQUIRED)
	message ("GLIB2_INCLUDE_DIR  : " ${GLIB2_INCLUDE_DIR})
	message ("GLIB2_LIBRARIES : " ${GLIB2_LIBRARIES})
	include_directories(${GLIB2_INCLUDE_DIR})
	#link_directories(${GLIB2_LIBRARY_DIRS})
	target_link_libraries(${EXE_NAME} ${GLIB2_LIBRARIES})

	# gobject-2.0
	# https://gitorious.org/phonon/phonon/source/e2bfb1fd2cb5721d54573cd8c9ff361e615d1c00:cmake/FindGLIB2.cmake#L52
	find_package( GObject REQUIRED)
	message ("GOBJECT_INCLUDE_DIR : " ${GOBJECT_INCLUDE_DIR})
	message ("GOBJECT_LIBRARIES : " ${GOBJECT_LIBRARIES})
	include_directories(${GOBJECT_INCLUDE_DIR})
	#link_directories(${GOBJECT_LIBRARY_DIRS})
	target_link_libraries(${EXE_NAME} ${GOBJECT_LIBRARIES})

	# Aravis
	# https://code.google.com/p/tiy/source/browse/trunk/src/Findaravis.cmake
	find_package( aravis REQUIRED)
	message ("ARAVIS_FOUND : " ${aravis_FOUND})
	message ("ARAVIS_INCLUDE_DIR : " ${aravis_INCLUDE_DIR})
	message ("ARAVIS_LIBRARIES : " ${aravis_LIBRARIES})
	message ("aravis_LIBRARY : " ${aravis_LIBRARY})
	include_directories(${aravis_INCLUDE_DIR})
	#link_directories(${ARAVIS_LIBRARY_DIRS})
	target_link_libraries(${EXE_NAME} ${aravis_LIBRARIES})

	# Openssl
	include(FindOpenSSL)
	find_package( OpenSSL REQUIRED)
	message ("OPENSSL_FOUND : " ${OPENSSL_FOUND})
	message ("OPENSSL_INCLUDE_DIR : " ${OPENSSL_INCLUDE_DIR})
	message ("OPENSSL_LIBRARIES : " ${OPENSSL_LIBRARIES})
	message ("OPENSSL_LIBRARY : " ${OPENSSL_LIBRARY})
	include_directories(${OPENSSL_INCLUDE_DIR})
	#link_directories(${OPENSSL_LIBRARY_DIRS})
	target_link_libraries(${EXE_NAME} ${OPENSSL_LIBRARIES})

	# V4L2
	include(FindV4L2)
	find_package( V4L2 REQUIRED)
	message ("V4L2_FOUND : " ${V4L2_FOUND})
	message ("V4L2_INCLUDE_DIR : " ${V4L2_INCLUDE_DIR})
	message ("V4L2_LIBRARIES : " ${V4L2_LIBRARIES})
	message ("V4L2_LIBRARY : " ${V4L2_LIBRARY})
	include_directories(${V4L2_INCLUDE_DIR})
	#link_directories(${OPENSSL_LIBRARY_DIRS})
	target_link_libraries(${EXE_NAME} ${V4L2_LIBRARIES})

	ENDIF(${OperatingSystem} MATCHES "Linux")
ENDIF(${OperatingSystem} MATCHES "Windows")

#------------------------------------------------
# configure a header file to pass some of the CMake settings
# to the source code
configure_file(
#${PROJECT_SOURCE_DIR}
  "./config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)
include_directories("${PROJECT_BINARY_DIR}")


