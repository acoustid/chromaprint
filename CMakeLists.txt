cmake_minimum_required(VERSION 2.8)

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 OLD)
endif()

project(chromaprint C CXX)
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

set(AUDIO_PROCESSOR_LIB CACHE STRING "Library to use for audio processing")
set_property(CACHE AUDIO_PROCESSOR_LIB PROPERTY STRINGS avresample swresample)

set(FFT_LIB CACHE STRING "Library to use for FFT calculations")
set_property(CACHE FFT_LIB PROPERTY STRINGS avfft fftw3 fftw3f kissfft vdsp)

set(chromaprint_VERSION ${chromaprint_SOVERSION_MAJOR}.${chromaprint_SOVERSION_MINOR}.${chromaprint_SOVERSION_PATCH})
set(chromaprint_SOVERSION ${chromaprint_SOVERSION_MAJOR})

include(CMakePushCheckState)
include(CheckFunctionExists)
include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

cmake_push_check_state(RESET)
set(CMAKE_REQUIRED_LIBRARIES -lm)
check_function_exists(lrintf HAVE_LRINTF)
check_function_exists(round HAVE_ROUND)
cmake_pop_check_state()

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
	set(FRAMEWORK_INSTALL_DIR "/Library/Frameworks" CACHE STRING "Directory to install frameworks to")
endif()

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TOOLS "Build command line tools" OFF)
option(BUILD_TESTS "Build test suite" OFF)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif()

if(NOT BUILD_SHARED_LIBS)
	add_definitions(-DCHROMAPRINT_NODLL)
endif()

set(TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests/)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

find_package(FFmpeg)

if(FFMPEG_LIBAVCODEC_FOUND)
	cmake_push_check_state(RESET)
	set(CMAKE_REQUIRED_LIBRARIES ${FFMPEG_LIBAVCODEC_LIBRARIES})
	set(CMAKE_REQUIRED_INCLUDES ${FFMPEG_LIBAVCODEC_INCLUDE_DIRS})
	check_function_exists(av_packet_unref HAVE_AV_PACKET_UNREF)
	cmake_pop_check_state()
endif()

if(FFMPEG_LIBAVUTIL_FOUND)
	cmake_push_check_state(RESET)
	set(CMAKE_REQUIRED_LIBRARIES ${FFMPEG_LIBAVUTIL_LIBRARIES})
	set(CMAKE_REQUIRED_INCLUDES ${FFMPEG_LIBAVUTIL_INCLUDE_DIRS})
	check_function_exists(av_frame_alloc HAVE_AV_FRAME_ALLOC)
	check_function_exists(av_frame_free HAVE_AV_FRAME_FREE)
	cmake_pop_check_state()
endif()

if(NOT FFT_LIB OR FFT_LIB STREQUAL "fftw3" OR FFT_LIB STREQUAL "fftw3f")
	find_package(FFTW3)
endif()

if(APPLE)
	if(NOT FFT_LIB OR FFT_LIB STREQUAL "vdsp")
		find_library(ACCELERATE_LIBRARIES Accelerate)
	endif()
endif()

if(NOT FFT_LIB OR FFT_LIB STREQUAL "kissfft")
	find_package(KissFFT)
endif()

set(USE_VDSP OFF)
set(USE_AVFFT OFF)
set(USE_FFTW3 OFF)
set(USE_FFTW3F OFF)
set(USE_KISSFFT OFF)

if(NOT FFT_LIB)
	if(APPLE AND ACCELERATE_LIBRARIES)
		set(FFT_LIB "vdsp")
	elseif(FFMPEG_LIBAVCODEC_FFT_FOUND)
		set(FFT_LIB "avfft")
	elseif(FFTW3_LIBRARIES)
		set(FFT_LIB "fftw3")
	elseif(FFTW3_FFTWF_LIBRARY)
		set(FFT_LIB "fftw3f")
	elseif(KISSFFT_FOUND)
		set(FFT_LIB "kissfft")
	endif()
