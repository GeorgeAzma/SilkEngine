#pragma once

class Audio;

class AudioSource
{
public:
	AudioSource();
	~AudioSource();

	vec3 getPosition() const { return position; }
	vec3 getVelocity() const { return velocity; }
	vec3 getDirection() const { return direction; }
	float getVolume() const { return volume; }
	float getPitch() const { return pitch; }
	bool isLooping() const { return looping; }
	bool isPaused() const { return paused; }
	float getMinDistance() const { return min_distance; }
	float getMaxDistance() const { return max_distance; }
	float getMaxGain() const { return max_gain; }
	float getMinGain() const { return min_gain; }
	float getAttenuationStrength() const { return attenuation_strength; }
	bool isRelative() const { return relative; }
	float getTimeOffset() const;
	uint32_t getSampleOffset() const;
	uint32_t getByteOffset() const;
	uint32_t getProcessedBufferCount() const;
	uint32_t getQueuedBufferCount() const;
	bool isPlaying() const;

	void setPosition(const vec3& position);
	void setVelocity(const vec3& velocity);
	void setDirection(const vec3& direction);
	void setVolume(float volume);
	void setPitch(float pitch);
	void setLooping(bool looping = true);
	void setMinDistance(float min_distance);
	void setMaxDistance(float max_distance);
	void setMaxGain(float max_gain);
	void setMinGain(float min_gain);
	void setAttenuationStrength(float attenuation_strength);
	void setTimeOffset(float seconds) const;
	void setSampleOffset(uint32_t sample) const;
	void setByteOffset(uint32_t byte) const;
	void setRelative(bool relative = true);

	void play(const Audio& audio);
	void play();
	void pause();
	void stop() const;
	void rewind() const;
	void queue(const Audio& audio) const;
	uint32_t unqueue() const;

private:
	void setAudio(const Audio& audio);

private:
	uint32_t id = 0;
	uint32_t audio_id = 0;
	vec3 position = vec3(0);
	vec3 velocity = vec3(0);
	vec3 direction = vec3(0);
	float volume = 1.0f;
	float pitch = 1.0f;
	bool looping = false;
	bool paused = false;
	float min_distance = 1.0f; // No attenuation happens below min distance
	float max_distance = std::numeric_limits<float>::max();
	float max_gain = 1.0f;
	float min_gain = 0.0f;
	float attenuation_strength = 1.0f;
	bool relative = false;
};