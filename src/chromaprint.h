// Copyright (C) 2010-2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_CHROMAPRINT_H_
#define CHROMAPRINT_CHROMAPRINT_H_

/**
 * @mainpage Chromaprint
 *
 * @section intro Introduction
 *
 * Chromaprint is a library for generating audio fingerprints, mainly to be used with the <a href="https://acoustid.org">AcoustID</a> service.
 *
 * It needs raw audio stream (16-bit signed int) on input. The audio can have any sampling rate and any number of channels. Typically,
 * you would use some native library for decoding compressed audio files and feed the result into Chromaprint.
 *
 * Audio fingerprints returned from the library can be represented either as
 * base64-encoded strings or 32-bit integer arrays. The base64-encoded strings
 * are usually what's used externally when you need to send the fingerprint
 * to a service. You can't directly compare the fingerprints in such form.
 * The 32-bit integer arrays are also called "raw fingerprints" and they
 * represent the internal structure of the fingerprints. If you want to
 * compare two fingerprints yourself, you probably want them in this form.
 *
 * @section generating Generating fingerprints
 *
 * Here is a simple example code that generates a fingerprint from audio samples in memory:
 *
 * @code
 * ChromaprintContext *ctx;
 * char *fp;
 *
 * const int sample_rate = 44100;
 * const int num_channels = 2;
 *
 * ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);
 *
 * chromaprint_start(ctx, sample_rate, num_channels);
 * while (has_more_data) {
 *     chromaprint_feed(ctx, data, size);
 * }
 * chromaprint_finish(ctx);
 *
 * chromaprint_get_fingerprint(ctx, &fp);
 * printf("%s\n", fp);
 * chromaprint_dealloc(fp);
 *
 * chromaprint_free(ctx);
 * @endcode
 *
 * @section comparing Comparing fingerprints
 *
 * Here is a simple example code for matching two fingerprints:
 *
 * @code
 * ChromaprintMatcherContext *ctx;
 * int num_segments, i, pos1, pos2, duration, score;
 *
 * ctx = chromaprint_matcher_new();
 *
 * chromaprint_matcher_set_fingerprint(ctx, 0, "AQAAS5IURssi4vnxPRr-CHuHP-B3P...");
 * chromaprint_matcher_set_fingerprint(ctx, 1, "AQAAS1KiiHGW4MqOH86HF01OJOee4...");
 * chromaprint_matcher_run(ctx);
 *
 * chromaprint_matcher_get_num_segments(ctx, &num_segments);
 * for (i = 0; i < num_segments; i++) {
 *     chromaprint_matcher_get_segment_position_ms(ctx, i, &pos1, &pos2, &duration);
 *     chromaprint_matcher_get_segment_score(ctx, i, &score);
 *     printf("%d-%d from fp1 matches %d-%d from fp2 with score %d\n", pos1, pos1 + duration - 1, pos2, pos2 + duration - 1, score);
 * }
 *
 * chromaprint_matcher_free(ctx);
 * @endcode
 *
 * Note that there is no error handling in the code above. Almost any of the called functions can fail.
 * You should check the return values in an actual code.
 */

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(_WIN32) || defined(_WIN64))
#	ifdef CHROMAPRINT_NODLL
#		define CHROMAPRINT_API
#	else
#		ifdef CHROMAPRINT_API_EXPORTS
#			define CHROMAPRINT_API __declspec(dllexport)
#		else
#			define CHROMAPRINT_API __declspec(dllimport)
#		endif
#	endif
#else
#   if __GNUC__ >= 4
#       define CHROMAPRINT_API __attribute__ ((visibility("default")))
#   else
#       define CHROMAPRINT_API
#   endif
#endif

#include <stdint.h>

struct ChromaprintContextPrivate;
typedef struct ChromaprintContextPrivate ChromaprintContext;

struct ChromaprintMatcherContextPrivate;
typedef struct ChromaprintMatcherContextPrivate ChromaprintMatcherContext;

