// Copyright (C) 2023  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#include <algorithm>
#include <zstd.h>
#include "fingerprint_compressor_v2.h"

namespace chromaprint {

bool CompressFingerprintV2(const std::vector<uint32_t> &hashes, int algorithm, std::string &compressed)
{
    size_t num_hashes = hashes.size();

    std::string packed;
    packed.resize(4 * num_hashes);

    uint32_t last_hash = 0;
    for (size_t i = 0; i < num_hashes; i++) {
        uint32_t hash = hashes[i];
        uint32_t delta = hash ^ last_hash;
        last_hash = hash;
        packed[i] = delta >> 24;
        packed[i + num_hashes] = delta >> 16;
        packed[i + 2 * num_hashes] = delta >> 8;
        packed[i + 3 * num_hashes] = delta;
    }

    compressed.resize(ZSTD_compressBound(packed.size()) + 4);

    size_t compressed_size = ZSTD_compress(&compressed[0] + 4, compressed.size() - 4, &packed[0], packed.size(), 1);
    if (ZSTD_isError(compressed_size)) {
        return false;
    }

    compressed.resize(compressed_size + 4);

    int compression_version = 1;

    compressed[0] = (compression_version & 0x3) << 6 | (algorithm & 0x3f);
    compressed[1] = num_hashes >> 16;
    compressed[2] = num_hashes >> 8;
    compressed[3] = num_hashes;

    return true;
}

std::vector<uint32_t> DecompressFingerprintV2(const std::string &data, int &algorithm)
{
    if (data.size() < 4) {
        return std::vector<uint32_t>();
    }

    int compression_version = (data[0] >> 6) & 0x3;
    if (compression_version != 1) {
        return std::vector<uint32_t>();
    }

    algorithm = data[0] & 0x3f;
    size_t num_hashes = (data[1] << 16) | (data[2] << 8) | data[3];

    std::string packed;
    packed.resize(4 * num_hashes);

    size_t packed_size = ZSTD_decompress(&packed[0], packed.size(), &data[0] + 4, data.size() - 4);
    if (ZSTD_isError(packed_size)) {
        return std::vector<uint32_t>();
    }

    std::vector<uint32_t> hashes;
    hashes.resize(num_hashes);

    uint32_t last_hash = 0;
    for (size_t i = 0; i < num_hashes; i++) {
        uint32_t delta = (packed[i] << 24) | (packed[i + num_hashes] << 16) | (packed[i + 2 * num_hashes] << 8) | packed[i + 3 * num_hashes];
        uint32_t hash = last_hash ^ delta;
        last_hash = hash;
        hashes[i] = hash;
    }

    return hashes;
}

}; // namespace chromaprint

