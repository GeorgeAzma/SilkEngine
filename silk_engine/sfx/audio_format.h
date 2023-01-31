#pragma once

#include <AL/al.h>

enum class AudioFormat
{
	MONO8 = AL_FORMAT_MONO8,
	MONO16 = AL_FORMAT_MONO16,
	STEREO8 = AL_FORMAT_STEREO8,
	STEREO16 = AL_FORMAT_STEREO16
};

class AudioFormatEnum
{
public:
	static size_t getSize(AudioFormat format);
	static uint32_t getChannelCount(AudioFormat format);
	static uint32_t getBitsPerSample(AudioFormat format);
};