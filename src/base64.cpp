/*
 * Chromaprint -- Audio fingerprinting toolkit
 * Copyright (C) 2010  Lukas Lalinsky <lalinsky@gmail.com>
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

#include <algorithm>
#include "base64.h"

using namespace std;
using namespace Chromaprint;

static const char kBase64Chars[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

string Chromaprint::Base64Encode(const string &orig)
{
	int size = orig.size();
	int encoded_size = (size * 4 + 2) / 3;
	string encoded(encoded_size, ' ');
	const unsigned char *src = (unsigned char *)orig.data();
	string::iterator dest = encoded.begin();
	int i = 0, j = 0;
	while (size > 0) {
		*dest++ = kBase64Chars[(src[0] >> 2)];
		*dest++ = kBase64Chars[((src[0] << 4) | (--size ? (src[1] >> 4) : 0)) & 63];
		if (size) {
			*dest++ = kBase64Chars[((src[1] << 2) | (--size ? (src[2] >> 6) : 0)) & 63];
			if (size) {
				*dest++ = kBase64Chars[src[2] & 63];
				--size;
			}
		}
		src += 3;
	}
	return encoded;
}

