#include <string>
#include <iostream>

using namespace std;

#include "ext/ffmpeg_decoder.h"
#include "ext/audio_dumper.h"
#include "audio_processor.h"
#include "chroma.h"
#include "spectrum.h"
#include "chroma_normalizer.h"
#include "chroma_resampler.h"
#include "chroma_filter.h"
#include "fft.h"
#include "audio_processor.h"
#include "image.h"
#include "image_builder.h"
#include "utils.h"
#include "ext/image_utils.h"

static const int kSampleRate = 44100;
static const int kFrameDataSize = 2 * 256; // 11.60 ms
static const int kFrameIncrement = 2 * 64; // 2.90 ms
static const int kFrameTotalSize = 2 * 1024; // 46.44 ms

int main(int argc, char **argv)
{
	if (argc < 3) {
		cerr << "Usage: " << argv[0] << " AUDIOFILE IMAGEFILE\n";
		return 1;
	}

	string file_name(argv[1]);
	cout << "Loading file " << file_name << "\n";

	Decoder decoder(file_name);
	if (!decoder.Open()) {
		cerr << "ERROR: " << decoder.LastError() << "\n";
		return 2;
	}

	const int numBands = 25;
	Chromaprint::Image image(numBands);
	Chromaprint::ImageBuilder image_builder(&image);
	Chromaprint::Spectrum chroma(numBands, 0, 15500, kFrameTotalSize, kSampleRate, &image_builder);
	Chromaprint::FFT fft(kFrameTotalSize, kFrameTotalSize - kFrameIncrement, &chroma, Chromaprint::FFT::kHannWindow, kFrameDataSize);
	Chromaprint::AudioProcessor processor(kSampleRate, &fft);

	processor.Reset(decoder.SampleRate(), decoder.Channels());
	decoder.Decode(&processor);
	processor.Flush();

	//Chromaprint::ExportTextImage(&image, argv[2]);
	Chromaprint::ExportImage(&image, argv[2], 0.5);

	return 0;
}

