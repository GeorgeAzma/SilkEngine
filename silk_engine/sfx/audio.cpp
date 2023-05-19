#include "audio.h"
#include "raw_audio.h"

Audio::Audio(const RawAudio& audio)
	: format(audio.format), sample_rate(audio.sample_rate)
{
	alGenBuffers(1, &id);
	setData(audio.data.data(), audio.data.size());
}

Audio::Audio(const fs::path& file)
	: Audio(RawAudio(file))
{
}

Audio::~Audio()
{
	alDeleteBuffers(1, &id);
}

uint32_t Audio::getChannelCount() const
{
	return AudioFormatEnum::getChannelCount(format);
}

uint32_t Audio::getBitsPerSample() const
{
	return AudioFormatEnum::getBitsPerSample(format);
}

uint32_t Audio::getSamples() const
{
	return getSize() / AudioFormatEnum::getSize(format);
}

float Audio::getDuration() const
{
	return float(getSamples()) / getSampleRate();
}

void Audio::setData(const void* data, size_t size)
{
	this->size = size;
	alBufferData(id, ALenum(format), data, size, sample_rate);
}