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

#ifndef CHROMAPRINT_EXT_IMAGE_UTILS_H_
#define CHROMAPRINT_EXT_IMAGE_UTILS_H_

#include <string>
#include <iostream>
#include <fstream>
#include "image.h"

namespace Chromaprint
{

	//! Export image to a PNG file
	void ExportImage(Image *image, const std::string &file_name);

	//! Export image in a text format (floating point numbers) to any stream
	template <class ImageType>
	void ExportTextImage(ImageType *image, std::ostream &stream)
	{
		for (int i = 0; i < image->NumRows(); i++) {
			for (int j = 0; j < image->NumColumns(); j++) {
				stream << image->Row(i)[j] << " ";
			}
			stream << "\n";
		}
	}

	//! Export image in a text format (floating point numbers) to a file
	template <class ImageType>
	void ExportTextImage(ImageType *image, const std::string &file_name)
	{
		std::fstream out(file_name.c_str(), std::ios::out);
		ExportTextImage(image, out);
	}

};

#endif
