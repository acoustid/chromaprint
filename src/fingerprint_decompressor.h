// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_FINGERPRINT_DECOMPRESSOR_H_
#define CHROMAPRINT_FINGERPRINT_DECOMPRESSOR_H_

#include <cstdint>
#include <vector>
#include <string>

namespace chromaprint {

class FingerprintDecompressor
{
public:
	FingerprintDecompressor();
	bool DecompressHeader(const std::string &fingerprint);
	bool Decompress(const std::string &fingerprint);

	std::vector<uint32_t> GetOutput() const { return m_output; }
	size_t GetSize() const { return m_size; }
	int GetAlgorithm() const { return m_algorithm; }

private:
	void UnpackBits();
	std::vector<uint32_t> m_output;
	size_t m_size { 0 };
	int m_algorithm { -1 };
	std::vector<unsigned char> m_bits;
	std::vector<unsigned char> m_exceptional_bits;
};

inline bool DecompressFingerprint(const std::string &input, std::vector<uint32_t> &output, int &algorithm)
{
	FingerprintDecompressor decompressor;
	auto ok = decompressor.Decompress(input);
	if (ok) {
		output = decompressor.GetOutput();
		algorithm = decompressor.GetAlgorithm();
	}
	return ok;
}

inline bool DecompressFingerprintHeader(const std::string &input, size_t &size, int &algorithm)
{
	FingerprintDecompressor decompressor;
	auto ok = decompressor.DecompressHeader(input);
	if (ok) {
		size = decompressor.GetSize();
		algorithm = decompressor.GetAlgorithm();
	}
	return ok;
}

}; // namespace chromaprint

#endif
