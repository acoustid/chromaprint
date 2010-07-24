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

#ifndef CHROMAPRINT_FINGERPRINTER_CONFIGURATION_H_
#define CHROMAPRINT_FINGERPRINTER_CONFIGURATION_H_

#include <stdint.h>
#include <vector>
#include "audio_consumer.h"

namespace Chromaprint
{
	class Image;
	class ImageBuilder;
	class IntegralImage;
	class FFT;
	class Chroma;
	class AudioProcessor;

	class FingerprinterConfiguration
	{
	public:	
		FingerprinterConfiguration()
			: m_classifiers(0), m_num_classifiers(0)
		{}

		virtual ~FingerprinterConfiguration() {}

		virtual CreatePipeline(ImageBuilder *image_builder);

		/**
		 * Initialize the fingerprinting process.
		 */
		void Init(int length, int sample_rate, bool stereo);

		/**
		 * Process a block of raw audio data. Call this method as many times
		 * as you need. 
		 */
		void Consume(short *input, int length);

		int num_classifiers() const
		{
			return m_num_classifiers;
		}

		const Classifier &classifier(int i) const
		{
			return m_classifiers[i];
		}

	protected:
		FingerprinterConfiguration(const Classifier *classifiers, int num_classifiers)
			: m_classifiers(classifiers), m_num_classifiers(num_classifiers)
		{}

	private:
		int m_num_classifiers;
		const Classifier *m_classifiers;
		Chroma *m_chroma;
		FFT *m_fft;
		AudioProcessor *m_audio_processor;
	};

};

#endif