#define CHROMAPRINT_VERSION_MAJOR 1
#define CHROMAPRINT_VERSION_MINOR 3
#define CHROMAPRINT_VERSION_PATCH 1

enum ChromaprintAlgorithm {
	CHROMAPRINT_ALGORITHM_TEST1 = 0,
	CHROMAPRINT_ALGORITHM_TEST2,
	CHROMAPRINT_ALGORITHM_TEST3,
	CHROMAPRINT_ALGORITHM_TEST4,
	CHROMAPRINT_ALGORITHM_TEST5,
	CHROMAPRINT_ALGORITHM_DEFAULT = CHROMAPRINT_ALGORITHM_TEST2,
};

/**
 * Return the version number of Chromaprint.
 */
CHROMAPRINT_API const char *chromaprint_get_version(void);

/**
 * Allocate and initialize the Chromaprint context.
 *
 * Note that when Chromaprint is compiled with FFTW, this function is
 * not reentrant and you need to call it only from one thread at a time.
 * This is not a problem when using FFmpeg or vDSP.
 *
 * @param algorithm the fingerprint algorithm version you want to use, or
 *		CHROMAPRINT_ALGORITHM_DEFAULT for the default algorithm
 *
 * @return ctx Chromaprint context pointer
 */
CHROMAPRINT_API ChromaprintContext *chromaprint_new(int algorithm);

/**
 * Deallocate the Chromaprint context.
 *
 * Note that when Chromaprint is compiled with FFTW, this function is
 * not reentrant and you need to call it only from one thread at a time.
 * This is not a problem when using FFmpeg or vDSP.
 *
 * @param[in] ctx Chromaprint context pointer
 */
CHROMAPRINT_API void chromaprint_free(ChromaprintContext *ctx);

/**
 * Return the fingerprint algorithm this context is configured to use.
 * @param[in] ctx Chromaprint context pointer
 * @return current algorithm version
 */
CHROMAPRINT_API int chromaprint_get_algorithm(ChromaprintContext *ctx);

