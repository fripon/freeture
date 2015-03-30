cmake_minimum_required(VERSION 2.6)
#------------------------------------------------
# Project name. 
project(freeture)
#------------------------------------------------
# The version number. 
set(VERSION 0.6)
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
	#../src/AstThread.h ../src/AstThread.cpp
	#../src/Base64.h ../src/Base64.cpp
	#../src/Camera.h ../src/Camera.cpp
	#../src/CameraFrames.h src/CameraFrames.cpp
	#../src/CameraSDK.h src/CameraSDK.cpp
	#../src/CameraVideo.h src/CameraVideo.cpp
	#../src/Configuration.h src/Configuration.cpp
	#../src/Conversion.h src/Conversion.cpp
	#../src/DetByLines.h src/DetByLines.cpp
	#../src/DetByLists.h src/DetByLists.cpp
	#../src/DetThread.h src/DetThread.cpp
	#../src/ECamBitDepth.h 
	#../src/ECamType.h 
	#../src/EDetMeth.h 
	#../src/EImgBitDepth.h 
	#../src/ELogSeverityLevel.h
	#../src/EParser.h src/EParser.cpp
	#../src/EStackMeth.h 
	#../src/Fits.h src/Fits.cpp
	#../src/Fits2D.h src/Fits2D.cpp
	#src/Fits3D.h src/Fits3D.cpp
	#src/Frame.h src/Frame.cpp
	#src/FreeTure.h src/FreeTure.cpp
	#src/GlobalEvent.h src/GlobalEvent.cpp
	#src/Histogram.h src/Histogram.cpp
	#src/ImgReduction.h src/ImgReduction.cpp
	#src/includes.h
	#src/LocalEvent.h src/LocalEvent.cpp
	#src/main.cpp
	#src/ManageFiles.h src/ManageFiles.cpp
	#src/PixelEvent.h src/PixelEvent.cpp
	#src/RecEvent.h src/RecEvent.cpp
	#src/SaveImg.h src/SaveImg.cpp
	#src/SMTPClient.h src/SMTPClient.cpp
	#src/StackedFrames.h src/StackedFrames.cpp
	#src/TimeDate.h src/TimeDate.cpp
	#src/CameraSDKAravis.h src/CameraSDKAravis.cpp
	#src/CameraSDKPylon.h src/CameraSDKPylon.cpp
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


