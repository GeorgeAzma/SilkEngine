#pragma once

class ALCdevice;

class Microphone : NonCopyable
{
public:
	Microphone(const char* microphone_name = nullptr);
	~Microphone();

	void start();
	void getSamples(void* buffer, uint32_t samples) const;
	uint32_t getAvailableSampleCount() const;
	uint32_t getAvailableSamples(void* buffer) const;
	void stop();

private:
	bool recording = false;
	ALCdevice* device = nullptr;
};