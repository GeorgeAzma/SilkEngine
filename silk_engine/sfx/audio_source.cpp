#include "audio_source.h"
#include "audio.h"

AudioSource::AudioSource()
{
	alGenSources(1, &id);
}

AudioSource::~AudioSource()
{
	stop();
	alDeleteSources(1, &id);
}

float AudioSource::getTimeOffset() const
{
	ALfloat seconds = 0.0f;
	alGetSourcef(id, AL_SEC_OFFSET, &seconds);
	return seconds;
}

uint32_t AudioSource::getSampleOffset() const
{
	ALint sample = 0;
	alGetSourcei(id, AL_SAMPLE_OFFSET, &sample);
	return sample;
}

uint32_t AudioSource::getByteOffset() const
{
	ALint byte = 0;
	alGetSourcei(id, AL_BYTE_OFFSET, &byte);
	return byte;
}

uint32_t AudioSource::getProcessedBufferCount() const
{
	ALint processed = 0;
	alGetSourcei(id, AL_BUFFERS_PROCESSED, &processed);
	return processed;
}

uint32_t AudioSource::getQueuedBufferCount() const
{
	ALint queued = 0;
	alGetSourcei(id, AL_BUFFERS_QUEUED, &queued);
	return queued;
}

bool AudioSource::isPlaying() const
{
	ALint playing = 0; 
	alGetSourcei(id, AL_SOURCE_STATE, &playing); 
	return playing == AL_PLAYING;
}

void AudioSource::setPosition(const vec3& position)
{
	if (this->position == position)
		return;
	this->position = position;
	alSource3f(id, AL_POSITION, position.x, position.y, position.z);
}

void AudioSource::setVelocity(const vec3& velocity)
{
	if (this->velocity == velocity)
		return;
	this->velocity = velocity;
	alSource3f(id, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

void AudioSource::setDirection(const vec3& direction)
{
	if (this->direction == direction)
		return;
	this->direction = direction;
	alSource3f(id, AL_DIRECTION, direction.x, direction.y, direction.z);
}

void AudioSource::setVolume(float volume)
{
	if (this->volume == volume)
		return;
	this->volume = volume;
	alSourcef(id, AL_GAIN, volume);
}

void AudioSource::setPitch(float pitch)
{
	if (this->pitch == pitch)
		return;
	this->pitch = pitch;
	alSourcef(id, AL_PITCH, pitch);
}

void AudioSource::setLooping(bool looping)
{
	if (this->looping == looping)
		return;
	this->looping = looping;
	alSourcei(id, AL_LOOPING, looping);
}

void AudioSource::setMinDistance(float min_distance)
{
	if (this->min_distance == min_distance)
		return;
	this->min_distance = min_distance;
	alSourcef(id, AL_REFERENCE_DISTANCE, min_distance);
}

void AudioSource::setMaxDistance(float max_distance)
{
	if (this->max_distance == max_distance)
		return;
	this->max_distance = max_distance;
	alSourcef(id, AL_MAX_DISTANCE, max_distance);
}

void AudioSource::setMaxGain(float max_gain)
{
	if (this->max_gain == max_gain)
		return;
	this->max_gain = max_gain;
	alSourcef(id, AL_MAX_GAIN, max_gain);
}

void AudioSource::setMinGain(float min_gain)
{
	if (this->min_gain == min_gain)
		return;
	this->min_gain = min_gain;
	alSourcef(id, AL_MIN_GAIN, min_gain);
}

void AudioSource::setAttenuationStrength(float attenuation_strength)
{
	if (this->attenuation_strength == attenuation_strength)
		return;
	this->attenuation_strength = attenuation_strength;
	alSourcef(id, AL_ROLLOFF_FACTOR, attenuation_strength);
}

void AudioSource::play(const Audio& audio)
{
	setAudio(audio);
	play();
}

void AudioSource::play()
{
	alSourcePlay(id);
	paused = false;
}

void AudioSource::pause()
{
	if (paused)
		return;

	alSourcePause(id);
	paused = true;
}

void AudioSource::stop() const
{
	alSourceStop(id);
}

void AudioSource::rewind() const
{
	alSourceRewind(id);
}

void AudioSource::queue(const Audio& audio) const
{
	uint32_t audio_id = audio.getID();
	alSourceQueueBuffers(id, 1, (const ALuint*)&audio_id);
}

uint32_t AudioSource::unqueue() const
{
	uint32_t audio_id = 0;
	alSourceUnqueueBuffers(id, 1, (ALuint*)&audio_id);
	return audio_id;
}

void AudioSource::setTimeOffset(float seconds) const
{
	alSourcef(id, AL_SEC_OFFSET, seconds);
}

void AudioSource::setSampleOffset(uint32_t sample) const
{
	alSourcei(id, AL_SAMPLE_OFFSET, sample);
}

void AudioSource::setByteOffset(uint32_t byte) const
{
	alSourcei(id, AL_BYTE_OFFSET, byte);
}

void AudioSource::setRelative(bool relative)
{
	if (this->relative == relative)
		return;
	this->relative = relative;
	alSourcei(id, AL_SOURCE_RELATIVE, relative);
}

void AudioSource::setAudio(const Audio& audio)
{
	if (audio_id == audio.getID())
		return;
	alSourcei(id, AL_BUFFER, audio.getID());
	audio_id = audio.getID();
}
