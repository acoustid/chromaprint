// Copyright (C) 2010-2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <cstring>
#include <chromaprint.h>
#include "fingerprinter.h"
#include "fingerprint_compressor.h"
#include "fingerprint_decompressor.h"
#include "fingerprint_matcher.h"
#include "fingerprinter_configuration.h"
#include "base64.h"
#include "simhash.h"
#include "debug.h"

using namespace chromaprint;

struct ChromaprintContextPrivate {
	ChromaprintContextPrivate(int algorithm)
		: algorithm(algorithm),
		  fingerprinter(CreateFingerprinterConfiguration(algorithm)) {}
	int algorithm;
	Fingerprinter fingerprinter;
};

struct ChromaprintMatcherContextPrivate {
	int algorithm = -1;
	std::unique_ptr<FingerprintMatcher> matcher;
	std::vector<uint32_t> fp[2];
};

extern "C" {

#define FAIL_IF(x, msg) if (x) { DEBUG(msg); return 0; }

#define STR(x) #x
#define VERSION_STR(major, minor, patch) \
	STR(major) "." STR(minor) "." STR(patch)

static const char *version_str = VERSION_STR(
	CHROMAPRINT_VERSION_MAJOR,
	CHROMAPRINT_VERSION_MINOR,
	CHROMAPRINT_VERSION_PATCH);

const char *chromaprint_get_version(void)
{
	return version_str;
}

ChromaprintContext *chromaprint_new(int algorithm)
{
	ChromaprintContextPrivate *ctx = new ChromaprintContextPrivate(algorithm);
	return (ChromaprintContext *) ctx;
}

void chromaprint_free(ChromaprintContext *c)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *) c;
	delete ctx;
}

int chromaprint_set_option(ChromaprintContext *c, const char *name, int value)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *) c;
	return ctx->fingerprinter.SetOption(name, value) ? 1 : 0;
}

int chromaprint_start(ChromaprintContext *c, int sample_rate, int num_channels)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *) c;
	return ctx->fingerprinter.Start(sample_rate, num_channels) ? 1 : 0;
}

int chromaprint_feed(ChromaprintContext *c, const int16_t *data, int length)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *) c;
	ctx->fingerprinter.Consume(data, length);
	return 1;
}

int chromaprint_finish(ChromaprintContext *c)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *) c;
	ctx->fingerprinter.Finish();
	return 1;
}

int chromaprint_get_fingerprint(ChromaprintContext *c, char **data)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *) c;
	std::string fingerprint = CompressFingerprint(ctx->fingerprinter.GetFingerprint(), ctx->algorithm);
	*data = (char *) malloc(GetBase64EncodedSize(fingerprint.size()) + 1);
	Base64Encode(fingerprint.begin(), fingerprint.end(), *data, true);
	return 1;
}

int chromaprint_get_raw_fingerprint(ChromaprintContext *c, uint32_t **data, int *size)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *) c;
	const auto fingerprint = ctx->fingerprinter.GetFingerprint();
	*data = (uint32_t *) malloc(sizeof(uint32_t) * fingerprint.size());
	if (!*data) {
		return 0;
	}
	*size = fingerprint.size();
	std::copy(fingerprint.begin(), fingerprint.end(), *data);
	return 1;
}

int chromaprint_get_fingerprint_hash(ChromaprintContext *c, uint32_t *hash)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *) c;
	*hash = SimHash(ctx->fingerprinter.GetFingerprint());
	return 1;
}

int chromaprint_clear_fingerprint(ChromaprintContext *c)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *) c;
	ctx->fingerprinter.ClearFingerprint();
	return 1;
}

int chromaprint_encode_fingerprint(const uint32_t *fp, int size, int algorithm, char **encoded_fp, int *encoded_size, int base64)
{
	std::vector<uint32_t> uncompressed(fp, fp + size);
	std::string encoded = CompressFingerprint(uncompressed, algorithm);
	if (base64) {
		encoded = Base64Encode(encoded);
	}
	*encoded_fp = (char *) malloc(encoded.size() + 1);
	*encoded_size = encoded.size();	
	std::copy(encoded.data(), encoded.data() + encoded.size() + 1, *encoded_fp);
	return 1;
}

int chromaprint_decode_fingerprint(const char *encoded_fp, int encoded_size, uint32_t **fp, int *size, int *algorithm, int base64)
{
	std::string encoded(encoded_fp, encoded_size);
	if (base64) {
		encoded = Base64Decode(encoded);
	}
	std::vector<uint32_t> uncompressed = DecompressFingerprint(encoded, algorithm);
	*fp = (uint32_t *) malloc(sizeof(uint32_t) * uncompressed.size());
	*size = uncompressed.size();
	std::copy(uncompressed.begin(), uncompressed.end(), *fp);
	return 1;
}

