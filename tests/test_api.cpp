#include <gtest/gtest.h>
#include <stdint.h>
#include <algorithm>
#include <vector>
#include <fstream>
#include "chromaprint.h"
#include "test_utils.h"
#include "utils/scope_exit.h"

namespace chromaprint {

TEST(API, TestFp) {
	std::vector<short> data = LoadAudioFile("data/test_stereo_44100.raw");

	ChromaprintContext *ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_TEST2);
	ASSERT_NE(nullptr, ctx);
	SCOPE_EXIT(chromaprint_free(ctx));

	ASSERT_EQ(1, chromaprint_get_num_channels(ctx));
	ASSERT_EQ(11025, chromaprint_get_sample_rate(ctx));

	ASSERT_EQ(1, chromaprint_start(ctx, 44100, 1));
	ASSERT_EQ(1, chromaprint_feed(ctx, data.data(), data.size()));

	char *fp;
	uint32_t fp_hash;

	ASSERT_EQ(1, chromaprint_finish(ctx));
	ASSERT_EQ(1, chromaprint_get_fingerprint(ctx, &fp));
	SCOPE_EXIT(chromaprint_dealloc(fp));
	ASSERT_EQ(1, chromaprint_get_fingerprint_hash(ctx, &fp_hash));

	EXPECT_EQ(std::string("AQAAC0kkZUqYREkUnFAXHk8uuMZl6EfO4zu-4ABKFGESWIIMEQE"), std::string(fp));
	ASSERT_EQ(3732003127, fp_hash);
}

TEST(API, Test2SilenceFp)
{
	short zeroes[1024];
	std::fill(zeroes, zeroes + 1024, 0);

	ChromaprintContext *ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_TEST2);
	ASSERT_NE(nullptr, ctx);
	SCOPE_EXIT(chromaprint_free(ctx));

	ASSERT_EQ(1, chromaprint_start(ctx, 44100, 1));
	for (int i = 0; i < 130; i++) {
		ASSERT_EQ(1, chromaprint_feed(ctx, zeroes, 1024));
	}

	char *fp;
	uint32_t fp_hash;

	ASSERT_EQ(1, chromaprint_finish(ctx));
	ASSERT_EQ(1, chromaprint_get_fingerprint(ctx, &fp));
	SCOPE_EXIT(chromaprint_dealloc(fp));
	ASSERT_EQ(1, chromaprint_get_fingerprint_hash(ctx, &fp_hash));

	ASSERT_EQ(18, strlen(fp));
	EXPECT_EQ(std::string("AQAAA0mUaEkSRZEGAA"), std::string(fp));
	ASSERT_EQ(627964279, fp_hash);
}

TEST(API, Test2SilenceRawFp)
{
	short zeroes[1024];
	std::fill(zeroes, zeroes + 1024, 0);

	ChromaprintContext *ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_TEST2);
	ASSERT_NE(nullptr, ctx);
	SCOPE_EXIT(chromaprint_free(ctx));

	ASSERT_EQ(1, chromaprint_start(ctx, 44100, 1));
	for (int i = 0; i < 130; i++) {
		ASSERT_EQ(1, chromaprint_feed(ctx, zeroes, 1024));
	}

	uint32_t *fp;
	int length;

	ASSERT_EQ(1, chromaprint_finish(ctx));
	ASSERT_EQ(1, chromaprint_get_raw_fingerprint(ctx, &fp, &length));
	SCOPE_EXIT(chromaprint_dealloc(fp));

	ASSERT_EQ(3, length);
	EXPECT_EQ(627964279, fp[0]);
	EXPECT_EQ(627964279, fp[1]);
	EXPECT_EQ(627964279, fp[2]);
}

TEST(API, TestEncodeFingerprint)
{
	uint32_t fingerprint[] = { 1, 0 };
	char expected[] = { 55, 0, 0, 2, 65, 0 };

	char *encoded;
	int encoded_size;
	ASSERT_EQ(1, chromaprint_encode_fingerprint(fingerprint, 2, 55, &encoded, &encoded_size, 0));
	SCOPE_EXIT(chromaprint_dealloc(encoded));

	ASSERT_EQ(6, encoded_size);
	for (int i = 0; i < encoded_size; i++) {
		ASSERT_EQ(expected[i], encoded[i]) << "Different at " << i;
	}
}

TEST(API, TestEncodeFingerprintBase64)
{
	uint32_t fingerprint[] = { 1, 0 };
	char expected[] = "NwAAAkEA";

	char *encoded;
	int encoded_size;
	ASSERT_EQ(1, chromaprint_encode_fingerprint(fingerprint, 2, 55, &encoded, &encoded_size, 1));
	SCOPE_EXIT(chromaprint_dealloc(encoded));

	ASSERT_EQ(8, encoded_size);
	ASSERT_STREQ(expected, encoded);
}

TEST(API, TestDecodeFingerprint)
{
	char data[] = { 55, 0, 0, 2, 65, 0 };

	uint32_t *fingerprint;
	int size;
	int algorithm;
	ASSERT_EQ(1, chromaprint_decode_fingerprint(data, 6, &fingerprint, &size, &algorithm, 0));
	SCOPE_EXIT(chromaprint_dealloc(fingerprint));

	ASSERT_EQ(2, size);
	ASSERT_EQ(55, algorithm);
	ASSERT_EQ(1, fingerprint[0]);
	ASSERT_EQ(0, fingerprint[1]);
}

TEST(API, TestHashFingerprint)
{
	uint32_t fingerprint[] = { 19681, 22345, 312312, 453425 };
    uint32_t hash;

    ASSERT_EQ(0, chromaprint_hash_fingerprint(NULL, 4, &hash));
    ASSERT_EQ(0, chromaprint_hash_fingerprint(fingerprint, -1, &hash));
    ASSERT_EQ(0, chromaprint_hash_fingerprint(fingerprint, 4, NULL));

    ASSERT_EQ(1, chromaprint_hash_fingerprint(fingerprint, 4, &hash));
    ASSERT_EQ(17249, hash);
}

}; // namespace chromaprint
