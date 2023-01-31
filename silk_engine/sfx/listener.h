#pragma once

class Listener
{
public:
	vec3 getPosition() const { return position; }
	vec3 getVelocity() const { return velocity; }
	vec3 getRotation() const { return rotation; }
	float getVolume() const { return volume; }

	void setPosition(const vec3& position);
	void setVelocity(const vec3& velocity);
	void setRotation(const vec3& rotation);
	void setVolume(float volume);
	void setInnerConeAngle(float angle);
	void setOuterConeAngle(float angle);
	void setOuterConeGain(float gain);

private:
	vec3 position = vec3(0);
	vec3 velocity = vec3(0);
	vec3 rotation = vec3(0);
	float volume = 1.0f;
	float inner_cone_angle = 360.0f;
	float outer_cone_angle = 360.0f;
};