/**
 * Set a configuration option for the selected fingerprint algorithm.
 *
 * DO NOT USE THIS FUNCTION IF YOU ARE PLANNING TO USE
 * THE GENERATED FINGERPRINTS WITH THE ACOUSTID SERVICE.
 *
 * Possible options:
 *  - silence_threshold: threshold for detecting silence, 0-32767
 *
 * @param[in] ctx Chromaprint context pointer
 * @param[in] name option name
 * @param[in] value option value
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_set_option(ChromaprintContext *ctx, const char *name, int value);

/**
 * Restart the computation of a fingerprint with a new audio stream.
 *
 * @param[in] ctx Chromaprint context pointer
 * @param[in] sample_rate sample rate of the audio stream (in Hz)
 * @param[in] num_channels numbers of channels in the audio stream (1 or 2)
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_start(ChromaprintContext *ctx, int sample_rate, int num_channels);

/**
 * Send audio data to the fingerprint calculator.
 *
 * @param[in] ctx Chromaprint context pointer
 * @param[in] data raw audio data, should point to an array of 16-bit signed
 *          integers in native byte-order
 * @param[in] size size of the data buffer (in samples)
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_feed(ChromaprintContext *ctx, const int16_t *data, int size);

/**
 * Process any remaining buffered audio data.
 *
 * @param[in] ctx Chromaprint context pointer
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_finish(ChromaprintContext *ctx);

/**
 * Return the calculated fingerprint as a compressed string.
 *
 * The caller is responsible for freeing the returned pointer using
 * chromaprint_dealloc().
 *
 * @param[in] ctx Chromaprint context pointer
 * @param[out] fingerprint pointer to a pointer, where a pointer to the allocated array
 *                 will be stored
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_get_fingerprint(ChromaprintContext *ctx, char **fingerprint);

/**
 * Return the calculated fingerprint as an array of 32-bit integers.
 *
 * The caller is responsible for freeing the returned pointer using
 * chromaprint_dealloc().
 *
 * @param[in] ctx Chromaprint context pointer
 * @param[out] fingerprint pointer to a pointer, where a pointer to the allocated array
 *                 will be stored
 * @param[out] size number of items in the returned raw fingerprint
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_get_raw_fingerprint(ChromaprintContext *ctx, uint32_t **fingerprint, int *size);

/**
 * Return 32-bit hash of the calculated fingerprint.
 *
 * See chromaprint_hash_fingerprint() for details on how to use the hash.
 *
 * @param[in] ctx Chromaprint context pointer
 * @param[out] hash pointer to a 32-bit integer where the hash will be stored
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_get_fingerprint_hash(ChromaprintContext *ctx, uint32_t *hash);

/**
 * Clear the current fingerprint, but allow more data to be processed.
 *
 * This is useful if you are processing a long stream and want to many
 * smaller fingerprints, instead of waiting for the entire stream to be
 * processed.
 *
 * @param[in] ctx Chromaprint context pointer
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_clear_fingerprint(ChromaprintContext *ctx);

/**
 * Compress and optionally base64-encode a raw fingerprint
 *
 * The caller is responsible for freeing the returned pointer using
 * chromaprint_dealloc().
 *
 * @param[in] fp pointer to an array of 32-bit integers representing the raw
 *        fingerprint to be encoded
 * @param[in] size number of items in the raw fingerprint
 * @param[in] algorithm Chromaprint algorithm version which was used to generate the
 *               raw fingerprint
 * @param[out] encoded_fp pointer to a pointer, where the encoded fingerprint will be
 *                stored
 * @param[out] encoded_size size of the encoded fingerprint in bytes
 * @param[in] base64 Whether to return binary data or base64-encoded ASCII data. The
 *            compressed fingerprint will be encoded using base64 with the
 *            URL-safe scheme if you set this parameter to 1. It will return
 *            binary data if it's 0.
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_encode_fingerprint(const uint32_t *fp, int size, int algorithm, char **encoded_fp, int *encoded_size, int base64);

/**
 * Uncompress and optionally base64-decode an encoded fingerprint
 *
 * The caller is responsible for freeing the returned pointer using
 * chromaprint_dealloc().
 *
 * @param[in] encoded_fp pointer to an encoded fingerprint
 * @param[in] encoded_size size of the encoded fingerprint in bytes
 * @param[out] fp pointer to a pointer, where the decoded raw fingerprint (array
 *        of 32-bit integers) will be stored
 * @param[out] size Number of items in the returned raw fingerprint
 * @param[out] algorithm Chromaprint algorithm version which was used to generate the
 *               raw fingerprint
 * @param[in] base64 Whether the encoded_fp parameter contains binary data or
 *            base64-encoded ASCII data. If 1, it will base64-decode the data
 *            before uncompressing the fingerprint.
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_decode_fingerprint(const char *encoded_fp, int encoded_size, uint32_t **fp, int *size, int *algorithm, int base64);

/**
 * Generate a single 32-bit hash for a raw fingerprint.
 *
 * If two fingerprints are similar, their hashes generated by this function
 * will also be similar. If they are significantly different, their hashes
 * will most likely be significantly different as well, but you can't rely
 * on that.
 *
 * You compare two hashes by counting the bits in which they differ. Normally
 * that would be something like POPCNT(hash1 XOR hash2), which returns a
 * number between 0 and 32. Anthing above 15 means the hashes are
 * completely different.
 *
 * @param[in] fp pointer to an array of 32-bit integers representing the raw
 *        fingerprint to be hashed
 * @param[in] size number of items in the raw fingerprint
 * @param[out] hash pointer to a 32-bit integer where the hash will be stored
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_hash_fingerprint(const uint32_t *fp, int size, uint32_t *hash);

/**
 * Create a new fingerprint matcher context.
 *
 * You should use chromaprint_matcher_free() to free the pointer.
 *
 * @return Chromaprint matcher context pointer or NULL on error
 */
