find_package(FFmpeg REQUIRED)

set(EXTRA_LIBS)
set(fpcalc_LINK_FLAGS)

if(APPLE)
    set(EXTRA_LIBS ${EXTRA_LIBS} -lz)
	set(fpcalc_LINK_FLAGS "${fpcalc_LINK_FLAGS} -sectcreate __TEXT __info_plist ${CMAKE_CURRENT_BINARY_DIR}/fpcalc.Info.plist")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fpcalc.Info.plist.in ${CMAKE_CURRENT_BINARY_DIR}/fpcalc.Info.plist)
endif()

if(UNIX)
    set(EXTRA_LIBS ${EXTRA_LIBS} -pthread -lm)
endif()

set(CMAKE_REQUIRED_LIBRARIES
	${FFMPEG_LIBAVFORMAT_LIBRARIES}
	${FFMPEG_LIBAVCODEC_LIBRARIES}
	${FFMPEG_LIBAVUTIL_LIBRARIES}
	${EXTRA_LIBS})

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../src
	${FFMPEG_LIBAVCODEC_INCLUDE_DIRS}
	${FFMPEG_LIBAVFORMAT_INCLUDE_DIRS}
	${FFMPEG_LIBAVUTIL_INCLUDE_DIRS}
)

set(fpcalc_LIBS
	chromaprint
	${FFMPEG_LIBAVFORMAT_LIBRARIES}
	${FFMPEG_LIBAVCODEC_LIBRARIES}
	${FFMPEG_LIBAVUTIL_LIBRARIES}
	${EXTRA_LIBS}
)

if(FFMPEG_LIBSWRESAMPLE_FOUND)
	add_definitions(-DHAVE_SWRESAMPLE)
	include_directories(${FFMPEG_LIBSWRESAMPLE_INCLUDE_DIRS})
	set(fpcalc_LIBS ${fpcalc_LIBS} ${FFMPEG_LIBSWRESAMPLE_LIBRARIES})
elseif(FFMPEG_LIBAVRESAMPLE_FOUND)
	add_definitions(-DHAVE_AVRESAMPLE)
	include_directories(${FFMPEG_LIBAVRESAMPLE_INCLUDE_DIRS})
	set(fpcalc_LIBS ${fpcalc_LIBS} ${FFMPEG_LIBAVRESAMPLE_LIBRARIES})
else()
	message(STATUS "Building without audio conversion support, please install FFmpeg with libswresample")
endif()

add_executable(fpcalc fpcalc.cpp)
target_link_libraries(fpcalc ${fpcalc_LIBS})
if(fpcalc_LINK_FLAGS)
	set_target_properties(fpcalc PROPERTIES LINK_FLAGS ${fpcalc_LINK_FLAGS})
endif()
install(TARGETS fpcalc RUNTIME DESTINATION ${BIN_INSTALL_DIR})