endif()

if(FFT_LIB STREQUAL "vdsp")
	if(ACCELERATE_LIBRARIES)
		set(USE_VDSP ON)
	else()
		message(FATAL_ERROR "Selected ${FFT_LIB} for FFT calculations, but the library is not found")
	endif()
elseif(FFT_LIB STREQUAL "avfft")
	if(FFMPEG_LIBAVCODEC_FFT_FOUND)
		set(USE_AVFFT ON)
	else()
		message(FATAL_ERROR "Selected ${FFT_LIB} for FFT calculations, but the library is not found")
	endif()
elseif(FFT_LIB STREQUAL "fftw3")
	if(FFTW3_LIBRARIES)
		set(USE_FFTW3 ON)
	else()
		message(FATAL_ERROR "Selected ${FFT_LIB} for FFT calculations, but the library is not found")
	endif()
elseif(FFT_LIB STREQUAL "fftw3f")
	if(FFTW3_FFTWF_LIBRARY)
		set(USE_FFTW3F ON)
	else()
		message(FATAL_ERROR "Selected ${FFT_LIB} for FFT calculations, but the library is not found")
	endif()
elseif(FFT_LIB STREQUAL "kissfft")
	if(KISSFFT_FOUND)
		set(USE_KISSFFT ON)
	else()
		message(FATAL_ERROR "Selected ${FFT_LIB} for FFT calculations, but the library is not found")
	endif()
else()
	message(FATAL_ERROR "No FFT library found")
endif()

message(STATUS "Using ${FFT_LIB} for FFT calculations")

if(NOT AUDIO_PROCESSOR_LIB)
	if(FFMPEG_LIBSWRESAMPLE_FOUND)
		set(AUDIO_PROCESSOR_LIB "swresample")
	elseif(FFMPEG_LIBAVRESAMPLE_FOUND)
		set(AUDIO_PROCESSOR_LIB "avresample")
	endif()
endif()

if(AUDIO_PROCESSOR_LIB STREQUAL "swresample")
	if(FFMPEG_LIBSWRESAMPLE_FOUND)
		set(USE_AVRESAMPLE OFF)
		set(USE_SWRESAMPLE ON)
		set(AUDIO_PROCESSOR_LIBRARIES ${FFMPEG_LIBSWRESAMPLE_LIBRARIES})
		set(AUDIO_PROCESSOR_INCLUDE_DIRS ${FFMPEG_LIBSWRESAMPLE_INCLUDE_DIRS})
	else()
		message(FATAL_ERROR "Selected ${AUDIO_PROCESSOR_LIB} for audio processing, but the library is not found")
	endif()
	message(STATUS "Using ${AUDIO_PROCESSOR_LIB} for audio conversion")
elseif(AUDIO_PROCESSOR_LIB STREQUAL "avresample")
	if(FFMPEG_LIBAVRESAMPLE_FOUND)
		set(USE_AVRESAMPLE ON)
		set(USE_SWRESAMPLE OFF)
		set(AUDIO_PROCESSOR_LIBRARIES ${FFMPEG_LIBAVRESAMPLE_LIBRARIES})
		set(AUDIO_PROCESSOR_INCLUDE_DIRS ${FFMPEG_LIBAVRESAMPLE_INCLUDE_DIRS})
	else()
		message(FATAL_ERROR "Selected ${AUDIO_PROCESSOR_LIB} for audio processing, but the library is not found")
	endif()
	message(STATUS "Using ${AUDIO_PROCESSOR_LIB} for audio conversion")
else()
	message(STATUS "Building without audio conversion support, please install FFmpeg with libswresample")
endif()

if(NOT BUILD_FRAMEWORK)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libchromaprint.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libchromaprint.pc)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libchromaprint.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(src)

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

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
add_custom_target(docs doxygen)
