/*
 * Chromaprint -- Audio fingerprinting toolkit
 * Copyright (C) 2010  Lukas Lalinsky <lalinsky@gmail.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "fingerprinter.h"
#include "chroma.h"
#include "chroma_normalizer.h"
#include "chroma_filter.h"
#include "fft.h"
#include "audio_processor.h"
#include "image_builder.h"
#include "fingerprint_calculator.h"
#include "classifier.h"
#include "utils.h"

using namespace std;
using namespace Chromaprint;

static const int SAMPLE_RATE = 11025;
static const int FRAME_SIZE = 4096;
static const int OVERLAP = FRAME_SIZE - FRAME_SIZE / 3;
static const int MIN_FREQ = 28;
static const int MAX_FREQ = 3520;

static const int kChromaFilterSize = 5;
static const double kChromaFilterCoefficients[] = { 0.25, 0.75, 1.0, 0.75, 0.25 };

static const int kNumClassifiers = 16;
static const Classifier kClassifiers[] = {
/*	Classifier(Filter(0, 1, 3, 15), Quantizer(1.95009, 2.33824, 2.61099)),
	Classifier(Filter(1, 9, 2, 16), Quantizer(-0.277503, 0.0394634, 0.360053)),
	Classifier(Filter(4, 0, 9, 11), Quantizer(-1.1158, -0.769526, -0.414701)),
	Classifier(Filter(3, 7, 4, 16), Quantizer(-0.127323, 0.00634234, 0.211132)),
	Classifier(Filter(4, 1, 3, 15), Quantizer(-0.529438, -0.234716, 0.0427898)),
	Classifier(Filter(2, 7, 2, 8), Quantizer(-0.0807463, 0.0249708, 0.201308)),
	Classifier(Filter(2, 3, 1, 14), Quantizer(-0.225254, -0.0298985, 0.0831879)),
	Classifier(Filter(5, 4, 1, 15), Quantizer(-0.573732, -0.409864, -0.248336)),
	Classifier(Filter(5, 2, 9, 9), Quantizer(-0.682885, -0.622708, -0.564849)),
	Classifier(Filter(3, 9, 2, 12), Quantizer(-0.0764421, 0.0218273, 0.126766)),
	Classifier(Filter(2, 5, 3, 2), Quantizer(-0.0456227, -0.00807542, 0.0271403)),
	Classifier(Filter(3, 1, 2, 6), Quantizer(-0.0944788, -0.0163406, 0.0961356)),
	Classifier(Filter(3, 0, 6, 4), Quantizer(-0.0519634, 0.0120367, 0.0776205)),
	Classifier(Filter(1, 6, 2, 2), Quantizer(-0.111264, -0.0180069, 0.0696652)),
	Classifier(Filter(3, 7, 2, 6), Quantizer(-0.129769, -0.0150235, 0.0544394)),
	Classifier(Filter(4, 0, 3, 13), Quantizer(-1.34322, -0.897052, -0.612338)),*/
	Classifier(Filter(0, 0, 3, 15), Quantizer(2.10543, 2.45354, 2.69414)),
	Classifier(Filter(1, 0, 4, 14), Quantizer(-0.345922, 0.0463746, 0.446251)),
	Classifier(Filter(1, 4, 4, 11), Quantizer(-0.392132, 0.0291077, 0.443391)),
	Classifier(Filter(3, 0, 4, 14), Quantizer(-0.192851, 0.00583535, 0.204053)),
	Classifier(Filter(2, 8, 2, 4), Quantizer(-0.0771619, -0.00991999, 0.0575406)),
	Classifier(Filter(5, 6, 2, 15), Quantizer(-0.710437, -0.518954, -0.330402)),
	Classifier(Filter(1, 9, 2, 16), Quantizer(-0.353724, -0.0189719, 0.289768)),
	Classifier(Filter(3, 4, 2, 10), Quantizer(-0.128418, -0.0285697, 0.0591791)),
	Classifier(Filter(3, 9, 2, 16), Quantizer(-0.139052, -0.0228468, 0.0879723)),
	Classifier(Filter(2, 1, 3, 6), Quantizer(-0.133562, 0.00669205, 0.155012)),
	Classifier(Filter(3, 3, 6, 2), Quantizer(-0.0267, 0.00804829, 0.0459773)),
	Classifier(Filter(2, 8, 1, 10), Quantizer(-0.0972417, 0.0152227, 0.129003)),
	Classifier(Filter(3, 4, 4, 14), Quantizer(-0.141434, 0.00374515, 0.149935)),
	Classifier(Filter(5, 4, 2, 15), Quantizer(-0.64035, -0.466999, -0.285493)),
	Classifier(Filter(5, 9, 2, 3), Quantizer(-0.322792, -0.254258, -0.174278)),
	Classifier(Filter(2, 1, 8, 4), Quantizer(-0.0741375, -0.00590933, 0.0600357)),
};

Fingerprinter::Fingerprinter()
	: m_image(12)
{
	m_image_builder = new ImageBuilder(&m_image);
	m_chroma_normalizer = new ChromaNormalizer(m_image_builder);
	m_chroma_filter = new ChromaFilter(kChromaFilterCoefficients, kChromaFilterSize, m_chroma_normalizer);
	m_chroma = new Chroma(MIN_FREQ, MAX_FREQ, FRAME_SIZE, SAMPLE_RATE, m_chroma_filter);
	m_fft = new FFT(FRAME_SIZE, OVERLAP, m_chroma);
	m_audio_processor = new AudioProcessor(SAMPLE_RATE, m_fft);
	m_fingerprint_calculator = new FingerprintCalculator(kClassifiers, kNumClassifiers);
}

Fingerprinter::~Fingerprinter()
{
	delete m_fingerprint_calculator;
	delete m_audio_processor;
	delete m_fft;
	delete m_chroma;
	delete m_chroma_filter;
	delete m_chroma_normalizer;
	delete m_image_builder;
}

bool Fingerprinter::Init(int sample_rate, int num_channels)
{
	if (!m_audio_processor->Reset(sample_rate, num_channels)) {
		// FIXME save error message somewhere
		return false;
	}
	m_fft->Reset();
	m_chroma->Reset();
	m_chroma_filter->Reset();
	m_chroma_normalizer->Reset();
	m_image = Image(12); // XXX
	m_image_builder->Reset(&m_image);
	return true;
}

void Fingerprinter::Consume(short *samples, int length)
{
	assert(length >= 0);
	m_audio_processor->Consume(samples, length);
}

vector<int32_t> Fingerprinter::Calculate()
{
	return m_fingerprint_calculator->Calculate(&m_image);
}

