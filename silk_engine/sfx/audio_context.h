#pragma once

class ALCdevice;
class ALCcontext;

class AudioContext
{
public:
	AudioContext(ALCdevice* device);
	~AudioContext();

private:
	ALCcontext* context = nullptr;
};