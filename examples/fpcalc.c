#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <chromaprint.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

int decode_audio_file(ChromaprintContext *chromaprint_ctx, const char *file_name, int max_length, int *duration)
{
	int ok = 0, remaining, length, consumed, codec_ctx_opened = 0, got_frame, stream_index;
	AVFormatContext *format_ctx = NULL;
	AVCodecContext *codec_ctx = NULL;
	AVCodec *codec = NULL;
	AVStream *stream = NULL;
	AVFrame *frame = NULL;
	SwrContext *swr_ctx = NULL;
	uint8_t *dst_data[1] = { NULL }, **data;
	int max_dst_nb_samples = 0, dst_linsize = 0;
	AVPacket packet;

	if (!strcmp(file_name, "-")) {
		file_name = "pipe:0";
	}

	if (avformat_open_input(&format_ctx, file_name, NULL, NULL) != 0) {
		fprintf(stderr, "ERROR: couldn't open the file\n");
		goto done;
	}

	if (avformat_find_stream_info(format_ctx, NULL) < 0) {
		fprintf(stderr, "ERROR: couldn't find stream information in the file\n");
		goto done;
	}

	stream_index = av_find_best_stream(format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
	if (stream_index < 0) {
		fprintf(stderr, "ERROR: couldn't find any audio stream in the file\n");
		goto done;
	}

	stream = format_ctx->streams[stream_index];

	codec_ctx = stream->codec;
	codec_ctx->request_sample_fmt = AV_SAMPLE_FMT_S16;

	if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
		fprintf(stderr, "ERROR: couldn't open the codec\n");
		goto done;
	}
	codec_ctx_opened = 1;

	if (codec_ctx->channels <= 0) {
		fprintf(stderr, "ERROR: no channels found in the audio stream\n");
		goto done;
	}

	if (codec_ctx->sample_fmt != AV_SAMPLE_FMT_S16) {
		swr_ctx = swr_alloc();
		if (!swr_ctx) {
			fprintf(stderr, "ERROR: couldn't allocate audio converter\n");
			goto done;
		}
		av_opt_set_int(swr_ctx, "in_channel_layout", codec_ctx->channel_layout, 0);
		av_opt_set_int(swr_ctx, "out_channel_layout", codec_ctx->channel_layout, 0);
		av_opt_set_int(swr_ctx, "in_sample_rate", codec_ctx->sample_rate, 0);
		av_opt_set_int(swr_ctx, "out_sample_rate", codec_ctx->sample_rate, 0);
		av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", codec_ctx->sample_fmt, 0);
		av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
		if (swr_init(swr_ctx) < 0) {
			fprintf(stderr, "ERROR: couldn't initialize the audio converter\n");
			goto done;
		}
		fprintf(stderr, "WARNING converting audio format\n");
	}

	*duration = stream->time_base.num * stream->duration / stream->time_base.den;

	remaining = max_length * codec_ctx->channels * codec_ctx->sample_rate;
	chromaprint_start(chromaprint_ctx, codec_ctx->sample_rate, codec_ctx->channels);

	frame = avcodec_alloc_frame();

	while (1) {
		if (av_read_frame(format_ctx, &packet) < 0) {
			break;
		}

		if (packet.stream_index == stream_index) {
			avcodec_get_frame_defaults(frame);

			got_frame = 0;
			consumed = avcodec_decode_audio4(codec_ctx, frame, &got_frame, &packet);
			if (consumed < 0) {
				fprintf(stderr, "WARNING: error decoding audio\n");
				continue;
			}

			if (got_frame) {
				if (swr_ctx) {
					if (frame->nb_samples > max_dst_nb_samples) {
						av_freep(&dst_data[0]);
						if (av_samples_alloc(dst_data, &dst_linsize, codec_ctx->channels, frame->nb_samples, AV_SAMPLE_FMT_S16, 1) < 0) {
							fprintf(stderr, "ERROR: couldn't allocate audio converter buffer\n");
							goto done;
						}
						max_dst_nb_samples = frame->nb_samples;
					}
					if (swr_convert(swr_ctx, dst_data, frame->nb_samples, (const uint8_t **)frame->data, frame->nb_samples) < 0) {
						fprintf(stderr, "ERROR: couldn't convert the audio\n");
						goto done;
					}
					data = dst_data;
				}
				else {
					data = frame->data;
				}

				length = MIN(remaining, frame->nb_samples * codec_ctx->channels);
				if (!chromaprint_feed(chromaprint_ctx, data[0], length)) {
					goto done;
				}

				if (max_length) {
					remaining -= length;
					if (remaining <= 0) {
						goto finish;
					}
				}
			}
		}
		av_free_packet(&packet);
	}

finish:
	if (!chromaprint_finish(chromaprint_ctx)) {
		fprintf(stderr, "ERROR: fingerprint calculation failed\n");
		goto done;
	}

	ok = 1;

done:
	if (frame) {
		avcodec_free_frame(&frame);
	}
	if (dst_data[0]) {
		av_freep(&dst_data[0]);
	}
	if (swr_ctx) {
		swr_free(&swr_ctx);
	}
	if (codec_ctx_opened) {
		avcodec_close(codec_ctx);
	}
	if (format_ctx) {
		avformat_close_input(&format_ctx);
	}
	return ok;
}

