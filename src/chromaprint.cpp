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

#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <chromaprint.h>
#include "fingerprinter.h"
#include "fingerprint_compressor.h"
#include "fingerprint_decompressor.h"
#include "fingerprinter_configuration.h"
#include "base64.h"
#include "simhash.h"

using namespace Chromaprint;

extern "C" {

struct ChromaprintContextPrivate {
	bool finished;
	int algorithm;
	Fingerprinter *fingerprinter;
	std::vector<uint32_t> fingerprint;
};

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
	ChromaprintContextPrivate *ctx = new ChromaprintContextPrivate();
	ctx->finished = false;
	ctx->algorithm = algorithm;
	ctx->fingerprinter = new Fingerprinter(CreateFingerprinterConfiguration(algorithm));
	return (ChromaprintContext *)ctx;
}

void chromaprint_free(ChromaprintContext *c)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *)c;
	delete ctx->fingerprinter;
	delete ctx;
}

int chromaprint_set_option(ChromaprintContext *c, const char *name, int value)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *)c;
	return ctx->fingerprinter->SetOption(name, value) ? 1 : 0;
}

int chromaprint_start(ChromaprintContext *c, int sample_rate, int num_channels)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *)c;
	ctx->finished = false;
	return ctx->fingerprinter->Start(sample_rate, num_channels) ? 1 : 0;
}

int chromaprint_feed(ChromaprintContext *c, void *data, int length)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *)c;
	ctx->fingerprinter->Consume((short *)data, length);
	return 1;
}

int chromaprint_finish(ChromaprintContext *c)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *)c;
	ctx->fingerprint = ctx->fingerprinter->Finish();
	ctx->finished = true;
	return 1;
}

int chromaprint_get_fingerprint(ChromaprintContext *c, char **data)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *)c;
	if (!ctx->finished) {
		return 0;
	}
	std::string fp = Chromaprint::Base64Encode(Chromaprint::CompressFingerprint(ctx->fingerprint, ctx->algorithm));
	*data = (char *)malloc(fp.size() + 1);
	if (!*data) {
		return 0;
	}
	copy(fp.begin(), fp.end(), *data);
	(*data)[fp.size()] = 0;
	return 1;
}

int chromaprint_get_raw_fingerprint(ChromaprintContext *c, uint32_t **data, int *size)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *)c;
	if (!ctx->finished) {
		return 0;
	}
	*data = (uint32_t *) malloc(sizeof(uint32_t) * ctx->fingerprint.size());
	if (!*data) {
		return 0;
	}
	*size = ctx->fingerprint.size();
	copy(ctx->fingerprint.begin(), ctx->fingerprint.end(), *data);
	return 1;
}

int chromaprint_get_fingerprint_hash(ChromaprintContext *c, uint32_t *hash)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *)c;
	if (!ctx->finished) {
		return 0;
	}
	*hash = SimHash(ctx->fingerprint);
	return 1;
}

int chromaprint_encode_fingerprint(const uint32_t *fp, int size, int algorithm, char **encoded_fp, int *encoded_size, int base64)
{
	std::vector<uint32_t> uncompressed(fp, fp + size);
	std::string encoded = Chromaprint::CompressFingerprint(uncompressed, algorithm);
	if (base64) {
		encoded = Chromaprint::Base64Encode(encoded);
	}
	*encoded_fp = (char *) malloc(encoded.size() + 1);
	*encoded_size = encoded.size();	
	memcpy(*encoded_fp, encoded.c_str(), encoded.size() + 1);
	return 1;
}

int chromaprint_decode_fingerprint(const char *encoded_fp, int encoded_size, uint32_t **fp, int *size, int *algorithm, int base64)
{
	std::string encoded(encoded_fp, encoded_size);
	if (base64) {
		encoded = Chromaprint::Base64Decode(encoded);
	}
	std::vector<uint32_t> uncompressed = Chromaprint::DecompressFingerprint(encoded, algorithm);
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

void chromaprint_dealloc(void *ptr)
{
	free(ptr);
}

}
