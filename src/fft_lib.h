// Copyright (C) 2010-2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_FFT_LIB_H_
#define CHROMAPRINT_FFT_LIB_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WITH_AVFFT
#include "fft_lib_avfft.h"
#endif

#ifdef WITH_FFTW3
#include "fft_lib_fftw3.h"
#endif

#ifdef WITH_FFTW3F
#include "fft_lib_fftw3.h"
#endif

#ifdef WITH_VDSP
#include "fft_lib_vdsp.h"
#endif

#ifdef WITH_KISSFFT
#include "fft_lib_kissfft.h"
#endif

#endif
