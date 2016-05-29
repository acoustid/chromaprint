// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#include <algorithm>
#include "fingerprint_matcher.h"
#include "fingerprinter_configuration.h"
#include "utils.h"
#include "utils/gaussian_filter.h"
#include "utils/gradient.h"
#include "debug.h"

namespace chromaprint
{

/* fingerprint matcher settings */
#define ACOUSTID_MAX_BIT_ERROR 2
#define ACOUSTID_MAX_ALIGN_OFFSET 120
#define ACOUSTID_QUERY_START 80
#define ACOUSTID_QUERY_LENGTH 120
#define ACOUSTID_QUERY_BITS 28
#define ACOUSTID_QUERY_MASK (((1<<ACOUSTID_QUERY_BITS)-1)<<(32-ACOUSTID_QUERY_BITS))
#define ACOUSTID_QUERY_STRIP(x) ((x) & ACOUSTID_QUERY_MASK)

#define ALIGN_BITS 12
#define ALIGN_MASK ((1 << ALIGN_BITS) - 1)
#define ALIGN_STRIP(x) ((uint32_t)(x) >> (32 - ALIGN_BITS))

#define UNIQ_BITS 16
#define UNIQ_MASK ((1 << MATCH_BITS) - 1)
#define UNIQ_STRIP(x) ((uint32_t)(x) >> (32 - MATCH_BITS))

FingerprintMatcher::FingerprintMatcher(FingerprinterConfiguration *config)
	: m_config(config)
{
}

double FingerprintMatcher::GetHashTime(int i) const {
	const auto frame_step = m_config->frame_size() - m_config->frame_overlap();
	return i * frame_step * 1.0 / m_config->sample_rate();
}

double FingerprintMatcher::GetHashDuration(int i) const {
	const auto frame_size = m_config->frame_size();
	const auto frame_step = frame_size - m_config->frame_overlap();
	return ((i + (m_config->num_filter_coefficients() - 1) + (m_config->max_filter_width() - 1)) * frame_step + frame_size) * 1.0 / m_config->sample_rate();
}

bool FingerprintMatcher::Match(std::vector<uint32_t> &fp1, std::vector<uint32_t> &fp2)
{
	const uint32_t hash_shift = 32 - ALIGN_BITS;
	const uint32_t hash_mask = ((1u << ALIGN_BITS) - 1) << hash_shift;
	const uint32_t offset_mask = (1u << (32 - ALIGN_BITS - 1)) - 1;
	const uint32_t source_mask = 1u << (32 - ALIGN_BITS - 1);

	const auto fp1_size = fp1.size();
	const auto fp2_size = fp2.size();

	DEBUG("duration1 " << GetHashDuration(fp1_size));
	DEBUG("duration2 " << GetHashDuration(fp2_size));

	if (fp1_size + 1 >= offset_mask) {
		DEBUG("chromaprint::FingerprintMatcher::Match() -- Fingerprint 1 too long.");
		return false;
	}
	if (fp2_size + 1 >= offset_mask) {
		DEBUG("chromaprint::FingerprintMatcher::Match() -- Fingerprint 2 too long.");
		return false;
	}

	m_offsets.clear();
	m_offsets.reserve(fp1_size + fp2_size);
	for (size_t i = 0; i < fp1_size; i++) {
		m_offsets.push_back((ALIGN_STRIP(fp1[i]) << hash_shift) | (i & offset_mask));
	}
	for (size_t i = 0; i < fp2_size; i++) {
		m_offsets.push_back((ALIGN_STRIP(fp2[i]) << hash_shift) | (i & offset_mask) | source_mask);
	}
	std::sort(m_offsets.begin(), m_offsets.end());

	m_histogram.assign(fp1_size + fp2_size, 0);
	for (auto it = m_offsets.cbegin(); it != m_offsets.cend(); ++it) {
		const uint32_t hash = (*it) & hash_mask;
		const uint32_t offset1 = (*it) & offset_mask;
		const uint32_t source1 = (*it) & source_mask;
		if (source1 != 0) {
			// if we got hash from fp2, it means there is no hash from fp1,
			// because if there was, it would be first
			continue;
		}
		auto it2 = it;
		while (++it2 != m_offsets.end()) {
			const uint32_t hash2 = (*it2) & hash_mask;
			if (hash != hash2) {
				break;
			}
			const uint32_t offset2 = (*it2) & offset_mask;
			const uint32_t source2 = (*it2) & source_mask;
			if (source2 != 0) {
				const size_t offset_diff = offset1 + fp2_size - offset2;
				m_histogram[offset_diff] += 1;
			}
		}
	}

	m_best_alignments.clear();
	const auto histogram_size = m_histogram.size();
	for (size_t i = 0; i < histogram_size; i++) {
		const uint32_t count = m_histogram[i];
		if (m_histogram[i] > 1) {
			const bool is_peak_left = (i > 0) ? m_histogram[i - 1] <= count : true;
			const bool is_peak_right = (i < histogram_size - 1) ? m_histogram[i + 1] <= count : true;
			if (is_peak_left && is_peak_right) {
				m_best_alignments.push_back(std::make_pair(count, i));
			}
		}
	}
	std::sort(m_best_alignments.rbegin(), m_best_alignments.rend());

	for (const auto &item : m_best_alignments) {
		const auto count = item.first;
		const int offset_diff = item.second - fp2_size;
		DEBUG("found " << count << " matches at offset " << offset_diff);

		const size_t offset1 = offset_diff > 0 ? offset_diff : 0;
		const size_t offset2 = offset_diff < 0 ? -offset_diff : 0;

		auto it1 = fp1.begin() + offset1;
		auto it2 = fp2.begin() + offset2;

		const auto size = std::min(std::distance(it1, fp1.end()), std::distance(it2, fp2.end()));
		std::vector<float> bit_counts(size);
		for (size_t i = 0; i < size; i++) {
			bit_counts[i] = HammingDistance(*it1++, *it2++) + rand() * (0.001 / RAND_MAX);
		}

		std::vector<float> orig_bit_counts = bit_counts;
		std::vector<float> smoothed_bit_counts;
		GaussianFilter(bit_counts, smoothed_bit_counts, 8.0, 3);

		std::vector<float> gradient(size);
		Gradient(smoothed_bit_counts.begin(), smoothed_bit_counts.end(), gradient.begin());

		for (size_t i = 0; i < size; i++) {
			gradient[i] = std::abs(gradient[i]);
		}

		std::vector<size_t> gradient_peaks;
		for (size_t i = 0; i < size; i++) {
			const auto gi = gradient[i];
			if (i > 0 && i < size - 1 && gi > 0.09 && gi >= gradient[i - 1] && gi >= gradient[i + 1]) {
				if (gradient_peaks.empty() || gradient_peaks.back() + 1 < i) {
					gradient_peaks.push_back(i);
				}
			}
//			DEBUG("x " << i << " " << orig_bit_counts[i] << " " << smoothed_bit_counts[i] << " " << gradient[i]);
		}
		gradient_peaks.push_back(size);

		struct Segment {
			size_t begin;
			size_t end;
			double score;
			Segment(size_t begin, size_t end, double score) : begin(begin), end(end), score(score) {}
			size_t size() const { return end - begin; }
		};

		std::vector<Segment> segments;
		size_t last_i = 0;
		for (size_t i : gradient_peaks) {
			segments.emplace_back(last_i, i, 0.0);
			last_i = i;
		}

		for (auto &s : segments) {
			for (size_t i = s.begin; i < s.end; i++) {
				s.score += orig_bit_counts[i];
			}
			s.score /= s.size();
			if (s.score < 10.0) {
				const auto t1 = GetHashTime(offset1 + s.begin);
				const auto t2 = GetHashTime(offset2 + s.begin);
				const auto duration = GetHashDuration(s.size());
				DEBUG("segment " << t1 << "-" << t1 + duration << " " << t2 << "-" << t2 + duration << " " << s.score);
				//DEBUG("segment " << offset1 + s.begin << "-" << offset1 + s.end << " " << offset2 + s.begin << "-" << offset2 + s.end << " " << s.score);
			}
		}

		break;
		//MatchAligned(fp1_begin, fp1.end(), fp2_begin, fp2.end());
	}
	
	return true;
}



};
