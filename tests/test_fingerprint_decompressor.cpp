#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include "fingerprint_decompressor.h"
#include "utils/base64.h"
#include "utils.h"
#include "test_utils.h"

using namespace chromaprint;

TEST(FingerprintDecompressor, Header)
{
	std::string data = Base64Decode("AQAAEwkjrUmSJQpUHflR9mjSJMdZpcO_Imdw9dCO9Clu4_wQPvhCB01w6xAtXNcAp5RASgDBhDSCGGIAcwA");

	size_t size = 0;
	int algorithm = 0;
	ASSERT_EQ(true, DecompressFingerprintHeader(data, size, algorithm));
	ASSERT_EQ(19, size);
	ASSERT_EQ(1, algorithm);
}

TEST(FingerprintDecompressor, OneItemOneBit)
{
	uint32_t expected[] = { 1 };
	char data[] = { 0, 0, 0, 1, 1 };

	int algorithm = 1;
	std::vector<uint32_t> value;
	ASSERT_EQ(true, DecompressFingerprint(std::string(data, NELEMS(data)), value, algorithm));
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}


TEST(FingerprintDecompressor, OneItemThreeBits)
{
	uint32_t expected[] = { 7 };
	char data[] = { 0, 0, 0, 1, 73, 0 };

	int algorithm = 1;
	std::vector<uint32_t> value;
	ASSERT_EQ(true, DecompressFingerprint(std::string(data, NELEMS(data)), value, algorithm));
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}

TEST(FingerprintDecompressor, OneItemOneBitExcept)
{
	uint32_t expected[] = { 1<<6 };
	char data[] = { 0, 0, 0, 1, 7, 0 };

	int algorithm = 1;
	std::vector<uint32_t> value;
	ASSERT_EQ(true, DecompressFingerprint(std::string(data, NELEMS(data)), value, algorithm));
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}

TEST(FingerprintDecompressor, OneItemOneBitExcept2)
{
	uint32_t expected[] = { 1<<8 };
	char data[] = { 0, 0, 0, 1, 7, 2 };

	int algorithm = 1;
	std::vector<uint32_t> value;
	ASSERT_EQ(true, DecompressFingerprint(std::string(data, NELEMS(data)), value, algorithm));
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}

TEST(FingerprintDecompressor, OneItemOneBitExceptWithTrailingData)
{
	uint32_t expected[] = { 1<<6 };
	char data[] = { 0, 0, 0, 1, 7, 0, char(255), char(255), char(255), char(255) };

	int algorithm = 1;
	std::vector<uint32_t> value;
	ASSERT_EQ(true, DecompressFingerprint(std::string(data, NELEMS(data)), value, algorithm));
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}

TEST(FingerprintDecompressor, TwoItems)
{
	uint32_t expected[] = { 1, 0 };
	char data[] = { 0, 0, 0, 2, 65, 0 };

	int algorithm = 1;
	std::vector<uint32_t> value;
	ASSERT_EQ(true, DecompressFingerprint(std::string(data, NELEMS(data)), value, algorithm));
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}

TEST(FingerprintDecompressor, TwoItemsNoChange)
{
	uint32_t expected[] = { 1, 1 };
	char data[] = { 0, 0, 0, 2, 1, 0 };

	int algorithm = 1;
	std::vector<uint32_t> value;
	ASSERT_EQ(true, DecompressFingerprint(std::string(data, NELEMS(data)), value, algorithm));
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}

TEST(FingerprintDecompressor, Invalid1)
{
	uint32_t expected[] = { 0 };
	char data[] = { 0, char(255), char(255), char(255) };

	int algorithm = 1;
	std::vector<uint32_t> value;
	ASSERT_EQ(false, DecompressFingerprint(std::string(data, NELEMS(data)), value, algorithm));
	CheckFingerprints(value, expected, 0);
	ASSERT_EQ(1, algorithm);
}

TEST(FingerprintDecompressor, Long)
{
	int32_t expected[] = { -587455133,-591649759,-574868448,-576973520,-543396544,1330439488,1326360000,1326355649,1191625921,1192674515,1194804466,1195336818,1165981042,1165956451,1157441379,1157441299,1291679571,1291673457,1170079601 };
	std::string data = Base64Decode("AQAAEwkjrUmSJQpUHflR9mjSJMdZpcO_Imdw9dCO9Clu4_wQPvhCB01w6xAtXNcAp5RASgDBhDSCGGIAcwA");

	int algorithm = 2;
	std::vector<uint32_t> value;
	ASSERT_EQ(true, DecompressFingerprint(data, value, algorithm));
	CheckFingerprints(value, (uint32_t *) expected, NELEMS(expected));
	ASSERT_EQ(1, algorithm);
}

