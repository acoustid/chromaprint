find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_library(gtest STATIC ${GTEST_SOURCES})
target_include_directories(gtest INTERFACE ${GTEST_INCLUDE_DIRS})
target_include_directories(gtest PRIVATE ${GTEST_INCLUDE_DIRS} ${GTEST_SOURCE_DIR})
target_link_libraries(gtest Threads::Threads)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

set(SRCS
	main.cpp
	test_api.cpp
	test_utils.cpp
	test_quantizer.cpp
	test_filter.cpp
	test_filter_utils.cpp
	test_audio_processor.cpp
	test_simhash.cpp
	test_chromaprint.cpp
	test_chroma.cpp
	test_chroma_filter.cpp
	test_chroma_resampler.cpp
	test_fingerprint_compressor.cpp
	test_fingerprint_compressor_v2.cpp
	test_fingerprint_decompressor.cpp
	test_fingerprint_matcher.cpp
	test_silence_remover.cpp
	test_moving_average.cpp
	test_utils_gradient.cpp
	test_utils_gaussian_filter.cpp
	../src/fft_test.cpp
	../src/audio/audio_slicer_test.cpp
	../src/utils/base64_test.cpp
	../src/utils/rolling_integral_image_test.cpp
)

if(BUILD_TOOLS)
	set(SRCS ${SRCS} ../src/audio/ffmpeg_audio_reader_test.cpp)
    include_directories(
        ${FFMPEG_LIBAVFORMAT_INCLUDE_DIRS}
        ${FFMPEG_LIBAVCODEC_INCLUDE_DIRS}
        ${FFMPEG_LIBAVUTIL_INCLUDE_DIRS}
        ${AUDIO_PROCESSOR_INCLUDE_DIRS}
    )
	link_libraries(fpcalc_libs)
endif()

add_executable(all_tests ${SRCS} $<TARGET_OBJECTS:chromaprint_objs>)
target_link_libraries(all_tests PRIVATE chromaprint gtest)

add_custom_target(check ${CMAKE_CURRENT_BINARY_DIR}/all_tests $ENV{GTEST_FLAGS} DEPENDS all_tests)

enable_testing(true)
add_test(ChromaprintTests all_tests)
