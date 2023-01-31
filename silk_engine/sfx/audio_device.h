#pragma once

class AudioContext;
class ALCdevice;

class AudioDevice : NonCopyable
{
public:
	AudioDevice(const char* device_name = nullptr);
	~AudioDevice();

private:
	AudioContext* context = nullptr;
	ALCdevice* device = nullptr;
};