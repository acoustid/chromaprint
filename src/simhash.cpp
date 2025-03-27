// Copyright (C) 2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#include "simhash.h"

namespace chromaprint {

uint32_t SimHash(const uint32_t *data, size_t size)
{
	int v[32];

	for (size_t i = 0; i < 32; i++) {
		v[i] = 0;
	}

	for (size_t i = 0; i < size; i++) {
		uint32_t local_hash = data[i];
		for (size_t j = 0; j < 32; j++) {
			v[j] += (local_hash >> j) & 1;
		}
	}

	const int threshold = size / 2;
	uint32_t hash = 0;
	for (size_t i = 0; i < 32; i++) {
		const int b = v[i] > threshold ? 1 : 0;
		hash |= (b << i);
	}

	return hash;
}

uint32_t SimHash(const std::vector<uint32_t> &data)
{
	if (data.empty()) {
		return 0;
	} else {
		return SimHash(&data[0], data.size());
	}
}

}; // namespace chromaprint
