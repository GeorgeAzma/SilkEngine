#include "audio_format.h"


size_t AudioFormatEnum::getSize(AudioFormat format)
{
	switch (format)
	{
	case AudioFormat::MONO8: return 1;
	case AudioFormat::MONO16: return 2;
	case AudioFormat::STEREO8: return 2;
	case AudioFormat::STEREO16: return 4;
	}

	SK_ERROR("Invalid audio format");
	return 0;
}

uint32_t AudioFormatEnum::getChannelCount(AudioFormat format)
{
	switch (format)
	{
	case AudioFormat::STEREO8: 
	case AudioFormat::STEREO16: 
		return 2;
	}
	return 1;
}

uint32_t AudioFormatEnum::getBitsPerSample(AudioFormat format)
{
	switch (format)
	{
	case AudioFormat::MONO16:
	case AudioFormat::STEREO16:
		return 16;
	}
	return 8;
}
