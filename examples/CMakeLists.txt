set(EXTRA_LIBS)
if(APPLE)
    set(EXTRA_LIBS ${EXTRA_LIBS} -lz)
endif()
if(UNIX)
    set(EXTRA_LIBS ${EXTRA_LIBS} -lpthread)
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

add_executable(fpcalc fpcalc.c)

target_link_libraries(fpcalc chromaprint
	${FFMPEG_LIBAVFORMAT_LIBRARIES}
	${FFMPEG_LIBAVCODEC_LIBRARIES}
	${FFMPEG_LIBAVUTIL_LIBRARIES}
	${EXTRA_LIBS})

if(FFMPEG_LIBSWRESAMPLE_FOUND)
	add_definitions(-DHAVE_SWRESAMPLE)
	include_directories(${FFMPEG_LIBSWRESAMPLE_INCLUDE_DIRS})
	target_link_libraries(fpcalc ${FFMPEG_LIBSWRESAMPLE_LIBRARIES})
elseif(FFMPEG_LIBAVRESAMPLE_FOUND)
	add_definitions(-DHAVE_AVRESAMPLE)
	include_directories(${FFMPEG_LIBAVRESAMPLE_INCLUDE_DIRS})
	target_link_libraries(fpcalc ${FFMPEG_LIBAVRESAMPLE_LIBRARIES})
else()
	message(STATUS "Building without audio conversion support, please install FFmpeg with libswresample")
endif()

install(TARGETS fpcalc
	RUNTIME DESTINATION ${BIN_INSTALL_DIR}
)

