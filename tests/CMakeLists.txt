include(GoogleTest)

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
mark_as_advanced(BUILD_GMOCK INSTALL_GTEST GTEST_HAS_ABSL)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/googletest" "test/3rdparty/googletest")

add_executable(all_tests 
  $<TARGET_OBJECTS:chromaprint_objs>
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

target_include_directories(all_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(all_tests PRIVATE chromaprint gtest gtest_main)

if(BUILD_TOOLS)
  target_sources(all_tests PRIVATE ../src/audio/ffmpeg_audio_reader_test.cpp)
  target_include_directories(all_tests
    PRIVATE
      ${FFMPEG_LIBAVFORMAT_INCLUDE_DIRS}
      ${FFMPEG_LIBAVCODEC_INCLUDE_DIRS}
      ${FFMPEG_LIBAVUTIL_INCLUDE_DIRS}
      ${AUDIO_PROCESSOR_INCLUDE_DIRS}
    )
  target_link_libraries(all_tests PRIVATE fpcalc_libs)
endif()

if(WIN32 AND BUILD_SHARED_LIBS AND CMAKE_VERSION VERSION_GREATER_EQUAL "3.21.0")
  add_custom_command(TARGET all_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:all_tests> $<TARGET_FILE_DIR:all_tests>
    COMMAND_EXPAND_LISTS
  )
endif()

gtest_discover_tests(all_tests
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  DISCOVERY_TIMEOUT 100
  PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

set_target_properties(all_tests PROPERTIES FOLDER tests)