CHROMAPRINT_API ChromaprintMatcherContext *chromaprint_matcher_new();

/**
 * Free the matcher context allocated by chromaprint_matcher_new().
 *
 * @param[in] ctx Chromaprint matcher context
 */
CHROMAPRINT_API void chromaprint_matcher_free(ChromaprintMatcherContext *ctx);

/**
 * Set one of the two fingerprints to be matched.
 *
 * @param[in] ctx Chromaprint matcher context
 * @param[in] idx 0 or 1, depending on which fingerprint you want to set
 * @param[in] fp base64-encoded fingerprint string
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_matcher_set_fingerprint(ChromaprintMatcherContext *ctx, int idx, const char *fp);

/**
 * Set one of the two fingerprints to be matched.
 *
 * @param[in] ctx Chromaprint matcher context
 * @param[in] idx 0 or 1, depending on which fingerprint you want to set
 * @param[in] fp raw fingerprint represented as an 32-bit int array
 * @param[in] size number of items in the fp array
 * @param[in] algorithm Chromaprint algorithm version which was used to generate the
 *               raw fingerprint
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_matcher_set_raw_fingerprint(ChromaprintMatcherContext *ctx, int idx, const uint32_t *fp, int size, int algorithm);

/**
 * Compare two fingerprints.
 *
 * The fingerprints should be set before with
 * chromaprint_matcher_set_fingerprint() or
 * chromaprint_matcher_set_raw_fingerprint().
 *
 * @param[in] ctx Chromaprint matcher context
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_matcher_run(ChromaprintMatcherContext *ctx);

/**
 * Get the number of matching segments.
 *
 * @param[in] ctx Chromaprint matcher context
 * @param[out] num_segments pointer to an int where the number of segments will be stored
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_matcher_get_num_segments(ChromaprintMatcherContext *ctx, int *num_segments);

/**
 * Get position details of a particular matching segment.
 *
 * This function returns values that are indexes in the raw fingerprint array.
 * For times in milliseconds, see chromaprint_matcher_get_segment_position_ms().
 *
 * @param[in] ctx Chromaprint matcher context
 * @param[in] idx segment number
 * @param[out] pos1 starting position of the segment in the first fingerprint
 * @param[out] pos2 starting position of the segment in the second fingerprint
 * @param[out] duration total duration of the segment
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_matcher_get_segment_position(ChromaprintMatcherContext *ctx, int idx, int *pos1, int *pos2, int *duration);

/**
 * Get time position details of a particular matching segment.
 *
 * This function returns values in milliseconds.
 * For raw position offsets, see chromaprint_matcher_get_segment_position().
 *
 * @param[in] ctx Chromaprint matcher context
 * @param[in] idx segment number
 * @param[out] pos1 starting time of the segment in the first fingerprint
 * @param[out] pos2 starting time of the segment in the second fingerprint
 * @param[out] duration total duration of the segment
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_matcher_get_segment_position_ms(ChromaprintMatcherContext *ctx, int idx, int *pos1, int *pos2, int *duration);

/**
 * Get the score of a particular matching segment.
 *
 * @param[in] ctx Chromaprint matcher context
 * @param[in] idx segment number
 * @param[out] score number between 0 (worst) and 100 (best)
 *
 * @return 0 on error, 1 on success
 */
CHROMAPRINT_API int chromaprint_matcher_get_segment_score(ChromaprintMatcherContext *ctx, int idx, int *score);

/**
 * Free memory allocated by any function from the Chromaprint API.
 *
 * @param ptr pointer to be deallocated
 */
CHROMAPRINT_API void chromaprint_dealloc(void *ptr);

#ifdef __cplusplus
}
#endif

#endif
