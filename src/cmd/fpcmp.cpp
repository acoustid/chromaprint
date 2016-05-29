#include <vector>
#include <string>
#include <cstdio>
#include <iostream>
#include "chromaprint.h"
#include "fingerprinter_configuration.h"
#include "fingerprint_matcher.h"
#include "utils/scope_exit.h"

using namespace chromaprint;

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

int main(int argc, char **argv) {
	std::vector<std::string> file_names;
	for (int i = 1; i < argc; i++) {
		file_names.push_back(argv[i]);
	}

	if (file_names.size() != 2) {
		fprintf(stderr, "usage: %s [OPTIONS] FILE1 FILE2\n\n", argv[0]);
		return 2;
	}

	std::string fp1 = ReadFile(file_names[0]);
	std::string fp2 = ReadFile(file_names[1]);

	uint32_t *fp1_data;
	int fp1_size;
	int fp1_algorithm;

	if (!chromaprint_decode_fingerprint(fp1.c_str(), fp1.size(), &fp1_data, &fp1_size, &fp1_algorithm, 1)) {
		return 1;
	}
	SCOPE_EXIT(chromaprint_dealloc(fp1_data));

	uint32_t *fp2_data;
	int fp2_size;
	int fp2_algorithm;

	if (!chromaprint_decode_fingerprint(fp2.c_str(), fp2.size(), &fp2_data, &fp2_size, &fp2_algorithm, 1)) {
		return 1;
	}
	SCOPE_EXIT(chromaprint_dealloc(fp2_data));

	if (fp1_algorithm != fp2_algorithm) {
		fprintf(stderr, "Fingerprint algorithms do not match\n");
		return 1;
	}

	std::vector<uint32_t> fp1_data2(fp1_data, fp1_data + fp1_size);
	std::vector<uint32_t> fp2_data2(fp2_data, fp2_data + fp2_size);

	chromaprint::FingerprintMatcher matcher(chromaprint::CreateFingerprinterConfiguration(fp1_algorithm));
	matcher.Match(fp1_data2, fp2_data2);

	return 0;
}
