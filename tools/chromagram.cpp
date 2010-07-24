#include <string>
#include <iostream>

using namespace std;

#include "ext/ffmpeg_decoder.h"
#include "ext/audio_dumper.h"
#include "audio_processor.h"
#include "chroma.h"
#include "spectral_centroid.h"
#include "chroma_normalizer.h"
#include "chroma_resampler.h"
#include "fft.h"
#include "audio_processor.h"
#include "image.h"
#include "image_builder.h"
#include "utils.h"
#include "ext/image_utils.h"

static const int SAMPLE_RATE = 11025;
static const int FRAME_SIZE = 4096;
static const int OVERLAP = FRAME_SIZE - FRAME_SIZE / 3;// 2720;
static const int MIN_FREQ = 28;
static const int MAX_FREQ = 3520;
static const int MAX_FILTER_WIDTH = 20;

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

	Chromaprint::Image image(12);
	Chromaprint::ImageBuilder image_builder(&image);
	Chromaprint::ChromaNormalizer chroma_normalizer(&image_builder);
	//Chromaprint::ChromaResampler resampler(true, &image_builder);
	//Chromaprint::Chroma chroma(MIN_FREQ, MAX_FREQ, FRAME_SIZE, SAMPLE_RATE, &resampler);
	Chromaprint::Chroma chroma(MIN_FREQ, MAX_FREQ, FRAME_SIZE, SAMPLE_RATE, &chroma_normalizer);
	//Chromaprint::SpectralCentroid centroid(16, MIN_FREQ, MAX_FREQ, FRAME_SIZE, SAMPLE_RATE, &image_builder);
	Chromaprint::FFT fft(FRAME_SIZE, OVERLAP, &chroma);
	Chromaprint::AudioProcessor processor(SAMPLE_RATE, &fft);

	cout << decoder.Channels() << "\n";
	processor.Reset(decoder.SampleRate(), decoder.Channels());
	decoder.Decode(&processor);
	processor.Flush();

	//Chromaprint::ExportTextImage(&image, argv[2]);
	Chromaprint::ExportImage(&image, argv[2]);

	return 0;
}

