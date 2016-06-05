// Copyright (C) 2010-2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#include "utils.h"
#include "fft_lib_fftw3.h"

namespace chromaprint {

FFTLib::FFTLib(int frame_size, double *window)
	: m_window(window),
	  m_frame_size(frame_size)
{
	m_input = (double *) fftw_malloc(sizeof(double) * frame_size);
	m_output = (double *) fftw_malloc(sizeof(double) * frame_size);
	m_plan = fftw_plan_r2r_1d(frame_size, m_input, m_output, FFTW_R2HC, FFTW_ESTIMATE);
}

FFTLib::~FFTLib()
{
	fftw_destroy_plan(m_plan);
	fftw_free(m_input);
	fftw_free(m_output);
}

void FFTLib::ComputeFrame(CombinedBuffer<int16_t>::Iterator input, double *output)
{
	ApplyWindow(input, m_window, m_input, m_frame_size, 1.0);
	fftw_execute(m_plan);
	double *in_ptr = m_output;
	double *rev_in_ptr = m_output + m_frame_size - 1;
	output[0] = in_ptr[0] * in_ptr[0];
	output[m_frame_size / 2] = in_ptr[m_frame_size / 2] * in_ptr[m_frame_size / 2];
	in_ptr += 1;
	output += 1;
	for (int i = 1; i < m_frame_size / 2; i++) {
		*output++ = in_ptr[0] * in_ptr[0] + rev_in_ptr[0] * rev_in_ptr[0];
		in_ptr++;
		rev_in_ptr--;
	}
}

}; // namespace chromaprint
