// Copyright (C) 2010-2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_FFT_H_
#define CHROMAPRINT_FFT_H_

#include <math.h>
#include "utils.h"
#include "fft_frame.h"
#include "fft_frame_consumer.h"
#include "audio_consumer.h"
#include "combined_buffer.h"

namespace chromaprint {

class FFTLib;

class FFT : public AudioConsumer
{
public:
	FFT(int frame_size, int overlap, FFTFrameConsumer *consumer);
	~FFT();

	int FrameSize() const { return m_frame_size; }
	int Overlap() const { return m_frame_size - m_increment; }

	void Reset();
	void Consume(short *input, int length);

private:
	CHROMAPRINT_DISABLE_COPY(FFT);

	double *m_window;
	int m_buffer_offset;
	short *m_buffer;
	FFTFrame m_frame;
	int m_frame_size;
	int m_increment;
	FFTLib *m_lib;
	FFTFrameConsumer *m_consumer;
};

}; // namespace chromaprint

#endif
