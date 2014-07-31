set(EXTRA_LIBS)
if(APPLE)
    set(EXTRA_LIBS ${EXTRA_LIBS} -lz)
    set(CMAKE_EXE_LINKER_FLAGS "-sectcreate __TEXT __info_plist ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist")
endif()
if(UNIX)
    set(EXTRA_LIBS ${EXTRA_LIBS} -lpthread -lm)
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
	${FFMPEG_LIBAVCODEC_LIBRARIES})

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

set(fpcalc_LIBS ${fpcalc_LIBS}
	${FFMPEG_LIBAVUTIL_LIBRARIES}
	${EXTRA_LIBS})

add_executable(fpcalc fpcalc.c)

target_link_libraries(fpcalc ${fpcalc_LIBS})

install(TARGETS fpcalc
	RUNTIME DESTINATION ${BIN_INSTALL_DIR}
)

