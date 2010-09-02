#include <string>
#include <iostream>
#include "ext/ffmpeg_decoder.h"
#include "fingerprinter.h"

using namespace std;

int main(int argc, char **argv)
{
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " FILENAME\n";
		return 1;
	}

	string file_name(argv[1]);
	Decoder decoder(file_name);
	if (!decoder.Open()) {
		cerr << "ERROR: " << decoder.LastError() << "\n";
		return 2;
	}

	Chromaprint::Fingerprinter fingerprinter;

	if (!fingerprinter.Init(decoder.SampleRate(), decoder.Channels())) {
		return 2;
	}
	decoder.Decode(&fingerprinter);
	//decoder.Decode(&fingerprinter, 60);
	vector<int32_t> fingerprint = fingerprinter.Calculate();

	for (int i = 0; i < fingerprint.size(); i++) {
		cout << fingerprint[i] << "\n";
	}

	return 0;
}

