// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_AUDIO_FFMPEG_AUDIO_READER_H_
#define CHROMAPRINT_AUDIO_FFMPEG_AUDIO_READER_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "debug.h"
#include "utils/scope_exit.h"
#include <cstdlib>
#include <string>
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

#include "audio/ffmpeg_compat.h"
#include "audio/ffmpeg_audio_processor.h"

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

	void SetOutputSampleRate(int sample_rate) { m_output_sample_rate = sample_rate; }
	void SetOutputChannels(int channels) { m_output_channels = channels; }

	bool Open(const std::string &file_name);

	void Close();

	bool Read(const int16_t **data, size_t *size);

	bool IsOpen() const { return m_opened; }
	bool IsFinished() const { return !m_has_more_packets && !m_has_more_frames; }

	std::string GetError() const { return m_error; }
	int GetErrorCode() const { return m_error_code; }

private:
	inline void SetError(const char *format, int errnum = 0);
	inline bool ConverterMatchesFrame(const AVFrame *frame) const;
	inline int ResetConverter(const AVFrame *frame);
	inline bool EnsureConvertBuffer(int nb_samples);

	std::unique_ptr<FFmpegAudioProcessor> m_converter;
#if CHROMAPRINT_HAVE_AV_CHANNEL_LAYOUT
	AVChannelLayout m_converter_channel_layout = {};
#else
	uint64_t m_converter_channel_layout = 0;
#endif
	int m_converter_sample_format = AV_SAMPLE_FMT_NONE;
	int m_converter_sample_rate = 0;
	uint8_t *m_convert_buffer[1] = { nullptr };
	int m_convert_buffer_nb_samples = 0;

	CHROMAPRINT_CONST_AVFORMAT AVInputFormat *m_input_fmt = nullptr;
	AVDictionary *m_input_opts = nullptr;

	AVFormatContext *m_format_ctx = nullptr;
	AVCodecContext *m_codec_ctx = nullptr;
	int m_stream_index = -1;
	std::string m_error;
	int m_error_code = 0;
	bool m_opened = false;
	bool m_has_more_packets = true;
	bool m_has_more_frames = true;
	AVPacket *m_packet = nullptr;
	AVFrame *m_frame = nullptr;

	int m_output_sample_rate = 0;
	int m_output_channels = 0;

	uint64_t m_nb_packets = 0;
	int m_decode_error = 0;
};

inline FFmpegAudioReader::FFmpegAudioReader() {
	av_log_set_level(AV_LOG_QUIET);
}

inline FFmpegAudioReader::~FFmpegAudioReader() {
	Close();
	av_dict_free(&m_input_opts);
	av_freep(&m_convert_buffer[0]);
#if CHROMAPRINT_HAVE_AV_CHANNEL_LAYOUT
	av_channel_layout_uninit(&m_converter_channel_layout);
#endif
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
#if CHROMAPRINT_HAVE_AV_CHANNEL_LAYOUT
    if (channels == 1)
        sprintf(buf, "%s", "mono");
    else if (channels == 2)
        sprintf(buf, "%s", "stereo");
    else
        sprintf(buf, "%d channels", channels);
    return av_dict_set(&m_input_opts, "ch_layout", buf, 0) >= 0;
#else
    // Before 5.1 the demuxers take a channel count, not a layout description.
    sprintf(buf, "%d", channels);
    return av_dict_set(&m_input_opts, "channels", buf, 0) >= 0;
#endif
}

