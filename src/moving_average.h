/*
 * Chromaprint -- Audio fingerprinting toolkit
 * Copyright (C) 2012  Lukas Lalinsky <lalinsky@gmail.com>
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

#ifndef CHROMAPRINT_MOVING_AVERAGE_H_
#define CHROMAPRINT_MOVING_AVERAGE_H_

#include <math.h>
#include <assert.h>
#include <algorithm>
#include "debug.h"

namespace Chromaprint
{

	template<class T>
	class MovingAverage
	{
	public:
		MovingAverage(int size)
			: m_size(size), m_offset(0), m_sum(0), m_count(0)
		{
			m_buffer = new T[m_size];
			std::fill(m_buffer, m_buffer + m_size, 0);
		}

		~MovingAverage()
		{
			delete[] m_buffer;
		}

		void AddValue(const T &x)
		{
			DEBUG() << "offset is " << m_offset << "\n";
			m_sum += x;
			DEBUG() << "adding " << x << " sum is " << m_sum << "\n";
			m_sum -= m_buffer[m_offset];
			DEBUG() << "subtracting " << m_buffer[m_offset] << " sum is " << m_sum << "\n";
			if (m_count < m_size) {
				m_count++;
			}
			m_buffer[m_offset] = x;
			m_offset = (m_offset + 1) % m_size;
		}

		T GetAverage() const
		{
			if (!m_count) {
				return 0;
			}
			return m_sum / m_count;
		}

	private:
		T *m_buffer;
		int m_size;
		int m_offset;
		int m_sum;
		int m_count;
	};

};

#endif
