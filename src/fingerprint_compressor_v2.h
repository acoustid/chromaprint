// Copyright (C) 2023  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_FINGERPRINT_COMPRESSOR_V2_H_
#define CHROMAPRINT_FINGERPRINT_COMPRESSOR_V2_H_

#include <cstdint>
#include <vector>
#include <string>

namespace chromaprint {

bool CompressFingerprintV2(const std::vector<uint32_t> &hashes, int algorithm, std::string &data);

inline std::string CompressFingerprintV2(const std::vector<uint32_t> &hashes, int algorithm)
{
    std::string data;
    bool res = CompressFingerprintV2(hashes, algorithm, data);
    if (!res) {
        data.clear();
    }
    return data;
}

std::vector<uint32_t> DecompressFingerprintV2(const std::string &data, int &algorithm);

}; // namespace chromaprint

#endif

