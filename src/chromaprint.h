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

#if (defined(_WIN32) || defined(_WIN64))
#	ifdef CHROMAPRINT_API_EXPORTS
#       define CHROMAPRINT_API __declspec(dllexport)
#   else
#       define CHROMAPRINT_API __declspec(dllimport)
#   endif
#else
#   ifdef GCC_HASCLASSVISIBILITY
#       define CHROMAPRINT_API __attribute__ ((visibility("default")))
#   else
#       define CHROMAPRINT_API
#   endif
#endif

typedef void *ChromaprintContext;

#define CHROMAPRINT_VERSION_MAJOR 0
#define CHROMAPRINT_VERSION_MINOR 1
#define CHROMAPRINT_VERSION_PATCH 1

enum ChromaprintAlgorithm {
	CHROMAPRINT_ALGORITHM_TEST1 = 0,
	CHROMAPRINT_ALGORITHM_TEST2,
	CHROMAPRINT_ALGORITHM_TEST3
};

#define CHROMAPRINT_ALGORITHM_DEFAULT CHROMAPRINT_ALGORITHM_TEST2

/**
 * Return the version number of Chromaprint.
 */
CHROMAPRINT_API const char *chromaprint_get_version(void);

/**
 * Allocate and initialize the Chromaprint context.
 *
 * Parameters:
 *  - version: Version of the fingerprint algorithm, use
 *             CHROMAPRINT_VERSION_DEFAULT for the default
 *             algorithm
 *
 * Returns:
 *  - Chromaprint context pointer
 */
CHROMAPRINT_API ChromaprintContext *chromaprint_new(int algorithm);

/**
 * Deallocate the Chromaprint context.
 *
 * Parameters:
 *  - ctx: Chromaprint context pointer
 */
CHROMAPRINT_API void chromaprint_free(ChromaprintContext *ctx);

/**
 * Return the fingerprint algorithm this context is configured to use.
 */
CHROMAPRINT_API int chromaprint_get_algorithm(ChromaprintContext *ctx);

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
CHROMAPRINT_API int chromaprint_start(ChromaprintContext *ctx, int sample_rate, int num_channels);

/**
 * Send audio data to the fingerprint calculator.
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
CHROMAPRINT_API int chromaprint_feed(ChromaprintContext *ctx, void *data, int size);

/**
 * Process any remaining buffered audio data and calculate the fingerprint.
 *
 * Parameters:
 *  - ctx: Chromaprint context pointer
 *
 * Returns:
 *  - 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_finish(ChromaprintContext *ctx);

/**
 * Return the calculated fingerprint as a compressed string.
 *
 * The returned string is allocated using malloc() and the caller
 * is responsible for freeing the memory.
 *
 * Parameters:
 *  - ctx: Chromaprint context pointer
 *  - fingerprint: pointer to a pointer, where a pointer to the allocated array
 *                 will be stored
 *
 * Returns:
 *  - 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_get_fingerprint(ChromaprintContext *ctx, char **fingerprint);

/**
 * Return the calculated fingerprint as an array of 32-bit integers.
 *
 * The returned array is allocated using malloc() and the caller
 * is responsible for freeing the memory.
 *
 * Parameters:
 *  - ctx: Chromaprint context pointer
 *  - fingerprint: pointer to a pointer, where a pointer to the allocated array
 *                 will be stored
 *  - size: pointer to an integer where the array size will be stored
 *
 * Returns:
 *  - 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_get_raw_fingerprint(ChromaprintContext *ctx, void **fingerprint, int *size);

#ifdef __cplusplus
}
#endif

#endif
