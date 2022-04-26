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
	bool IsFinished() const { return m_finished && !m_got_frame; }

	std::string GetError() const { return m_error; }
	int GetErrorCode() const { return m_error_code; }

private:
	inline void SetError(const char *format, int errnum = 0);

	std::unique_ptr<FFmpegAudioProcessor> m_converter;
	uint8_t *m_convert_buffer[1] = { nullptr };
	int m_convert_buffer_nb_samples = 0;

	const AVInputFormat *m_input_fmt = nullptr;
	AVDictionary *m_input_opts = nullptr;

	AVFormatContext *m_format_ctx = nullptr;
	AVCodecContext *m_codec_ctx = nullptr;
	AVFrame *m_frame = nullptr;
	int m_stream_index = -1;
	std::string m_error;
	int m_error_code = 0;
	bool m_finished = false;
	bool m_opened = false;
    bool m_processing_frame = false;
	int m_got_frame = 0;

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

	ret = avformat_open_input(&m_format_ctx, file_name.c_str(), m_input_fmt, &m_input_opts);
	if (ret < 0) {
		SetError("Could not open the input file", ret);
		return false;
	}

	ret = avformat_find_stream_info(m_format_ctx, nullptr);
	if (ret < 0) {
		SetError("Could not find stream information in the file", ret);
		return false;
	}

	const AVCodec *codec;
    ret = av_find_best_stream(m_format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
	if (ret < 0) {
		SetError("Could not find any audio stream in the file", ret);
		return false;
	}
	m_stream_index = ret;

    const AVCodec *stream_codec = avcodec_find_decoder(m_format_ctx->streams[m_stream_index]->codecpar->codec_id);
    m_codec_ctx = avcodec_alloc_context3(stream_codec);
    avcodec_parameters_to_context(m_codec_ctx, m_format_ctx->streams[m_stream_index]->codecpar);
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

	if (!m_output_sample_rate) {
		m_output_sample_rate = m_codec_ctx->sample_rate;
	}

	if (!m_output_channels) {
		m_output_channels = m_codec_ctx->channels;
	}

	if (m_codec_ctx->sample_fmt != AV_SAMPLE_FMT_S16 || m_codec_ctx->channels != m_output_channels || m_codec_ctx->sample_rate != m_output_sample_rate) {
		m_converter.reset(new FFmpegAudioProcessor());
		m_converter->SetCompatibleMode();
		m_converter->SetInputSampleFormat(m_codec_ctx->sample_fmt);
		m_converter->SetInputSampleRate(m_codec_ctx->sample_rate);
		m_converter->SetInputChannelLayout(m_codec_ctx->channel_layout);
		m_converter->SetOutputSampleFormat(AV_SAMPLE_FMT_S16);
		m_converter->SetOutputSampleRate(m_output_sample_rate);
		m_converter->SetOutputChannelLayout(av_get_default_channel_layout(m_output_channels));
		auto ret = m_converter->Init();
		if (ret != 0) {
			SetError("Could not create an audio converter instance", ret);
			return false;
		}
	}

	m_opened = true;
	m_finished = false;
    m_processing_frame = false;
	m_got_frame = 0;
	m_nb_packets = 0;
	m_decode_error = 0;

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

	int ret;
	if (!m_processing_frame) {
		AVPacket *m_packet = av_packet_alloc();
	
		// read frame to packet, set m_finished to true on EOF
		ret = av_read_frame(m_format_ctx, m_packet);
		if (ret == AVERROR_EOF) {
			m_finished = true;
		} else if (ret < 0) {
			SetError("Error reading from the audio source", ret);
			return false;
		}
		if (m_packet->stream_index == m_stream_index) {
			m_nb_packets++;
	
			// decode packet
			ret = avcodec_send_packet(m_codec_ctx, m_packet);
			if (ret < 0) {
				SetError("Error sending a packet to the decoder", ret);
				return false;
			}
		}
	}

	// read decoded frames
	ret = avcodec_receive_frame(m_codec_ctx, m_frame);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
		if (m_finished) {
			// no more frames to read
			m_got_frame = 0;
			// flush converter
			if (m_converter) {
				auto nb_samples = m_converter->Flush(m_convert_buffer, m_convert_buffer_nb_samples);
				if (nb_samples < 0) {
					SetError("Couldn't convert audio", ret);
					return false;
				} else if (nb_samples > 0) {
					*data = (const int16_t *) m_convert_buffer[0];
					*size = nb_samples;
				}
			}
			return true;
		} else {
			// the whole packet was consumed
			m_processing_frame = false;
			return true;
		}
	} else if (ret < 0) {
		SetError("Error during decoding", ret);
		return false;
	}

	m_processing_frame = true;
	m_got_frame = 1;
	if (m_converter) {
		if (m_frame->nb_samples > m_convert_buffer_nb_samples) {
			int line_size;
			av_freep(&m_convert_buffer[0]);
			m_convert_buffer_nb_samples = std::max(1024 * 8, m_frame->nb_samples);
			ret = av_samples_alloc(m_convert_buffer, &line_size, m_codec_ctx->channels, m_convert_buffer_nb_samples, AV_SAMPLE_FMT_S16, 1);
			if (ret < 0) {
				SetError("Couldn't allocate audio converter buffer", ret);
				return false;
			}
		}
		auto nb_samples = m_converter->Convert(m_convert_buffer, m_convert_buffer_nb_samples, (const uint8_t **) m_frame->data, m_frame->nb_samples);
		if (nb_samples < 0) {
			SetError("Couldn't convert audio", ret);
			return false;
		}
		*data = (const int16_t *) m_convert_buffer[0];
		*size = nb_samples;
	} else {
		*data = (const int16_t *) m_frame->data[0];
		*size = m_frame->nb_samples;
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
	m_error_code = errnum;
}

}; // namespace chromaprint

#endif
