// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_FINGERPRINT_DECOMPRESSOR_H_
#define CHROMAPRINT_FINGERPRINT_DECOMPRESSOR_H_

#include <stdint.h>
#include <vector>
#include <string>
#include "bit_string_reader.h"

namespace chromaprint {

class FingerprintDecompressor
{
public:
	FingerprintDecompressor();
	std::vector<uint32_t> Decompress(const std::string &fingerprint, int *algorithm = 0);

private:

	bool ReadNormalBits(BitStringReader *reader);
	bool ReadExceptionBits(BitStringReader *reader);
	void UnpackBits();

	std::vector<uint32_t> m_result;
	std::vector<char> m_bits;
};

inline std::vector<uint32_t> DecompressFingerprint(const std::string &data, int *algorithm = 0)
{
	FingerprintDecompressor decompressor;
	return decompressor.Decompress(data, algorithm);
}

}; // namespace chromaprint

#endif
