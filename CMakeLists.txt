cmake_minimum_required(VERSION 2.6)

project(chromaprint)
set(PROJECT_VERSION 0.1.0)

# 1. If the library source code has changed at all since the last update, then increment revision.
# 2. If any interfaces have been added, removed, or changed since the last update, increment current, and set revision to 0.
# 3. If any interfaces have been added since the last public release, then increment age.
# 4. If any interfaces have been removed since the last public release, then set age to 0. 
set(chromaprint_SOVERSION_CURRENT  0)
set(chromaprint_SOVERSION_REVISION 0)
set(chromaprint_SOVERSION_AGE      0)

math(EXPR chromaprint_SOVERSION_MAJOR "${chromaprint_SOVERSION_CURRENT} - ${chromaprint_SOVERSION_AGE}")
math(EXPR chromaprint_SOVERSION_MINOR "${chromaprint_SOVERSION_AGE}")
math(EXPR chromaprint_SOVERSION_PATCH "${chromaprint_SOVERSION_REVISION}")

set(chromaprint_VERSION ${chromaprint_SOVERSION_MAJOR}.${chromaprint_SOVERSION_MINOR}.${chromaprint_SOVERSION_PATCH})
set(chromaprint_SOVERSION ${chromaprint_SOVERSION_MAJOR})

include(CheckFunctionExists)
set(CMAKE_REQUIRED_LIBRARIES -lm)
check_function_exists(lrintf HAVE_LRINTF)
check_function_exists(round HAVE_ROUND)

add_definitions(-DHAVE_CONFIG_H -D_SCL_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")
set(EXEC_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "Installation prefix for executables and object code libraries" FORCE)
set(BIN_INSTALL_DIR ${EXEC_INSTALL_PREFIX}/bin CACHE PATH "Installation prefix for user executables" FORCE)
set(LIB_INSTALL_DIR ${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX} CACHE PATH  "Installation prefix for object code libraries" FORCE)
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include CACHE PATH "Installation prefix for C header files" FORCE)

#if(CMAKE_COMPILER_IS_GNUCXX)
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
#	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
#endif(CMAKE_COMPILER_IS_GNUCXX)

option(BUILD_TESTS "Build the test suite" OFF)
option(BUILD_TOOLS "Build standard tools" OFF)
option(BUILD_EXTRA_TOOLS "Build extra tools (only useful for development of this library)" OFF)

option(WITH_AVFFT "Use FFmpeg for FFT calculations" OFF)
option(WITH_FFTW3 "Use FFTW3 for FFT calculations" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libchromaprint.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libchromaprint.pc)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libchromaprint.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

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

