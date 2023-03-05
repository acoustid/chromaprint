// Copyright (C) 2023  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_FINGERPRINT_COMPRESSOR_V2_H_
#define CHROMAPRINT_FINGERPRINT_COMPRESSOR_V2_H_

#include <cstdint>
#include <vector>
#include <string>

namespace chromaprint {

std::string CompressFingerprintV2(const std::vector<uint32_t> &data, int algorithm = 0);
std::vector<uint32_t> DecompressFingerprintV2(const std::string &data, int &algorithm);

}; // namespace chromaprint

#endif

