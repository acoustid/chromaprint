include_directories(
	${GTEST_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../src
)
add_subdirectory(${GTEST_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/gtest_build)

set(tests_SOURCES
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

add_executable(all_tests ${tests_SOURCES})
set_target_properties(all_tests PROPERTIES
	COMPILE_DEFINITIONS CHROMAPRINT_NODLL)
target_link_libraries(all_tests
	${GTEST_BOTH_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	chromaprint_p
)

add_custom_target(check
	${CMAKE_CURRENT_BINARY_DIR}/all_tests
	$ENV{GTEST_FLAGS} DEPENDS all_tests)

enable_testing(true)
add_test(ChromaprintTests all_tests)

