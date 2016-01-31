#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include "fingerprint_decompressor.h"
#include "base64.h"
#include "utils.h"
#include "test_utils.h"

using namespace std;
using namespace Chromaprint;

TEST(FingerprintDecompressor, OneItemOneBit)
{
	int32_t expected[] = { 1 };
	char data[] = { 0, 0, 0, 1, 1 };

	int algorithm = 1;
	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)), &algorithm);
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}


TEST(FingerprintDecompressor, OneItemThreeBits)
{
	int32_t expected[] = { 7 };
	char data[] = { 0, 0, 0, 1, 73, 0 };

	int algorithm = 1;
	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)), &algorithm);
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}

TEST(FingerprintDecompressor, OneItemOneBitExcept)
{
	int32_t expected[] = { 1<<6 };
	char data[] = { 0, 0, 0, 1, 7, 0 };

	int algorithm = 1;
	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)), &algorithm);
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}

TEST(FingerprintDecompressor, OneItemOneBitExcept2)
{
	int32_t expected[] = { 1<<8 };
	char data[] = { 0, 0, 0, 1, 7, 2 };

	int algorithm = 1;
	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)), &algorithm);
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}

TEST(FingerprintDecompressor, TwoItems)
{
	int32_t expected[] = { 1, 0 };
	char data[] = { 0, 0, 0, 2, 65, 0 };

	int algorithm = 1;
	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)), &algorithm);
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}

TEST(FingerprintDecompressor, TwoItemsNoChange)
{
	int32_t expected[] = { 1, 1 };
	char data[] = { 0, 0, 0, 2, 1, 0 };

	int algorithm = 1;
	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)), &algorithm);
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}

TEST(FingerprintDecompressor, Invalid1)
{
	int32_t expected[] = { 0 };
	char data[] = { 0, char(255), char(255), char(255) };

	int algorithm = 1;
	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)), &algorithm);
	CheckFingerprints(value, expected, 0);
	ASSERT_EQ(0, algorithm);
}

TEST(FingerprintDecompressor, Long)
{
	int32_t expected[] = { -587455133,-591649759,-574868448,-576973520,-543396544,1330439488,1326360000,1326355649,1191625921,1192674515,1194804466,1195336818,1165981042,1165956451,1157441379,1157441299,1291679571,1291673457,1170079601 };
	std::string data = Base64Decode("AQAAEwkjrUmSJQpUHflR9mjSJMdZpcO_Imdw9dCO9Clu4_wQPvhCB01w6xAtXNcAp5RASgDBhDSCGGIAcwA");

	int algorithm = 2;
	vector<int32_t> value = DecompressFingerprint(data, &algorithm);
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(1, algorithm);
}
