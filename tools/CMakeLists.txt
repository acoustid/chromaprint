include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../src
	${FFMPEG_LIBAVCODEC_INCLUDE_DIRS}
	${FFMPEG_LIBAVFORMAT_INCLUDE_DIRS}
	${FFMPEG_LIBAVUTIL_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${TAGLIB_INCLUDES}
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../src
	${FFMPE}
	${TAGLIB_INCLUDES}
)

add_definitions(
	-D__STDC_LIMIT_MACROS
	-D__STDC_CONSTANT_MACROS
)

if(BUILD_EXTRA_TOOLS)

	add_executable(resample resample.cpp)
	target_link_libraries(resample chromaprint chromaprintext)

	add_executable(decode decode.cpp)
	target_link_libraries(decode chromaprint chromaprintext)

	add_executable(chromagram chromagram.cpp)
	target_link_libraries(chromagram chromaprint chromaprintext -lpng)

	add_executable(learn_filters learn_filters.cpp)
	target_link_libraries(learn_filters chromaprint chromaprintext
		${Boost_SYSTEM_LIBRARY}
		${Boost_FILESYSTEM_LIBRARY})

	add_executable(fpeval fpeval.cpp)
	target_link_libraries(fpeval chromaprint chromaprintext
		${Boost_SYSTEM_LIBRARY}
		${Boost_FILESYSTEM_LIBRARY})

endif(BUILD_EXTRA_TOOLS)

add_executable(fpgen fpgen.cpp)
target_link_libraries(fpgen chromaprint chromaprintext)

add_executable(fpcollect fpcollect.cpp)
target_link_libraries(fpcollect chromaprint chromaprintext
	${TAGLIB_LIBRARIES})

