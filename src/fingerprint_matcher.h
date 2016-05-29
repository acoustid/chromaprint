// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_FINGERPRINT_MATCHER_H_
#define CHROMAPRINT_FINGERPRINT_MATCHER_H_

#include <vector>
#include <cstdint>

namespace Chromaprint
{

class FingerprinterConfiguration;

class FingerprintMatcher
{
public:
	FingerprintMatcher(FingerprinterConfiguration *config);

	bool Match(std::vector<int32_t> &fp1, std::vector<int32_t> &fp2);

	double GetHashTime(int i) const;
	double GetHashDuration(int i) const;

private:
	FingerprinterConfiguration *m_config;
	std::vector<uint32_t> m_offsets;
	std::vector<uint32_t> m_histogram;
	std::vector<std::pair<uint32_t, uint32_t>> m_best_alignments;
};

};

#endif
