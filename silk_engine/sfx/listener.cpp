#include "listener.h"
#include <AL/al.h>

void Listener::setPosition(const vec3& position)
{
	if (this->position == position)
		return;
	this->position = position; 
	alListener3f(AL_POSITION, position.x, position.y, position.z);
}

void Listener::setVelocity(const vec3& velocity)
{
	if (this->velocity == velocity)
		return;
	this->velocity = velocity;
	alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

void Listener::setRotation(const vec3& rotation)
{
	if (this->rotation == rotation)
		return;
	this->rotation = rotation;
	vec3 ori[2] = { rotation, { 0, 1, 0 } };
	alListenerfv(AL_ORIENTATION, (ALfloat*)ori);
}

void Listener::setVolume(float volume)
{
	if (this->volume == volume)
		return;
	this->volume = volume; 
	alListenerf(AL_GAIN, volume);
}

void Listener::setInnerConeAngle(float angle)
{
	alListenerf(AL_CONE_INNER_ANGLE, angle);
}

void Listener::setOuterConeAngle(float angle)
{
	alListenerf(AL_CONE_OUTER_ANGLE, angle);
}

void Listener::setOuterConeGain(float gain)
{
	alListenerf(AL_CONE_OUTER_GAIN, gain);
}