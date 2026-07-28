// Copyright (C) 2026  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_AUDIO_FFMPEG_COMPAT_H_
#define CHROMAPRINT_AUDIO_FFMPEG_COMPAT_H_

extern "C" {
#include <libavformat/version.h>
#include <libavutil/channel_layout.h>
#include <libavutil/version.h>
}

// AVChannelLayout, and the API around it, arrived in libavutil 57.24.100
// (FFmpeg 5.1). Before that a layout is a plain bitmask and the channel count
// lives directly on AVCodecContext.
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 24, 100)
#define CHROMAPRINT_HAVE_AV_CHANNEL_LAYOUT 1
#else
#define CHROMAPRINT_HAVE_AV_CHANNEL_LAYOUT 0
#endif

#if CHROMAPRINT_HAVE_AV_CHANNEL_LAYOUT
#define CHROMAPRINT_CODEC_CHANNELS(ctx) ((ctx)->ch_layout.nb_channels)
#else
#define CHROMAPRINT_CODEC_CHANNELS(ctx) ((ctx)->channels)
#endif

// avformat_open_input() and av_find_best_stream() took non-const AVInputFormat
// and AVCodec pointers before libavformat 59 (FFmpeg 5.0).
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(59, 0, 100)
#define CHROMAPRINT_CONST_AVFORMAT const
#else
#define CHROMAPRINT_CONST_AVFORMAT
#endif

#endif
