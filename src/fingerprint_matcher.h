// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_FINGERPRINT_MATCHER_H_
#define CHROMAPRINT_FINGERPRINT_MATCHER_H_

#include <vector>
#include <memory>
#include <cstdint>

namespace chromaprint {

class FingerprinterConfiguration;

struct Segment
{
	size_t pos1;
	size_t pos2;
	size_t duration;
	double score;
	Segment(size_t pos1, size_t pos2, size_t duration, double score)
		: pos1(pos1), pos2(pos2), duration(duration), score(score) {}
};

class FingerprintMatcher
{
public:
	FingerprintMatcher(FingerprinterConfiguration *config);

	// Anything above this is not considered a match.
	void set_match_threshold(double t) { m_match_threshold = t; }
	double match_threshold() const { return m_match_threshold; }
	static constexpr double kDefaultMatchThreshold = 10.0;

	bool Match(const std::vector<uint32_t> &fp1, const std::vector<uint32_t> &fp2);
	bool Match(const uint32_t fp1_data[], size_t fp1_size, const uint32_t fp2_data[], size_t fp2_size);

	double GetHashTime(size_t i) const;
	double GetHashDuration(size_t i) const;

	const std::vector<Segment> &segments() const { return m_segments; };

private:
	std::unique_ptr<FingerprinterConfiguration> m_config;
	std::vector<uint32_t> m_offsets;
	std::vector<uint32_t> m_histogram;
	std::vector<std::pair<uint32_t, uint32_t>> m_best_alignments;
	std::vector<Segment> m_segments;
	double m_match_threshold = kDefaultMatchThreshold;
};

}; // namespace chromaprint

#endif