int chromaprint_hash_fingerprint(const uint32_t *fp, int size, uint32_t *hash)
{
	if (fp == NULL || size < 0 || hash == NULL) {
		return 0;
	}
	*hash = SimHash(fp, size);
	return 1;
}

ChromaprintMatcherContext *chromaprint_matcher_new()
{
	return new ChromaprintMatcherContext();
}

void chromaprint_matcher_free(ChromaprintMatcherContext *ctx)
{
	delete ctx;
}

int chromaprint_matcher_set_fingerprint(ChromaprintMatcherContext *ctx, int idx, const char *fp)
{
	FAIL_IF(!ctx, "context can't be NULL");
	FAIL_IF(idx < 0 || idx > 1, "idx can be only 0 or 1");

	int algorithm;
	ctx->fp[idx] = DecompressFingerprint(Base64Decode(fp), &algorithm);

	if (ctx->algorithm == -1) {
		ctx->matcher.reset(new FingerprintMatcher(CreateFingerprinterConfiguration(algorithm)));
	} else {
		FAIL_IF(algorithm != ctx->algorithm, "invalid fingerprint algorithm");
	}

	return 1;
}

int chromaprint_matcher_set_raw_fingerprint(ChromaprintMatcherContext *ctx, int idx, const uint32_t *fp, int size, int algorithm)
{
	FAIL_IF(!ctx, "context can't be NULL");
	FAIL_IF(idx < 0 || idx > 1, "idx can be only 0 or 1");

	if (ctx->algorithm == -1) {
		ctx->matcher.reset(new FingerprintMatcher(CreateFingerprinterConfiguration(algorithm)));
	} else {
		FAIL_IF(algorithm != ctx->algorithm, "invalid fingerprint algorithm");
	}

	ctx->fp[idx].assign(fp, fp + size);
	return 1;
}

int chromaprint_matcher_run(ChromaprintMatcherContext *ctx)
{
	FAIL_IF(!ctx, "context can't be NULL");
	FAIL_IF(ctx->fp[0].empty(), "fingerprint 0 is empty");
	FAIL_IF(ctx->fp[1].empty(), "fingerprint 1 is empty");

	return ctx->matcher->Match(ctx->fp[0], ctx->fp[1]) ? 1 : 0;
}

int chromaprint_matcher_get_num_segments(ChromaprintMatcherContext *ctx, int *num_segments)
{
	FAIL_IF(!ctx, "context can't be NULL");
	FAIL_IF(!num_segments, "num_segments can't be NULL");

	*num_segments = ctx->matcher->segments().size();
	return 1;
}

int chromaprint_matcher_get_segment_position(ChromaprintMatcherContext *ctx, int idx, int *pos1, int *pos2, int *duration)
{
	FAIL_IF(!ctx, "context can't be NULL");

	const auto &segments = ctx->matcher->segments();
	const int num_segments = segments.size();
	FAIL_IF(idx < 0 || idx >= num_segments, "invalid idx");

	*pos1 = segments[idx].pos1;
	*pos2 = segments[idx].pos2;
	*duration = segments[idx].duration;
	return 1;
}

int chromaprint_matcher_get_segment_position_ms(ChromaprintMatcherContext *ctx, int idx, int *pos1, int *pos2, int *duration)
{
	FAIL_IF(!ctx, "context can't be NULL");

	const auto &segments = ctx->matcher->segments();
	const int num_segments = segments.size();
	FAIL_IF(idx < 0 || idx >= num_segments, "invalid idx");

	*pos1 = round(1000 * ctx->matcher->GetHashTime(segments[idx].pos1));
	*pos2 = round(1000 * ctx->matcher->GetHashTime(segments[idx].pos2));
	*duration = round(1000 * ctx->matcher->GetHashTime(segments[idx].duration));
	return 1;
}

int chromaprint_matcher_get_segment_score(ChromaprintMatcherContext *ctx, int idx, int *score)
{
	FAIL_IF(!ctx, "context can't be NULL");

	const auto &segments = ctx->matcher->segments();
	const int num_segments = segments.size();
	FAIL_IF(idx < 0 || idx >= num_segments, "invalid idx");

	*score = segments[idx].public_score();
	return 1;
}

void chromaprint_dealloc(void *ptr)
{
	free(ptr);
}

}; // extern "C"
