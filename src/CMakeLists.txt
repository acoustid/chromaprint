include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(
	-D__STDC_LIMIT_MACROS
	-D__STDC_CONSTANT_MACROS
)

set(chromaprint_SRCS
	audio_processor.cpp
	chroma.cpp
	chroma_resampler.cpp
	chroma_filter.cpp
	integral_image.cpp
	spectral_centroid.cpp
	filter.cpp
	fft.cpp
	fingerprinter.cpp
	image_builder.cpp
	lloyds.cpp
	silence_remover.cpp
	fingerprint_calculator.cpp
	fingerprint_compressor.cpp
	avresample/resample2.c
)

if(WITH_AVFFT)
	set(chromaprint_SRCS fft_lib_avfft.cpp ${chromaprint_SRCS})
	set(chromaprint_LINK_LIBS
		${FFMPEG_LIBAVCODEC_LIBRARIES}
		${FFMPEG_LIBAVUTIL_LIBRARIES}
	)
	include_directories(
		${FFMPEG_LIBAVCODEC_INCLUDE_DIRS}
		${FFMPEG_LIBAVFORMAT_INCLUDE_DIRS}
		${FFMPEG_LIBAVUTIL_INCLUDE_DIRS}
	)
endif(WITH_AVFFT)

if(WITH_FFTW3)
	set(chromaprint_SRCS fft_lib_fftw3.cpp ${chromaprint_SRCS})
	set(chromaprint_LINK_LIBS ${FFTW3_LIBRARIES})
	include_directories(${FFTW3_INCLUDE_DIR})
endif(WITH_FFTW3)

set(chromaprintext_SRCS
#	sox_audio_source.cpp
#	ext/image_utils.cpp
	foo.cpp
)

add_library(chromaprint SHARED ${chromaprint_SRCS})
target_link_libraries(chromaprint ${chromaprint_LINK_LIBS})

add_library(chromaprintext STATIC ${chromaprintext_SRCS})
target_link_libraries(chromaprintext
	${FFMPEG_LIBAVFORMAT_LIBRARIES}
	${FFMPEG_LIBAVCODEC_LIBRARIES}
	${FFMPEG_LIBAVUTIL_LIBRARIES}
	${PNG_LIBRARIES}
#	-lsox
)

