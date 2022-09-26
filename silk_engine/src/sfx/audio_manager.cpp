#include "audio_manager.h"

void AudioManager::setDistanceModel(DistanceModel distance_model)
{
	alDistanceModel(ALenum(distance_model));
}

void AudioManager::setSpeedOfSound(float speed)
{
	alSpeedOfSound(speed);
}

void AudioManager::setDopplerFactor(float value)
{
	alDopplerFactor(value);
}