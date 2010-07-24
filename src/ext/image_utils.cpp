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
#include <png++/png.hpp>
#include "image_utils.h"

using namespace std;
using namespace Chromaprint;

static const int kNumColors = 6;
static int colors[][3] = {
	{ 0, 0, 0 },
	{ 218, 38, 0 },
	{ 221, 99, 0 },
	{ 255, 253, 0 },
	{ 255, 254, 83 },
	{ 255, 255, 200 },
	{ 255, 255, 255 },
};

void Chromaprint::ExportImage(Image *data, const string &file_name)
{
	png::image<png::rgb_pixel> image(data->NumRows(), data->NumColumns());
	double min_value = (*data)[0][0], max_value = (*data)[0][0];
	for (size_t y = 0; y < data->NumRows(); y++) {
		for (size_t x = 0; x < data->NumColumns(); x++) {
			double value = (*data)[y][x];
			min_value = min(min_value, value);
			max_value = max(max_value, value);
		}
	}
	//std::cout << "min_value=" << min_value << "\n";
	//std::cout << "max_value=" << max_value << "\n";
	for (size_t y = 0; y < data->NumRows(); y++) {
		for (size_t x = 0; x < data->NumColumns(); x++) {
			double value = ((*data)[y][x] - min_value) / (max_value - min_value);
			double color_value = kNumColors * value;
			int color_index = int(color_value);
			double color_alpha = color_value - color_index;
			if (color_index < 0) {
				color_index = 0;
				color_alpha = 0;
			}
			else if (color_index > kNumColors) {
				color_index = kNumColors;
				color_alpha = 0;
			}
			//std::cout << "value=" << color_value << "\n";
			//std::cout << "alpha=" << color_alpha << "\n";
			int r = colors[color_index][0] + (colors[color_index+1][0] - colors[color_index][0]) * color_alpha;
			int g = colors[color_index][1] + (colors[color_index+1][1] - colors[color_index][1]) * color_alpha;
			int b = colors[color_index][2] + (colors[color_index+1][2] - colors[color_index][2]) * color_alpha;
			//int color = 255 * vlue + 0.5;
			image[data->NumColumns()-x-1][y] = png::rgb_pixel(r, g, b);
		}
	}
	image.write(file_name);
}

