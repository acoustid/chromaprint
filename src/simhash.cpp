/*
 * Chromaprint -- Audio fingerprinting toolkit
 * Copyright (C) 2015  Lukas Lalinsky <lalinsky@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "simhash.h"

namespace Chromaprint {

    int32_t SimHash(const int32_t *data, size_t size)
    {
        int v[32];

        for (size_t i = 0; i < 32; i++) {
            v[i] = 0;
        }

        for (size_t i = 0; i < size; i++) {
            uint32_t local_hash = SignedToUnsigned(data[i]);
            for (size_t j = 0; j < 32; j++) {
                v[j] += (local_hash & (1 << j)) ? 1 : -1;
            }
        }

        uint32_t hash = 0;
        for (size_t i = 0; i < 32; i++) {
            if (v[i] > 0) {
                hash |= (1 << i);
            }
        }

        return UnsignedToSigned(hash);
    }

    int32_t SimHash(const std::vector<int32_t> &data)
    {
		if (data.empty()) {
			return 0;
		} else {
			return SimHash(&data[0], data.size());
		}
    }

}
