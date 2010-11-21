#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include "fingerprint_decompressor.h"
#include "utils.h"
#include "test_utils.h"

using namespace std;
using namespace Chromaprint;

#define NELEMS(x) (sizeof(x)/sizeof(x[0]))

TEST(FingerprintDecompressor, OneItemOneBit)
{
	int32_t expected[] = { 1 };
	char data[] = { 0, 0, 0, 1, 1 };

	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)));
	CheckFingerprints(value, expected, NELEMS(expected));
}


TEST(FingerprintDecompressor, OneItemThreeBits)
{
	int32_t expected[] = { 7 };
	char data[] = { 0, 0, 0, 1, 73, 0 };

	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)));
	CheckFingerprints(value, expected, NELEMS(expected));
}

TEST(FingerprintDecompressor, OneItemOneBitExcept)
{
	int32_t expected[] = { 1<<6 };
	char data[] = { 0, 0, 0, 1, 7, 0 };

	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)));
	CheckFingerprints(value, expected, NELEMS(expected));
}

TEST(FingerprintDecompressor, OneItemOneBitExcept2)
{
	int32_t expected[] = { 1<<8 };
	char data[] = { 0, 0, 0, 1, 7, 2 };

	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)));
	CheckFingerprints(value, expected, NELEMS(expected));
}

TEST(FingerprintDecompressor, TwoItems)
{
	int32_t expected[] = { 1, 0 };
	char data[] = { 0, 0, 0, 2, 65, 0 };

	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)));
	CheckFingerprints(value, expected, NELEMS(expected));
}

TEST(FingerprintDecompressor, TwoItemsNoChange)
{
	int32_t expected[] = { 1, 1 };
	char data[] = { 0, 0, 0, 2, 1, 0 };

	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)));
	CheckFingerprints(value, expected, NELEMS(expected));
}

