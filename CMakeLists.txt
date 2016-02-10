cmake_minimum_required(VERSION 2.6)

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 OLD)
endif()

project(chromaprint)
set(PROJECT_VERSION 1.3.1)

# 1. If the library source code has changed at all since the last update, then increment revision.
# 2. If any interfaces have been added, removed, or changed since the last update, increment current, and set revision to 0.
# 3. If any interfaces have been added since the last public release, then increment age.
# 4. If any interfaces have been removed since the last public release, then set age to 0. 
set(chromaprint_SOVERSION_CURRENT  4)
set(chromaprint_SOVERSION_REVISION 0)
set(chromaprint_SOVERSION_AGE      3)

math(EXPR chromaprint_SOVERSION_MAJOR "${chromaprint_SOVERSION_CURRENT} - ${chromaprint_SOVERSION_AGE}")
math(EXPR chromaprint_SOVERSION_MINOR "${chromaprint_SOVERSION_AGE}")
math(EXPR chromaprint_SOVERSION_PATCH "${chromaprint_SOVERSION_REVISION}")

set(chromaprint_VERSION ${chromaprint_SOVERSION_MAJOR}.${chromaprint_SOVERSION_MINOR}.${chromaprint_SOVERSION_PATCH})
set(chromaprint_SOVERSION ${chromaprint_SOVERSION_MAJOR})

include(CheckFunctionExists)
set(CMAKE_REQUIRED_LIBRARIES -lm)
check_function_exists(lrintf HAVE_LRINTF)
check_function_exists(round HAVE_ROUND)

add_definitions(
	-DHAVE_CONFIG_H
	-D_SCL_SECURE_NO_WARNINGS
	-D_USE_MATH_DEFINES
	-D__STDC_LIMIT_MACROS
	-D__STDC_CONSTANT_MACROS
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")
set(EXEC_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "Installation prefix for executables and object code libraries" FORCE)
set(BIN_INSTALL_DIR ${EXEC_INSTALL_PREFIX}/bin CACHE PATH "Installation prefix for user executables" FORCE)
set(LIB_INSTALL_DIR ${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX} CACHE PATH  "Installation prefix for object code libraries" FORCE)
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include CACHE PATH "Installation prefix for C header files" FORCE)

if(APPLE)
	option(BUILD_FRAMEWORK "Build an OS X framework" OFF)
	set(FRAMEWORK_INSTALL_DIR "/Library/Frameworks" CACHE STRING "Directory to install frameworks to.")
endif()

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_EXAMPLES "Build the examples" OFF)
option(BUILD_TESTS "Build the test suite" OFF)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif()

if(NOT BUILD_SHARED_LIBS)
	add_definitions(-DCHROMAPRINT_NODLL)
endif()

option(WITH_AVFFT "Use FFmpeg for FFT calculations" OFF)
option(WITH_FFTW3 "Use FFTW3 for FFT calculations" OFF)
option(WITH_VDSP "Use vDSP for FFT calculations" OFF)
option(WITH_KISSFFT "Use KissFFT for FFT calculations" OFF)

set(TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests/)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
find_package(FFmpeg)
find_package(FFTW3)
if(APPLE)
	find_library(ACCELERATE_LIBRARIES Accelerate)
endif()
find_package(KissFFT)

set(FFT_OPTION_COUNT 0)

if(WITH_AVFFT)
	math(EXPR FFT_OPTION_COUNT "${FFT_OPTION_COUNT} + 1")
endif()
if(WITH_FFTW3)
	math(EXPR FFT_OPTION_COUNT "${FFT_OPTION_COUNT} + 1")
endif()
if(WITH_VDSP)
	math(EXPR FFT_OPTION_COUNT "${FFT_OPTION_COUNT} + 1")
endif()
if(WITH_KISSFFT)
	math(EXPR FFT_OPTION_COUNT "${FFT_OPTION_COUNT} + 1")
endif()
if(FFT_OPTION_COUNT GREATER "1")
	message(FATAL_ERROR "Only one of WITH_AVFFT, WITH_FFTW3, WITH_VDSP and WITH_KISSFFT can be selected")
endif()

if(WITH_AVFFT AND NOT FFMPEG_LIBAVCODEC_FFT_FOUND)
	message(FATAL_ERROR "FFmpeg with avfft.h not found")
endif()

if(WITH_FFTW3 AND NOT FFTW3_FOUND)
	message(FATAL_ERROR "FFTW3 not found")
endif()

if(APPLE AND WITH_VDSP AND NOT ACCELERATE_LIBRARIES)
	message(FATAL_ERROR "vDSP (Accelerate) not found")
endif()

if(WITH_KISSFFT AND NOT KISSFFT_FOUND)
	message(FATAL_ERROR "KissFFT not found")
endif()

if(NOT WITH_AVFFT AND NOT WITH_FFTW3 AND NOT WITH_KISSFFT)
	if(APPLE AND ACCELERATE_LIBRARIES)
		set(WITH_VDSP ON)
	elseif(FFMPEG_LIBAVCODEC_FFT_FOUND)
		set(WITH_AVFFT ON)
	elseif(FFTW3_FOUND)
		set(WITH_FFTW3 ON)
	elseif(KISSFFT_FOUND)
		set(WITH_KISSFFT ON)
	else()
		message(FATAL_ERROR "Neither FFmpeg with avfft.h nor FFTW3 found")
	endif()
endif()

if(WITH_AVFFT)
	message(STATUS "Using FFmpeg for FFT calculations")
endif()

if(WITH_FFTW3)
	message(STATUS "Using FFTW3 for FFT calculations")
endif()

if(WITH_KISSFFT)
	message(STATUS "Using KissFFT for FFT calculations")
endif()

if(NOT BUILD_FRAMEWORK)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libchromaprint.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libchromaprint.pc)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libchromaprint.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(src)

find_package(Boost COMPONENTS system filesystem)
if(BUILD_EXAMPLES)
	find_package(FFmpeg REQUIRED)
endif()

if(BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

if(BUILD_TESTS)
	find_package(Threads)
	find_package(GTest REQUIRED)
	add_subdirectory(tests)
endif(BUILD_TESTS)

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)

add_custom_target(uninstall
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

