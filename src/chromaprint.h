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

#ifndef CHROMAPRINT_CHROMAPRINT_H_
#define CHROMAPRINT_CHROMAPRINT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void *ChromaprintContext;

ChromaprintContext *chromaprint_init();
void chromaprint_destroy(ChromaprintContext *ctx);

/**
 * Restart the computation of a fingerprint with a new audio stream.
 *
 * Parameters:
 *  - ctx: Chromaprint context pointer
 *  - sample_rate: sample rate of the audio stream (in Hz)
 *  - num_channels: numbers of channels in the audio stream (1 or 2)
 *
 * Returns:
 *  - 0 on error, 1 on success
 */
int chromaprint_setup(ChromaprintContext *ctx, int sample_rate, int num_channels);

/**
 * Sent audio data to the fingerprint calculator.
 *
 * Parameters:
 *  - ctx: Chromaprint context pointer
 *  - data: raw audio data, should point to an array of 16-bit signed
 *          integers in native byte-order
 *  - size: size of the data buffer (in samples)
 *
 * Returns:
 *  - 0 on error, 1 on success
 */
int chromaprint_feed(ChromaprintContext *ctx, void *data, int size);

/**
 * Process the recieved audio stream and calculate the fingerprint.
 *
 * Parameters:
 *  - ctx: Chromaprint context pointer
 *  - data: raw audio data, should point to an array of 16-bit signed
 *          integers in native byte-order
 *  - size: size of the data buffer 
 *
 * Returns:
 *  - 0 on error, 1 on success
 */
int chromaprint_compute(ChromaprintContext *ctx, void **fingerprint, int **size);

#ifdef __cplusplus
}
#endif

#endif