TEST(FingerprintDecompressor, BitAtPosition32)
{
	// Five distances of 6 and one of 2 reach bit 32 exactly.
	uint32_t expected[] = { 0xA0820820 };
	char data[] = { 0, 0, 0, 1, char(182), 109, 1 };

	int algorithm = 1;
	std::vector<uint32_t> value;
	ASSERT_EQ(true, DecompressFingerprint(std::string(data, NELEMS(data)), value, algorithm));
	CheckFingerprints(value, expected, NELEMS(expected));
}

TEST(FingerprintDecompressor, BitPastPosition32)
{
	// Six distances of 6 reach bit 36.
	char data[] = { 0, 0, 0, 1, char(182), 109, 3 };

	int algorithm = 1;
	std::vector<uint32_t> value;
	ASSERT_EQ(false, DecompressFingerprint(std::string(data, NELEMS(data)), value, algorithm));
}

TEST(FingerprintDecompressor, ExceptionPastPosition32)
{
	// A distance of 7 + 26 reaches bit 33.
	char data[] = { 0, 0, 0, 1, 7, 26 };

	int algorithm = 1;
	std::vector<uint32_t> value;
	ASSERT_EQ(false, DecompressFingerprint(std::string(data, NELEMS(data)), value, algorithm));
}

TEST(FingerprintDecompressor, SecondItemPastPosition32)
{
	// The first item is valid, the second reaches bit 36.
	char data[] = { 0, 0, 0, 2, char(129), 109, char(219), 0 };

	int algorithm = 1;
	std::vector<uint32_t> value;
	ASSERT_EQ(false, DecompressFingerprint(std::string(data, NELEMS(data)), value, algorithm));
}

TEST(FingerprintDecompressor, SecondItemExceptionPastPosition32)
{
	// The first item takes an exception of 3, the second 7 + 26.
	char data[] = { 0, 0, 0, 2, char(199), 1, 67, 3 };

	int algorithm = 1;
	std::vector<uint32_t> value;
	ASSERT_EQ(false, DecompressFingerprint(std::string(data, NELEMS(data)), value, algorithm));
}

TEST(FingerprintDecompressor, ZeroItemsWithTrailingData)
{
	char data[][8] = {
		{ 1, 0, 0, 0, 0, 0, 0, 0 },
		{ 1, 0, 0, 0, char(255), char(255), char(255), char(255) },
		{ 1, 0, 0, 0, char(182), 109, 3, 0 },
	};

	for (size_t i = 0; i < NELEMS(data); i++) {
		int algorithm = 0;
		std::vector<uint32_t> value;
		ASSERT_EQ(true, DecompressFingerprint(std::string(data[i], NELEMS(data[i])), value, algorithm));
		ASSERT_EQ(0, value.size());
		ASSERT_EQ(1, algorithm);
	}
}

TEST(FingerprintDecompressor, Corrupted)
{
	// Every truncation and every single-bit flip of a valid fingerprint
	// either fails or decodes to as many items as the header says.
	const std::string data = Base64Decode("AQAAEwkjrUmSJQpUHflR9mjSJMdZpcO_Imdw9dCO9Clu4_wQPvhCB01w6xAtXNcAp5RASgDBhDSCGGIAcwA");

	auto check = [](const std::string &input) {
		size_t size = 0;
		int algorithm = 0;
		std::vector<uint32_t> value;
		if (DecompressFingerprint(input, value, algorithm)) {
			ASSERT_EQ(true, DecompressFingerprintHeader(input, size, algorithm));
			ASSERT_EQ(size, value.size());
		}
	};

	for (size_t len = 0; len < data.size(); len++) {
		check(data.substr(0, len));
	}
	for (size_t bit = 0; bit < data.size() * 8; bit++) {
		std::string corrupted = data;
		corrupted[bit / 8] ^= char(1 << (bit % 8));
		check(corrupted);
	}
}
