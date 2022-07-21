#pragma once

#include <AL/al.h>

enum class AudioFormat
{
	MONO8 = AL_FORMAT_MONO8,
	MONO16 = AL_FORMAT_MONO16,
	STEREO8 = AL_FORMAT_STEREO8,
	STEREO16 = AL_FORMAT_STEREO16
};