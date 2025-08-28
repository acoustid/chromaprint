// Copyright (C) 2010-2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#include "fft_lib_avtx.h"

namespace chromaprint {

FFTLib::FFTLib(size_t frame_size) : m_frame_size(frame_size) {
	m_window = (float *) av_malloc(sizeof(float) * frame_size);
	m_input = (float *) av_malloc(sizeof(float) * frame_size);
	m_output = (AVComplexFloat *) av_malloc(sizeof(AVComplexFloat) * (frame_size / 2 + 1));
	PrepareHammingWindow(m_window, m_window + frame_size, 1.0 / INT16_MAX);
	
	// Initialize the RDFT transform context
	// For real-to-complex transform: inv=0, no scaling, no special flags
	int ret = av_tx_init(&m_tx_ctx, &m_tx_fn, AV_TX_FLOAT_RDFT, 0, frame_size, NULL, 0);
	if (ret < 0) {
		// Handle initialization error - this should not happen with valid parameters
		m_tx_ctx = NULL;
		m_tx_fn = NULL;
	}
}

FFTLib::~FFTLib() {
	av_tx_uninit(&m_tx_ctx);
	av_free(m_output);
	av_free(m_input);
	av_free(m_window);
}

void FFTLib::Load(const int16_t *b1, const int16_t *e1, const int16_t *b2, const int16_t *e2) {
	auto window = m_window;
	auto output = m_input;
	ApplyWindow(b1, e1, window, output);
	ApplyWindow(b2, e2, window, output);
}

void FFTLib::Compute(FFTFrame &frame) {
	if (!m_tx_ctx || !m_tx_fn) {
		// Transform context initialization failed
		return;
	}
	
	// Perform the real-to-complex FFT
	// stride parameter: spacing between input samples in bytes
	m_tx_fn(m_tx_ctx, m_output, m_input, sizeof(float));
	
	// Convert complex output to power spectrum
	auto input = m_output;
	auto output = frame.begin();
	
	// Handle DC component (index 0)
	output[0] = input[0].re * input[0].re;
	
	// Handle Nyquist frequency (index frame_size/2)
	output[m_frame_size / 2] = input[m_frame_size / 2].re * input[m_frame_size / 2].re;
	
	// Handle intermediate frequencies (indices 1 to frame_size/2-1)
	output += 1;
	input += 1;
	for (size_t i = 1; i < m_frame_size / 2; i++) {
		*output++ = input->re * input->re + input->im * input->im;
		input++;
	}
}

}; // namespace chromaprint