inline bool FFmpegAudioReader::Open(const std::string &file_name) {
	int ret;

	Close();

	m_packet = av_packet_alloc();
	if (!m_packet) {
		return false;
	}

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

	CHROMAPRINT_CONST_AVFORMAT AVCodec *codec;
	ret = av_find_best_stream(m_format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
	if (ret < 0) {
		SetError("Could not find any audio stream in the file", ret);
		return false;
	}
	m_stream_index = ret;
	auto stream = m_format_ctx->streams[m_stream_index];

	m_codec_ctx = avcodec_alloc_context3(codec);
	m_codec_ctx->request_sample_fmt = AV_SAMPLE_FMT_S16;

	ret = avcodec_parameters_to_context(m_codec_ctx, stream->codecpar);
	if (ret < 0) {
		SetError("Could not copy the stream parameters", ret);
		return false;
	}

	ret = avcodec_open2(m_codec_ctx, codec, nullptr);
	if (ret < 0) {
		SetError("Could not open the codec", ret);
		return false;
	}

	av_dump_format(m_format_ctx, 0, "foo", 0);

	m_frame = av_frame_alloc();
	if (!m_frame) {
		return false;
	}

	if (!m_output_sample_rate) {
		m_output_sample_rate = m_codec_ctx->sample_rate;
	}

	if (!m_output_channels) {
		m_output_channels = CHROMAPRINT_CODEC_CHANNELS(m_codec_ctx);
	}

	// The converter is set up in Read() from the decoded frames, because
	// some decoders (e.g. AC-3) change the channel layout mid-stream.
	m_converter.reset();

	m_opened = true;
	m_has_more_packets = true;
	m_has_more_frames = true;
	m_decode_error = 0;

	return true;
}

inline void FFmpegAudioReader::Close() {
	av_frame_free(&m_frame);
	av_packet_free(&m_packet);

	m_stream_index = -1;

	avcodec_free_context(&m_codec_ctx);

	if (m_format_ctx) {
		avformat_close_input(&m_format_ctx);
	}
}

inline int FFmpegAudioReader::GetSampleRate() const {
	return m_output_sample_rate;
}

inline int FFmpegAudioReader::GetChannels() const {
	return m_output_channels;
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

	*data = nullptr;
	*size = 0;

	int ret;
	bool needs_packet = false;
	while (true) {
		while (needs_packet && m_packet->size == 0) {
			ret = av_read_frame(m_format_ctx, m_packet);
			if (ret < 0) {
				if (ret == AVERROR_EOF) {
					needs_packet = false;
					m_has_more_packets = false;
					break;
				}
				SetError("Error reading from the audio source", ret);
				return false;
			}
			if (m_packet->stream_index == m_stream_index) {
				needs_packet = false;
			} else {
				av_packet_unref(m_packet);
			}
		}

		if (m_packet->size != 0) {
			ret = avcodec_send_packet(m_codec_ctx, m_packet);
			if (ret < 0) {
				if (ret != AVERROR(EAGAIN)) {
					SetError("Error reading from the audio source", ret);
					return false;
				}
			} else {
				av_packet_unref(m_packet);
			}
		}

		ret = avcodec_receive_frame(m_codec_ctx, m_frame);
		if (ret < 0) {
			if (ret == AVERROR_EOF) {
				m_has_more_frames = false;
			} else if (ret == AVERROR(EAGAIN)) {
				if (m_has_more_packets) {
					needs_packet = true;
					continue;
				} else {
					m_has_more_frames = false;
				}
			} else {
				SetError("Error decoding the audio source", ret);
				return false;
			}
		}

		if (m_frame->nb_samples > 0) {
			int nb_flushed = 0;
			if (!ConverterMatchesFrame(m_frame)) {
				if (!EnsureConvertBuffer(m_frame->nb_samples)) {
					return false;
				}
				// The input format changed, drain what the old converter still
				// holds before replacing it.
				if (m_converter) {
					nb_flushed = m_converter->Flush(m_convert_buffer, m_convert_buffer_nb_samples);
					if (nb_flushed < 0) {
						SetError("Couldn't convert audio", nb_flushed);
						return false;
					}
				}
				ret = ResetConverter(m_frame);
				if (ret < 0) {
					SetError("Could not create an audio converter instance", ret);
					return false;
				}
			}
			if (m_converter) {
				if (!EnsureConvertBuffer(m_frame->nb_samples)) {
					return false;
				}
				uint8_t *out[1] = { m_convert_buffer[0] + nb_flushed * m_output_channels * sizeof(int16_t) };
				auto nb_samples = m_converter->Convert(out, m_convert_buffer_nb_samples - nb_flushed, (const uint8_t **) m_frame->extended_data, m_frame->nb_samples);
				if (nb_samples < 0) {
					SetError("Couldn't convert audio", nb_samples);
					return false;
				}
				*data = (const int16_t *) m_convert_buffer[0];
				*size = nb_flushed + nb_samples;
			} else {
				*data = (const int16_t *) m_frame->data[0];
				*size = m_frame->nb_samples;
			}
		} else {
			if (m_converter) {
				if (IsFinished()) {
					auto nb_samples = m_converter->Flush(m_convert_buffer, m_convert_buffer_nb_samples);
					if (nb_samples < 0) {
						SetError("Couldn't convert audio", ret);
						return false;
					} else if (nb_samples > 0) {
						*data = (const int16_t *) m_convert_buffer[0];
						*size = nb_samples;
					}
				}
			}
		}

		return true;
	}
}

inline bool FFmpegAudioReader::EnsureConvertBuffer(int nb_samples) {
	// Leave room for samples flushed from a previous converter and for upsampling.
	nb_samples = std::max(1024 * 8, nb_samples * 4);
	if (nb_samples > m_convert_buffer_nb_samples) {
		int linesize;
		av_freep(&m_convert_buffer[0]);
		m_convert_buffer_nb_samples = 0;
		int ret = av_samples_alloc(m_convert_buffer, &linesize, m_output_channels, nb_samples, AV_SAMPLE_FMT_S16, 1);
		if (ret < 0) {
			SetError("Couldn't allocate audio converter buffer", ret);
			return false;
		}
		m_convert_buffer_nb_samples = nb_samples;
	}
	return true;
}

inline bool FFmpegAudioReader::ConverterMatchesFrame(const AVFrame *frame) const {
	if (!m_converter) {
		return frame->format == AV_SAMPLE_FMT_S16 &&
			frame->sample_rate == m_output_sample_rate &&
			CHROMAPRINT_FRAME_CHANNELS(frame) == m_output_channels;
	}
	if (frame->format != m_converter_sample_format || frame->sample_rate != m_converter_sample_rate) {
		return false;
	}
#if CHROMAPRINT_HAVE_AV_CHANNEL_LAYOUT
	return av_channel_layout_compare(&frame->ch_layout, &m_converter_channel_layout) == 0;
#else
	return (frame->channel_layout ? frame->channel_layout : av_get_default_channel_layout(frame->channels)) == m_converter_channel_layout;
#endif
}

inline int FFmpegAudioReader::ResetConverter(const AVFrame *frame) {
	int ret;

	m_converter.reset();
	m_converter_sample_format = frame->format;
	m_converter_sample_rate = frame->sample_rate;
#if CHROMAPRINT_HAVE_AV_CHANNEL_LAYOUT
	ret = av_channel_layout_copy(&m_converter_channel_layout, &frame->ch_layout);
	if (ret < 0) {
		return ret;
	}
	AVChannelLayout output_channel_layout;
	av_channel_layout_default(&output_channel_layout, m_output_channels);
#else
	m_converter_channel_layout = frame->channel_layout
		? frame->channel_layout
		: av_get_default_channel_layout(frame->channels);
	uint64_t output_channel_layout = av_get_default_channel_layout(m_output_channels);
#endif

	m_converter.reset(new FFmpegAudioProcessor());
	m_converter->SetCompatibleMode();
	m_converter->SetInputSampleFormat((AVSampleFormat) frame->format);
	m_converter->SetInputSampleRate(frame->sample_rate);
#if CHROMAPRINT_HAVE_AV_CHANNEL_LAYOUT
	m_converter->SetInputChannelLayout(&m_converter_channel_layout);
	m_converter->SetOutputChannelLayout(&output_channel_layout);
#else
	m_converter->SetInputChannelLayout(m_converter_channel_layout);
	m_converter->SetOutputChannelLayout(output_channel_layout);
#endif
	m_converter->SetOutputSampleFormat(AV_SAMPLE_FMT_S16);
	m_converter->SetOutputSampleRate(m_output_sample_rate);

#if CHROMAPRINT_HAVE_AV_CHANNEL_LAYOUT
	av_channel_layout_uninit(&output_channel_layout);
#endif

	ret = m_converter->Init();
	if (ret < 0) {
		m_converter.reset();
	}
	return ret;
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
	m_error_code = errnum;
}

}; // namespace chromaprint

#endif