int fpcalc_main(int argc, char **argv)
{
	int i, j, max_length = 120, num_file_names = 0, raw = 0, raw_fingerprint_size, duration;
	int32_t *raw_fingerprint;
	char *file_name, *fingerprint, **file_names;
	ChromaprintContext *chromaprint_ctx;
	int algo = CHROMAPRINT_ALGORITHM_DEFAULT, num_failed = 0;

	file_names = malloc(argc * sizeof(char *));
	for (i = 1; i < argc; i++) {
		char *arg = argv[i];
		if (!strcmp(arg, "-length") && i + 1 < argc) {
			max_length = atoi(argv[++i]);
		}
		else if (!strcmp(arg, "-version") || !strcmp(arg, "-v")) {
			printf("fpcalc version %s\n", chromaprint_get_version());
			return 0;
		}
		else if (!strcmp(arg, "-raw")) {
			raw = 1;
		}
		else if (!strcmp(arg, "-algo") && i + 1 < argc) {
			const char *v = argv[++i];
			if (!strcmp(v, "test1")) { algo = CHROMAPRINT_ALGORITHM_TEST1; }
			else if (!strcmp(v, "test2")) { algo = CHROMAPRINT_ALGORITHM_TEST2; }
			else if (!strcmp(v, "test3")) { algo = CHROMAPRINT_ALGORITHM_TEST3; }
			else if (!strcmp(v, "test4")) { algo = CHROMAPRINT_ALGORITHM_TEST4; }
			else {
				fprintf(stderr, "WARNING: unknown algorithm, using the default\n");
			}
		}
		else if (!strcmp(arg, "-set") && i + 1 < argc) {
			i += 1;
		}
		else {
			file_names[num_file_names++] = argv[i];
		}
	}

	if (!num_file_names) {
		printf("usage: %s [OPTIONS] FILE...\n\n", argv[0]);
		printf("Options:\n");
		printf("  -version      print version information\n");
		printf("  -length SECS  length of the audio data used for fingerprint calculation (default 120)\n");
		printf("  -raw          output the raw uncompressed fingerprint\n");
		printf("  -algo NAME    version of the fingerprint algorithm\n");
		return 2;
	}

	av_register_all();
	av_log_set_level(AV_LOG_ERROR);

	chromaprint_ctx = chromaprint_new(algo);

	for (i = 1; i < argc; i++) {
		char *arg = argv[i];
		if (!strcmp(arg, "-set") && i + 1 < argc) {
			char *name = argv[++i];
			char *value = strchr(name, '=');
			if (value) {
				*value++ = '\0';
				chromaprint_set_option(chromaprint_ctx, name, atoi(value));
			}
		}
	}

	for (i = 0; i < num_file_names; i++) {
		file_name = file_names[i];
		if (!decode_audio_file(chromaprint_ctx, file_name, max_length, &duration)) {
			fprintf(stderr, "ERROR: unable to calculate fingerprint for file %s, skipping\n", file_name);
			num_failed++;
			continue;
		}
		if (i > 0) {
			printf("\n");
		}
		printf("FILE=%s\n", file_name);
		printf("DURATION=%d\n", duration);
		if (raw) {
			if (!chromaprint_get_raw_fingerprint(chromaprint_ctx, (void **)&raw_fingerprint, &raw_fingerprint_size)) {
				fprintf(stderr, "ERROR: unable to calculate fingerprint for file %s, skipping\n", file_name);
				num_failed++;
				continue;
			}
			printf("FINGERPRINT=");
			for (j = 0; j < raw_fingerprint_size; j++) {
				printf("%d%s", raw_fingerprint[j], j + 1 < raw_fingerprint_size ? "," : "");
			}
			printf("\n");
			chromaprint_dealloc(raw_fingerprint);
		}
		else {
			if (!chromaprint_get_fingerprint(chromaprint_ctx, &fingerprint)) {
				fprintf(stderr, "ERROR: unable to calculate fingerprint for file %s, skipping\n", file_name);
				num_failed++;
				continue;
			}
			printf("FINGERPRINT=%s\n", fingerprint);
			chromaprint_dealloc(fingerprint);
		}
	}

	chromaprint_free(chromaprint_ctx);
	free(file_names);

	return num_failed ? 1 : 0;
}

#ifdef _WIN32
int main(int win32_argc, char **win32_argv)
{
	int i, argc = 0, buffsize = 0, offset = 0;
	char **utf8_argv, *utf8_argv_ptr;
	wchar_t **argv;

	argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	buffsize = 0;
	for (i = 0; i < argc; i++) {
		buffsize += WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, NULL, 0, NULL, NULL);
	}

	utf8_argv = av_mallocz(sizeof(char *) * (argc + 1) + buffsize);
	utf8_argv_ptr = (char *)utf8_argv + sizeof(char *) * (argc + 1);

	for (i = 0; i < argc; i++) {
		utf8_argv[i] = &utf8_argv_ptr[offset];
		offset += WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, &utf8_argv_ptr[offset], buffsize - offset, NULL, NULL);
	}

	LocalFree(argv);

	return fpcalc_main(argc, utf8_argv);
}
#else
int main(int argc, char **argv)
{
	return fpcalc_main(argc, argv);
}
#endif

