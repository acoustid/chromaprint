// Copyright (C) 2010-2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_BIT_STRING_READER_H_
#define CHROMAPRINT_BIT_STRING_READER_H_

#include <cstdint>
#include <string>

namespace chromaprint {

class BitStringReader {
public:
	BitStringReader(const std::string &input) : m_value(input), m_buffer(0), m_buffer_size(0), m_eof(false) {
		m_value_iter = m_value.begin();
	}

	bool eof() const {
		return m_eof;
	}

	uint32_t Read(int bits) {
		if (m_buffer_size < bits) {
			if (m_value_iter != m_value.end()) {
				m_buffer |= (unsigned char)(*m_value_iter++) << m_buffer_size;
				m_buffer_size += 8;
			}
			else {
				m_eof = true;
			}
		}

		uint32_t result = m_buffer & ((1 << bits) - 1);
		m_buffer >>= bits;
		m_buffer_size -= bits;

		if (m_buffer_size <= 0 && m_value_iter == m_value.end()) {
			m_eof = true;
		}

		return result;
	}

	void Reset() {
		m_buffer = 0;
		m_buffer_size = 0;
	}

	size_t AvailableBits() const {
		return eof() ? 0 : m_buffer_size + 8 * (m_value.end() - m_value_iter);
	}

private:
	std::string m_value;
	std::string::iterator m_value_iter;
	uint32_t m_buffer;
	int m_buffer_size;
	bool m_eof;
};

}; // namespace chromaprint

#endif
