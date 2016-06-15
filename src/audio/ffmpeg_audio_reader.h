// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_AUDIO_FFMPEG_AUDIO_READER_H_
#define CHROMAPRINT_AUDIO_FFMPEG_AUDIO_READER_H_

#include "debug.h"
#include "utils/scope_exit.h"
#include <cstdlib>
#include <string>

extern "C" {

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>

#if defined(HAVE_SWRESAMPLE)
#include <libswresample/swresample.h>
#elif defined(HAVE_AVRESAMPLE)
#include <libavresample/avresample.h>
#endif

}

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55, 28, 1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif

#ifndef AV_ERROR_MAX_STRING_SIZE
#define AV_ERROR_MAX_STRING_SIZE 128
#endif

namespace chromaprint {

class FFmpegAudioReader {
public:
	FFmpegAudioReader();
	~FFmpegAudioReader();

	/**
	 * Get the sample rate in the audio stream.
	 * @return sample rate in Hz, -1 on error
	 */
	int GetSampleRate() const;

	/**
	 * Get the number of channels in the audio stream.
	 * @return number of channels, -1 on error
	 */
	int GetChannels() const;

	/**
	 * Get the estimated audio stream duration.
	 * @return stream duration in milliseconds, -1 on error
	 */
	int GetDuration() const;

	bool SetInputFormat(const char *name);
	bool SetInputSampleRate(int sample_rate);
	bool SetInputChannels(int channels);

	bool Open(const std::string &file_name);

	void Close();

	bool Read(const int16_t **data, size_t *size);

	template <typename Func>
	inline bool ReadFrame(Func consumer);

	bool IsOpen() const { return m_opened; }
	bool IsFinished() const { return m_finished && !m_got_frame; }

	std::string GetError() const { return m_error; }

private:
	inline void SetError(const char *format, int errnum = 0);

	AVInputFormat *m_input_fmt = nullptr;
	AVDictionary *m_input_opts = nullptr;

