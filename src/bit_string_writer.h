// Copyright (C) 2010-2016  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#ifndef CHROMAPRINT_BIT_STRING_WRITER_H_
#define CHROMAPRINT_BIT_STRING_WRITER_H_

#include <cstdint>
#include <string>

namespace chromaprint {

class BitStringWriter {
public:
	BitStringWriter() : m_buffer(0), m_buffer_size(0) { }

	void Write(uint32_t x, int bits) {
		m_buffer |= (x << m_buffer_size);
		m_buffer_size += bits;
		while (m_buffer_size >= 8) {
			m_value.push_back(m_buffer & 255);
			m_buffer >>= 8;
			m_buffer_size -= 8;
		}
	}

	void Flush() {
		while (m_buffer_size > 0) {
			m_value.push_back(m_buffer & 255);
			m_buffer >>= 8;
			m_buffer_size -= 8;
		}
		m_buffer_size = 0;
	}

	std::string value() const {
		return m_value;
	}

private:
	std::string m_value;
	uint32_t m_buffer;
	int m_buffer_size;
};

}; // namespace chromaprint

#endif
