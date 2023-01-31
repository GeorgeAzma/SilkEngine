#pragma once

#include "audio_format.h"

class ALCdevice;

class Microphone : NonCopyable
{
public:
	Microphone(const char* microphone_name = nullptr, uint32_t sample_rate = 44100, AudioFormat format = AudioFormat::MONO16, size_t buffer_size = 1024);
	~Microphone();

	void start();
	void getSamples(void* buffer, uint32_t samples) const;
	uint32_t getAvailableSampleCount() const;
	uint32_t getAvailableSamples(void* buffer) const;
	void stop();
	bool exists() const { return device; }
	uint32_t getSampleRate() const { return sample_rate; }
	AudioFormat getFormat() const { return format; }
	size_t getBufferSize() const { return buffer_size; }

private:
	bool recording = false;
	ALCdevice* device = nullptr;
	uint32_t sample_rate = 44100;
	AudioFormat format = AudioFormat::MONO16;
	size_t buffer_size = 1024;
};