	AVFormatContext *m_format_ctx = nullptr;
	AVCodecContext *m_codec_ctx = nullptr;
	AVFrame *m_frame = nullptr;
	int m_stream_index = -1;
	std::string m_error;
	bool m_finished = false;
	bool m_opened = false;
	int m_got_frame = 0;
	AVPacket m_packet;
	AVPacket m_packet0;
};

inline FFmpegAudioReader::FFmpegAudioReader() {
	av_log_set_level(AV_LOG_QUIET);
	av_register_all();
}

inline FFmpegAudioReader::~FFmpegAudioReader() {
	Close();
	av_dict_free(&m_input_opts);
}

inline bool FFmpegAudioReader::SetInputFormat(const char *name) {
	m_input_fmt = av_find_input_format(name);
	return m_input_fmt;
}

inline bool FFmpegAudioReader::SetInputSampleRate(int sample_rate) {
	char buf[64];
	sprintf(buf, "%d", sample_rate);
	return av_dict_set(&m_input_opts, "sample_rate", buf, 0) >= 0;
}

inline bool FFmpegAudioReader::SetInputChannels(int channels) {
	char buf[64];
	sprintf(buf, "%d", channels);
	return av_dict_set(&m_input_opts, "channels", buf, 0) >= 0;
}

inline bool FFmpegAudioReader::Open(const std::string &file_name) {
	int ret;

	Close();

    av_init_packet(&m_packet);
	m_packet.data = nullptr;
	m_packet.size = 0;

	m_packet0 = m_packet;

	ret = avformat_open_input(&m_format_ctx, file_name.c_str(), m_input_fmt, &m_input_opts);
	if (ret < 0) {
		SetError("Could not open the input file", ret);
		return false;
	}

	ret = avformat_find_stream_info(m_format_ctx, nullptr);
	if (ret < 0) {
		SetError("Coud not find stream information in the file", ret);
		return false;
	}

	AVCodec *codec;
	ret = av_find_best_stream(m_format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
	if (ret < 0) {
		SetError("Could not find any audio stream in the file", ret);
		return false;
	}
	m_stream_index = ret;

	m_codec_ctx = m_format_ctx->streams[m_stream_index]->codec;
	m_codec_ctx->request_sample_fmt = AV_SAMPLE_FMT_S16;

	ret = avcodec_open2(m_codec_ctx, codec, nullptr);
	if (ret < 0) {
		SetError("Could not open the codec", ret);
		return false;
	}

	if (!m_codec_ctx->channel_layout) {
		m_codec_ctx->channel_layout = av_get_default_channel_layout(m_codec_ctx->channels);
	}

	m_frame = av_frame_alloc();
	if (!m_frame) {
		return false;
	}

	if (m_codec_ctx->sample_fmt != AV_SAMPLE_FMT_S16) {
		SetError("Unsupported sample format");
		return false;
	}

	m_opened = true;
	m_finished = false;
	m_got_frame = 0;

	return true;
}

inline void FFmpegAudioReader::Close() {
	av_frame_free(&m_frame);
	m_stream_index = -1;
	if (m_codec_ctx) {
		avcodec_close(m_codec_ctx);
		m_codec_ctx = nullptr;
	}
	if (m_format_ctx) {
		avformat_close_input(&m_format_ctx);
	}
}

inline int FFmpegAudioReader::GetSampleRate() const {
	if (m_codec_ctx) {
		return m_codec_ctx->sample_rate;
	}
	return -1;
}

inline int FFmpegAudioReader::GetChannels() const {
	if (m_codec_ctx) {
		return m_codec_ctx->channels;
	}
	return -1;
}

inline int FFmpegAudioReader::GetDuration() const {
	if (m_format_ctx && m_stream_index >= 0) {
		const auto stream = m_format_ctx->streams[m_stream_index];
		if (stream->duration != AV_NOPTS_VALUE) {
			return 1000 * stream->time_base.num * stream->duration / stream->time_base.den;
		} else if (m_format_ctx->duration != AV_NOPTS_VALUE) {
			return 1000 * m_format_ctx->duration / AV_TIME_BASE;
		}
	}
	return -1;
}

inline bool FFmpegAudioReader::Read(const int16_t **data, size_t *size) {
	if (!IsOpen() || IsFinished()) {
		return false;
	}

	while (m_packet.size <= 0) {
		av_free_packet(&m_packet0);
		av_init_packet(&m_packet);
		m_packet.data = nullptr;
		m_packet.size = 0;
		int ret = av_read_frame(m_format_ctx, &m_packet);
		if (ret < 0) {
			if (ret == AVERROR_EOF) {
				m_finished = true;
				break;
			} else {
				SetError("Error reading from the audio source", ret);
				return false;
			}
		}
		m_packet0 = m_packet;
		if (m_packet.stream_index != m_stream_index) {
			m_packet.data = nullptr;
			m_packet.size = 0;
		}
	}

	int ret = avcodec_decode_audio4(m_codec_ctx, m_frame, &m_got_frame, &m_packet);
	if (ret < 0) {
		SetError("Error decoding audio frame", ret);
		return false;
	}

	const int decoded = std::min(ret, m_packet.size);
	m_packet.data += decoded;
	m_packet.size -= decoded;

	if (m_got_frame) {
		*data = (const int16_t *) m_frame->data[0];
		*size = m_frame->nb_samples;
	}

	return true;
}

template <typename Func>
inline bool FFmpegAudioReader::ReadFrame(Func consumer) {
	if (m_finished) {
		return true;
	}

	int ret;
	AVPacket packet0, packet;

    av_init_packet(&packet);
    packet.data = nullptr;
    packet.size = 0;

	ret = av_read_frame(m_format_ctx, &packet);
	if (ret < 0) {
		if (ret == AVERROR_EOF) {
			m_finished = true;
		} else {
			SetError("Error reading from the audio source", ret);
			return false;
		}
	}

	packet0 = packet;
	SCOPE_EXIT(av_free_packet(&packet0));

	if (m_finished || packet.stream_index == m_stream_index) {
		while (true) {
			int got_frame = 1;
			ret = avcodec_decode_audio4(m_codec_ctx, m_frame, &got_frame, &packet);
			if (ret < 0) {
				SetError("Error decoding audio frame", ret);
				return false;
			}

			const int decoded = std::min(ret, packet.size);
			packet.data += decoded;
			packet.size -= decoded;

			if (got_frame) {
				consumer(m_frame->data, m_frame->nb_samples);
			} else {
				break;
			}
		}
	}

	return true;
}

inline void FFmpegAudioReader::SetError(const char *message, int errnum) {
	m_error = message;
	if (errnum < 0) {
		char buf[AV_ERROR_MAX_STRING_SIZE];
		if (av_strerror(errnum, buf, AV_ERROR_MAX_STRING_SIZE) == 0) {
			m_error += " (";
			m_error += buf;
			m_error += ")";
		}
	}
	DEBUG(m_error);
}

}; // namespace chromaprint

#endif
