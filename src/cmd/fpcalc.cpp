#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sstream>

#include <chromaprint.h>

#include "audio/ffmpeg_audio_reader.h"
#include "utils/scope_exit.h"

using namespace chromaprint;

static bool g_json = false;
static char *g_input_format = nullptr;
static int g_input_channels = 0;
static int g_input_sample_rate = 0;
static double g_max_duration = 0;
static double g_chunk_duration = 0;
static bool g_overlap = false;
static bool g_raw = false;

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
	"  -raw         Output fingerprints in the uncompressed format\n"
	"  -length SECS Restrict the duration of the processed input audio\n"
	"  -chunk SECS  Split the input audio into chunks of this duration\n"
	"  -overlap     Overlap the chunks slightly to make sure audio on the edges is fingerprinted\n"
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
			auto value = atoi(argv[i + 1]);
			if (value > 0) {
				g_input_channels = value;
			} else {
				fprintf(stderr, "ERROR: The argument for %s must be a non-zero positive number\n", argv[i]);
				exit(2);
			}
			i++;
		} else if (!strcmp(argv[i], "-r") && i + 1 < argc) {
			auto value = atoi(argv[i + 1]);
			if (value >= 0) {
				g_input_sample_rate = value;
			} else {
				fprintf(stderr, "ERROR: The argument for %s must be a positive number\n", argv[i]);
				exit(2);
			}
			i++;
		} else if (!strcmp(argv[i], "-length") && i + 1 < argc) {
			auto value = atof(argv[i + 1]);
			if (value >= 0) {
				g_max_duration = value;
			} else {
				fprintf(stderr, "ERROR: The argument for %s must be a positive number\n", argv[i]);
				exit(2);
			}
			i++;
		} else if (!strcmp(argv[i], "-chunk") && i + 1 < argc) {
			auto value = atof(argv[i + 1]);
			if (value >= 0) {
				g_chunk_duration = value;
			} else {
				fprintf(stderr, "ERROR: The argument for %s must be a positive number\n", argv[i]);
				exit(2);
			}
			i++;
		} else if (!strcmp(argv[i], "-json")) {
			g_json = true;
		} else if (!strcmp(argv[i], "-overlap")) {
			g_overlap = true;
		} else if (!strcmp(argv[i], "-raw")) {
			g_raw = true;
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
	std::string tmp_fp;
	const char *fp;
	bool dealloc_fp = false;

	if (g_raw) {
		std::stringstream ss;
		uint32_t *raw_fp_data = nullptr;
		int raw_fp_size = 0;
		if (!chromaprint_get_raw_fingerprint(ctx, &raw_fp_data, &raw_fp_size)) {
			fprintf(stderr, "ERROR: Could not get the fingerprinting\n");
			exit(2);
		}
		SCOPE_EXIT(chromaprint_dealloc(raw_fp_data));
		for (int i = 0; i < raw_fp_size; i++) {
			if (i > 0) {
				ss << ',';
			}
			ss << int32_t(raw_fp_data[i]);
		}
		tmp_fp = ss.str();
		fp = tmp_fp.c_str();
	} else {
		char *tmp_fp2;
		if (!chromaprint_get_fingerprint(ctx, &tmp_fp2)) {
			fprintf(stderr, "ERROR: Could not get the fingerprinting\n");
			exit(2);
		}
		fp = tmp_fp2;
		dealloc_fp = true;
	}
	SCOPE_EXIT(if (dealloc_fp) { chromaprint_dealloc((void *) fp); });

	const auto duration = reader.GetDuration() / 1000.0;

	const char *json_fmt = "{\"duration\": %.2f, \"fingerprint\": \"%s\"}\n";
	const char *raw_json_fmt = "{\"duration\": %.2f, \"fingerprint\": [%s]}\n";
	const char *text_fmt = "DURATION=%.2f\nFINGERPRINT=%s\n";
	printf(g_json ? (g_raw ? raw_json_fmt : json_fmt) : text_fmt, duration, fp);
}

void ProcessFile(ChromaprintContext *ctx, FFmpegAudioReader &reader, const char *file_name) {
	if (!reader.Open(file_name)) {
		fprintf(stderr, "ERROR: %s\n", reader.GetError().c_str());
		exit(2);
	}

	size_t remaining = g_max_duration * reader.GetSampleRate();

	if (!chromaprint_start(ctx, reader.GetSampleRate(), reader.GetChannels())) {
		fprintf(stderr, "ERROR: Could not initialize the fingerprinting process\n");
		exit(2);
	}

	size_t chunk = 0;
	const size_t chunk_limit = g_chunk_duration * reader.GetSampleRate();
	while (!reader.IsFinished()) {
		const int16_t *data = nullptr;
		size_t size = 0;
		if (!reader.Read(&data, &size)) {
			fprintf(stderr, "ERROR: %s\n", reader.GetError().c_str());
			exit(2);
		}

		if (data) {
			if (g_max_duration > 0) {
				if (size > remaining) {
					size = remaining;
				}
				remaining -= size;
			}

			size_t first_part_size = size;
			if (chunk_limit > 0 && chunk + size > chunk_limit) {
				first_part_size = chunk_limit - chunk;
			}

			if (!chromaprint_feed(ctx, data, first_part_size * reader.GetChannels())) {
				fprintf(stderr, "ERROR: Could not process audio data\n");
				exit(2);
			}

			data += first_part_size * reader.GetChannels();
			size -= first_part_size;

			chunk += first_part_size;

			if (chunk_limit > 0 && chunk >= chunk_limit) {
				if (!chromaprint_finish(ctx)) {
					fprintf(stderr, "ERROR: Could not finish the fingerprinting process\n");
					exit(2);
				}
				PrintResult(ctx, reader);
				if (g_overlap) {
					if (!chromaprint_clear_fingerprint(ctx)) {
						fprintf(stderr, "ERROR: Could not initialize the fingerprinting process\n");
						exit(2);
					}
				} else {
					if (!chromaprint_start(ctx, reader.GetSampleRate(), reader.GetChannels())) {
						fprintf(stderr, "ERROR: Could not initialize the fingerprinting process\n");
						exit(2);
					}
				}
				chunk -= chunk_limit;
			}

			if (size > 0) {
				if (!chromaprint_feed(ctx, data, size * reader.GetChannels())) {
					fprintf(stderr, "ERROR: Could not process audio data\n");
					exit(2);
				}
			}

			chunk += size;
		}
	}

	if (!chromaprint_finish(ctx)) {
		fprintf(stderr, "ERROR: Could not finish the fingerprinting process\n");
		exit(2);
	}

	if (chunk > 0) {
		PrintResult(ctx, reader);
	}
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
