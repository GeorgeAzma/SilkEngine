#pragma once

#include "listener.h"
#include <AL/al.h>

enum class DistanceModel
{
	NONE = AL_NONE, // gain = default_gain
	INVERSE = AL_INVERSE_DISTANCE, // gain = 1 / distance
	INVERSE_CLAMPED = AL_INVERSE_DISTANCE_CLAMPED, // gain = clamp(1 / distance, x, y)
	LINEAR = AL_LINEAR_DISTANCE,  // gain = -distance
	LINEAR_CLAMPED = AL_LINEAR_DISTANCE_CLAMPED, // gain = clamp(-distance, x, y)
	EXPONENT = AL_EXPONENT_DISTANCE, // gain = -(distance * distance)
	EXPONENT_CLAMPED = AL_EXPONENT_DISTANCE_CLAMPED // gain = clamp(-(distance * distance), x ,y)
};

class AudioManager
{
public:
	static void setDistanceModel(DistanceModel distance_model);
	static void setSpeedOfSound(float speed);
	static void setDopplerFactor(float value);
	static Listener& getListener() { return listener; }

private:
	static inline Listener listener;
};