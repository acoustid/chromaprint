#include <gtest/gtest.h>
#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <vector>
#include <fstream>
#include "chromaprint.h"

using namespace std;

TEST(API, Test2SilenceFp)
{
	short zeroes[1024];
	fill(zeroes, zeroes + 1024, 0);

	ChromaprintContext *ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_TEST2);
	chromaprint_start(ctx, 44100, 1);
	for (int i = 0; i < 130; i++) {
		chromaprint_feed(ctx, zeroes, 1024);
	}

	char *fp;

	chromaprint_finish(ctx);
	chromaprint_get_fingerprint(ctx, &fp);

	ASSERT_EQ(18, strlen(fp));
	EXPECT_EQ(string("AQAAAkmUaEkSRZEGAA"), string(fp));
}

TEST(API, Test2SilenceRawFp)
{
	short zeroes[1024];
	fill(zeroes, zeroes + 1024, 0);

	ChromaprintContext *ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_TEST2);
	chromaprint_start(ctx, 44100, 1);
	for (int i = 0; i < 130; i++) {
		chromaprint_feed(ctx, zeroes, 1024);
	}

	int32_t *fp;
	int length;

	chromaprint_finish(ctx);
	chromaprint_get_raw_fingerprint(ctx, (void **)&fp, &length);

	ASSERT_EQ(2, length);
	EXPECT_EQ(627964279, fp[0]);
	EXPECT_EQ(627964279, fp[1]);
}
