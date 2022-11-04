// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_AUDIO_FFMPEG_AUDIO_PROCESSOR_H_
#define CHROMAPRINT_AUDIO_FFMPEG_AUDIO_PROCESSOR_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(USE_SWRESAMPLE)
#include "audio/ffmpeg_audio_processor_swresample.h"
#else
#error "no audio processing library"
#endif

#endif
