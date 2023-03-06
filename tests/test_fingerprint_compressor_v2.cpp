#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include "fingerprint_compressor_v2.h"
#include "test_utils.h"

using namespace chromaprint;

TEST(FingerprintCompressorV2, Compress)
{
    std::vector<uint32_t> original;
    for (int i = 0; i < 100; i++) {
        original.push_back(i);
    }

	std::string compressed = CompressFingerprintV2(original, 1);

    EXPECT_NE(compressed.size(), 0u);
    EXPECT_LT(compressed.size(), 4 * original.size());

    int algo = -1;
    auto decompressed = DecompressFingerprintV2(compressed, algo);
    EXPECT_EQ(algo, 1);
    EXPECT_EQ(decompressed.size(), original.size());
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(decompressed[i], original[i]);
    }
}
