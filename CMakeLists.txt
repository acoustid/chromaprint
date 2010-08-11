cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(CheckFunctionExists)
set(CMAKE_REQUIRED_LIBRARIES -lm)
check_function_exists(lrintf HAVE_LRINTF)

add_definitions(-DHAVE_CONFIG_H)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

option(BUILD_TESTS "Build the test suite" OFF)
option(BUILD_TOOLS "Build standard tools" OFF)
option(BUILD_EXTRA_TOOLS "Build extra tools (only useful for development of this library)" OFF)

option(WITH_AVFFT "Use FFmpeg for FFT calculations" OFF)
option(WITH_FFTW3 "Use FFTW3 for FFT calculations" OFF)

find_package(FFmpeg)
find_package(FFTW3)

if(WITH_AVFFT AND WITH_FFTW3)
	message(FATAL_ERROR "Only one of WITH_AVFFT and WITH_FFTW3 can be selected")
endif(WITH_AVFFT AND WITH_FFTW3)

if(WITH_AVFFT AND NOT FFMPEG_LIBAVCODEC_FFT_FOUND)
	message(FATAL_ERROR "FFmpeg with avfft.h not found")
endif(WITH_AVFFT AND NOT FFMPEG_LIBAVCODEC_FFT_FOUND)

if(WITH_FFTW3 AND NOT FFTW3_FOUND)
	message(FATAL_ERROR "FFTW3 not found")
endif(WITH_FFTW3 AND NOT FFTW3_FOUND)

if(NOT WITH_AVFFT AND NOT WITH_FFTW3)
	if(FFMPEG_LIBAVCODEC_FFT_FOUND)
		set(WITH_AVFFT ON)
	elseif(FFTW3_FOUND)
		set(WITH_FFTW3 ON)
	else(FFTW3_FOUND)
		message(FATAL_ERROR "Neither FFmpeg with avfft.h nor FFTW3 found")
	endif(FFMPEG_LIBAVCODEC_FFT_FOUND)
endif(NOT WITH_AVFFT AND NOT WITH_FFTW3)

if(WITH_AVFFT)
	message(STATUS "Using FFmpeg for FFT calculations")
endif(WITH_AVFFT)

if(WITH_FFTW3)
	message(STATUS "Using FFTW3 for FFT calculations")
endif(WITH_FFTW3)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(src)

if(BUILD_TOOLS_EXTRA)
	find_package(PNG REQUIRED)
endif(BUILD_TOOLS_EXTRA)

find_package(Boost COMPONENTS system filesystem)
if(BUILD_TOOLS OR BUILD_TOOLS_EXTRA)
	find_package(FFmpeg REQUIRED)
	find_package(Taglib REQUIRED)
	add_subdirectory(tools)
endif(BUILD_TOOLS OR BUILD_TOOLS_EXTRA)

if(BUILD_TESTS)
	find_package(GTest REQUIRED)
	add_subdirectory(tests)
endif(BUILD_TESTS)

