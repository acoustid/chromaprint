#include "audio/ffmpeg_audio_reader.h"
#include <gtest/gtest.h>

#include <config.h>

namespace chromaprint {

TEST(FFmpegAudioReaderTest, ReadRaw) {
	FFmpegAudioReader reader;

	ASSERT_TRUE(reader.SetInputFormat("s16le"));
	ASSERT_TRUE(reader.SetInputChannels(2));
	ASSERT_TRUE(reader.SetInputSampleRate(44100));

	ASSERT_TRUE(reader.Open(TESTS_DIR "/data/test_stereo_44100.raw")) << reader.GetError();
	ASSERT_TRUE(reader.IsOpen());

	ASSERT_EQ(2, reader.GetChannels());
	ASSERT_EQ(44100, reader.GetSampleRate());

	const int16_t *data;
	size_t size;
	while (reader.Read(&data, &size)) {

	}

/*	ASSERT_TRUE(reader.ReadAll([&](const int16_t *data, size_t size) {

	}));*/
}

TEST(FFmpegAudioReaderTest, ReadChannelLayoutChange) {
	FFmpegAudioReader reader;

	reader.SetOutputChannels(2);
	reader.SetOutputSampleRate(11025);

	// 5.1 AC-3 stream with a few stereo frames in the middle
	ASSERT_TRUE(reader.Open(TESTS_DIR "/data/test_51_stereo_switch.ac3")) << reader.GetError();
	ASSERT_TRUE(reader.IsOpen());

	ASSERT_EQ(2, reader.GetChannels());
	ASSERT_EQ(11025, reader.GetSampleRate());

	const int16_t *data;
	size_t size;
	size_t total_size = 0;
	while (reader.Read(&data, &size)) {
		total_size += size;
	}
	ASSERT_TRUE(reader.IsFinished()) << reader.GetError();

	// 15 frames of 1536 samples at 44100 Hz
	ASSERT_NEAR(15 * 1536 * 11025 / 44100, total_size, 100);
}

}; // namespace chromaprint
