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

#include "utils.h"
#include "fft_lib_kissfft.h"

using namespace std;
using namespace Chromaprint;

FFTLib::FFTLib(int frame_size, double *window)
	: m_window(window),
	  m_frame_size(frame_size)
{
	cfg = kiss_fftr_alloc(frame_size, 0, NULL, NULL);
	m_input = new kiss_fft_scalar[frame_size];
	m_output = new kiss_fft_cpx[frame_size];
}

FFTLib::~FFTLib()
{
	kiss_fftr_free(cfg);
	delete[] m_input;
	delete[] m_output;
}

void FFTLib::ComputeFrame(CombinedBuffer<short>::Iterator input, double *output)
{
	ApplyWindow(input, m_window, m_input, m_frame_size, 1.0);
	kiss_fftr(cfg, m_input, m_output);

	const kiss_fft_cpx *in_ptr = m_output;
	for (int i = 0; i <= m_frame_size / 2; i++) {
		*output++ = in_ptr[0].r * in_ptr[0].r + in_ptr[0].i * in_ptr[0].i;
		in_ptr++;
	}
}
