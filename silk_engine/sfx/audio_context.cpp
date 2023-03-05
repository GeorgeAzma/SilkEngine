#include "audio_context.h"
#include <AL/alc.h>

AudioContext::AudioContext(ALCdevice* device)
{
	context = alcCreateContext(device, nullptr);
	SK_VERIFY(context, "OpenAL: Couldn't create audio context");

	if (!alcMakeContextCurrent(context))
		SK_ERROR("OpenAL: Couldn't make audio context current");
}

AudioContext::~AudioContext()
{
	if(alcGetCurrentContext() == context)
		if (!alcMakeContextCurrent(nullptr))
			SK_ERROR("OpenAL: Couldn't clear active context");

	alcDestroyContext(context);
}
