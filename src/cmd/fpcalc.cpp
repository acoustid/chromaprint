#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <chromaprint.h>

#include "audio/ffmpeg_audio_reader.h"
#include "utils/scope_exit.h"

using namespace chromaprint;

static bool g_json = false;
static char *g_input_format = nullptr;
static int g_input_channels = 0;
static int g_input_sample_rate = 0;
static size_t g_chunk_size = 0;

const char *g_help =
	"Usage: %s [OPTIONS] FILE [FILE...]\n"
	"\n"
	"Generate fingerprints from audio files/streams.\n"
	"\n"
	"Options:\n"
	"  -f NAME      Set the input format name\n"
	"  -r NUM       Set the sample rate of the input audio\n"
	"  -c NUM       Set the number of channels in the input audio\n"
	"  -json        Print the output in JSON format\n"
	"  -chunk NUM   Size of one fingerprint chunk in seconds, default 0 (unlimited)\n"
	;

static void ParseOptions(int &argc, char **argv) {
	int j = 1;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--")) {
			while (++i < argc) {
				argv[j++] = argv[i];
			}
		} else if (!strcmp(argv[i], "-f") && i + 1 < argc) {
			g_input_format = argv[++i];
		} else if (!strcmp(argv[i], "-c") && i + 1 < argc) {
			g_input_channels = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "-r") && i + 1 < argc) {
			g_input_sample_rate = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "-chunk") && i + 1 < argc) {
			g_chunk_size = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "-json")) {
			g_json = true;
		} else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "-help") || !strcmp(argv[i], "--help")) {
			fprintf(stdout, g_help, argv[0]);
			exit(0);
		} else {
			const auto len = strlen(argv[i]);
			if (len > 1 && argv[i][0] == '-') {
				fprintf(stderr, "ERROR: Unknown option %s\n", argv[i]);
				exit(2);
			} else {
				argv[j++] = argv[i];
			}
		}
	}
	if (j < 2) {
		fprintf(stderr, "ERROR: No input files\n");
		exit(2);
	}
	argc = j;
}

void PrintResult(ChromaprintContext *ctx, FFmpegAudioReader &reader) {
	char *fp;
	if (!chromaprint_get_fingerprint(ctx, &fp)) {
		fprintf(stderr, "ERROR: Could not get the fingerprinting\n");
		exit(2);
	}
	SCOPE_EXIT(chromaprint_dealloc(fp));

	const auto duration = reader.GetDuration() / 1000.0;

	const char *json_fmt = "{\"duration\": %.2f, \"fingerprint\": \"%s\"}\n";
	const char *text_fmt = "DURATION=%.2f\nJSON=%s\n";
	printf(g_json ? json_fmt : text_fmt, duration, fp);
}

void ProcessFile(ChromaprintContext *ctx, FFmpegAudioReader &reader, const char *file_name) {
	if (!reader.Open(file_name)) {
		fprintf(stderr, "ERROR: %s\n", reader.GetError().c_str());
		exit(2);
	}

	if (!chromaprint_start(ctx, reader.GetSampleRate(), reader.GetChannels())) {
		fprintf(stderr, "ERROR: Could not initialize the fingerprinting process\n");
		exit(2);
	}

	size_t chunk = 0;
	const size_t chunk_limit = g_chunk_size * reader.GetSampleRate();
	while (!reader.IsFinished()) {
		const int16_t *data = nullptr;
		size_t size = 0;
		if (!reader.Read(&data, &size)) {
			fprintf(stderr, "ERROR: %s\n", reader.GetError().c_str());
			exit(2);
		}
		if (data) {
			if (!chromaprint_feed(ctx, data, size * reader.GetChannels())) {
				fprintf(stderr, "ERROR: Could not process audio data\n");
				exit(2);
			}
			chunk += size;
			if (chunk_limit > 0 && chunk >= chunk_limit) {
				PrintResult(ctx, reader);
				chromaprint_clear_fingerprint(ctx);
				chunk -= chunk_limit;
			}
		}
	}

	if (!chromaprint_finish(ctx)) {
		fprintf(stderr, "ERROR: Could not finish the fingerprinting process\n");
		exit(2);
	}

	PrintResult(ctx, reader);
}

int main(int argc, char **argv) {
	ParseOptions(argc, argv);

	FFmpegAudioReader reader;
	if (g_input_format) {
		if (!reader.SetInputFormat(g_input_format)) {
			fprintf(stderr, "ERROR: Invalid format\n");
			return 2;
		}
	}
	if (g_input_channels) {
		if (!reader.SetInputChannels(g_input_channels)) {
			fprintf(stderr, "ERROR: Invalid number of channels\n");
			return 2;
		}
	}
	if (g_input_sample_rate) {
		if (!reader.SetInputSampleRate(g_input_sample_rate)) {
			fprintf(stderr, "ERROR: Invalid sample rate\n");
			return 2;
		}
	}

	ChromaprintContext *chromaprint_ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);
	SCOPE_EXIT(chromaprint_free(chromaprint_ctx));

	for (int i = 1; i < argc; i++) {
		ProcessFile(chromaprint_ctx, reader, argv[i]);
	}

	return 0;
}
