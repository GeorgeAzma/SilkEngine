#include "audio.h"
#include "raw_audio.h"
#include <AL/al.h>

Audio::Audio(const RawAudio& audio)
{
	alGenBuffers(1, &id);
	alBufferData(id, ALenum(audio.format), audio.data.data(), audio.data.size(), audio.sample_rate);
}

Audio::Audio(std::string_view file)
	: Audio(RawAudio(file))
{
}

Audio::~Audio()
{
	alDeleteBuffers(1, &id);
}

uint32_t Audio::getChannelCount() const
{
	ALint channels = 0;
	alGetBufferi(id, AL_CHANNELS, &channels);
	return channels;
}

uint32_t Audio::getSize() const
{
	ALint size = 0;
	alGetBufferi(id, AL_SIZE, &size);
	return size;
}

uint32_t Audio::getBitsPerSample() const
{
	ALint bits = 0;
	alGetBufferi(id, AL_BITS, &bits);
	return bits;
}

uint32_t Audio::getSampleRate() const
{
	ALint sample_rate = 0;
	alGetBufferi(id, AL_FREQUENCY, &sample_rate);
	return sample_rate;
}

uint32_t Audio::getSamples() const
{
	return getSize() * 8 / (getChannelCount() * getBitsPerSample());
}

float Audio::getDuration() const
{
	float duration = float(getSamples()) / float(getSampleRate());
	return duration;
}