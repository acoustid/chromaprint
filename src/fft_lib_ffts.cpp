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

#include <xmmintrin.h>
#include "utils.h"
#include "fft_lib_ffts.h"

using namespace std;
using namespace Chromaprint;

FFTLib::FFTLib(int frame_size, double *window)
	: m_frame_size(frame_size),
	  m_window(window)
{
	m_input = (double *)_mm_malloc(frame_size, 32);
	m_output = (double *)_mm_malloc(frame_size, 32);
	m_plan = ffts_init_1d_real(frame_size, -1);
}

FFTLib::~FFTLib()
{
	ffts_free(m_plan);
	_mm_free(m_output);
	_mm_free(m_input);
}

void FFTLib::ComputeFrame(CombinedBuffer<short>::Iterator input, double *output)
{
	ApplyWindow(input, m_window, m_input, m_frame_size, 1.0);
	ffts_execute(m_plan, m_input, m_output);
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
