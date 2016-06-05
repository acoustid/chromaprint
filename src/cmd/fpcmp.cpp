#include <vector>
#include <string>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <chromaprint.h>
#include "utils/scope_exit.h"

using namespace chromaprint;

enum Format {
	TEXT = 0,
	JSON,
};

static std::string RStrip(std::string s) {
	while (!s.empty() && isspace(s.back())) {
		s.resize(s.size() - 1);
	}
	return s;
}

static std::string ReadFile(std::string file_name) {
	FILE *fp = fopen(file_name.c_str(), "r");
	if (fp == NULL) {
		return std::string();
	}
	SCOPE_EXIT(fclose(fp));

	char buffer[1024];
	std::string line;
	while (true) {
		bool new_line;
		if (fgets(buffer, sizeof(buffer), fp) == NULL) {
			if (line.empty()) {
				return std::string();
			}
			new_line = true;
		} else {
			line.append(buffer);
			new_line = line.back() == '\n';
		}
		if (new_line) {
			const std::string prefix = "FINGERPRINT=";
			if (line.compare(0, prefix.size(), prefix) == 0) {
				return RStrip(line.substr(prefix.size()));
			}
			line.clear();
		}
	}
	return std::string();
}

void PrintJson(ChromaprintMatcherContext *ctx) {
	int num_segments = 0;
	chromaprint_matcher_get_num_segments(ctx, &num_segments);
	printf("{\n  \"segments\": [\n");
	for (int i = 0; i < num_segments; i++) {
		int pos1, pos2, duration, score;
		chromaprint_matcher_get_segment_position_ms(ctx, i, &pos1, &pos2, &duration);
		chromaprint_matcher_get_segment_score(ctx, i, &score);
		printf("    {\n");
		printf("      \"pos1\": %d\n", pos1);
		printf("      \"pos2\": %d\n", pos2);
		printf("      \"duration\": %d\n", duration);
		printf("      \"score\": %d\n", score);
		if (i + 1 == num_segments) {
			printf("    }\n");
		} else {
			printf("    },\n");
		}
	}
	printf("  ]\n}\n");
}

std::string FormatTime(int ms, bool with_hours = false) {
	char buf[100];
	const int s = (ms + 500) / 1000;
	const int m = s / 60;
	const int h = m / 60;
	if (with_hours) {
		snprintf(buf, 100, "%02d:%02d:%02d", h, m % 60, s % 60);
	} else {
		snprintf(buf, 100, "%02d:%02d", m, s % 60);
	}
	return buf;
}

std::string FormatTimeRange(int ms1, int ms2) {
	char buf[100];
	snprintf(buf, 100, "%s-%s", FormatTime(ms1).c_str(), FormatTime(ms2).c_str());
	return buf;
}

void PrintText(ChromaprintMatcherContext *ctx) {
	int num_segments = 0;
	chromaprint_matcher_get_num_segments(ctx, &num_segments);
	printf("%s\t%s\t%s\t%s\n", "POSITION1", "POSITION2", "SCORE", "DURATION");
	for (int i = 0; i < num_segments; i++) {
		int pos1, pos1_ms, pos2, pos2_ms, duration, duration_ms, score;
		chromaprint_matcher_get_segment_position_ms(ctx, i, &pos1, &pos2, &duration);
		chromaprint_matcher_get_segment_position_ms(ctx, i, &pos1_ms, &pos2_ms, &duration_ms);
		chromaprint_matcher_get_segment_score(ctx, i, &score);
		printf("%s\t%s\t%d\t%s\n",
			FormatTimeRange(pos1, pos1 + duration).c_str(),
			FormatTimeRange(pos2, pos2 + duration).c_str(),
			score, FormatTime(duration).c_str());
	}
}

int main(int argc, char **argv) {
	Format format = TEXT;
	std::vector<std::string> file_names;
	bool no_more_options = false;
	for (int i = 1; i < argc; i++) {
		if (!no_more_options) {
			if (!strcmp(argv[i], "--")) {
				no_more_options = true;
				continue;
			} else if (!strcmp(argv[i], "-text")) {
				format = TEXT;
				continue;
			} else if (!strcmp(argv[i], "-json")) {
				format = JSON;
				continue;
			} else if (!strncmp(argv[i], "-", 1)) {
				fprintf(stderr, "ERROR: unknown option %s\n", argv[i]);
				return 2;
			}
		}
		file_names.push_back(argv[i]);
	}

	if (file_names.size() != 2) {
		fprintf(stderr, "usage: %s [OPTIONS] FILE1 FILE2\n\n", argv[0]);
		return 2;
	}

	ChromaprintMatcherContext *ctx = chromaprint_matcher_new();
	if (!ctx) {
		fprintf(stderr, "ERROR: could not create chromaprint matcher context\n");
		return 1;
	}

	SCOPE_EXIT(chromaprint_matcher_free(ctx));

	std::string fp1 = ReadFile(file_names[0]);
	if (fp1.empty()) {
		fprintf(stderr, "ERROR: could not load the first fingerprint\n");
		return 1;
	}

	if (!chromaprint_matcher_set_fingerprint(ctx, 0, fp1.c_str())) {
		fprintf(stderr, "ERROR: could not load the first fingerprint\n");
		return 1;
	}

	std::string fp2 = ReadFile(file_names[1]);
	if (fp2.empty()) {
		fprintf(stderr, "ERROR: could not load the second fingerprint\n");
		return 1;
	}

	if (!chromaprint_matcher_set_fingerprint(ctx, 1, fp2.c_str())) {
		fprintf(stderr, "ERROR: could not load the second fingerprint\n");
		return 1;
	}

	if (!chromaprint_matcher_run(ctx)) {
		fprintf(stderr, "ERROR: matched failed\n");
		return 1;
	}

	switch (format) {
		case JSON:
			PrintJson(ctx);
			break;
		default:
			PrintText(ctx);
			break;
	}

	return 0;
}
