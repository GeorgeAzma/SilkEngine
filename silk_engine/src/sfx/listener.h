#pragma once

class Listener
{
public:
	glm::vec3 getPosition() const { return position; }
	glm::vec3 getVelocity() const { return velocity; }
	glm::vec3 getRotation() const { return rotation; }
	float getVolume() const { return volume; }

	void setPosition(const glm::vec3& position);
	void setVelocity(const glm::vec3& velocity);
	void setRotation(const glm::vec3& rotation);
	void setVolume(float volume);
	void setInnerConeAngle(float angle);
	void setOuterConeAngle(float angle);
	void setOuterConeGain(float gain);

private:
	glm::vec3 position = glm::vec3(0);
	glm::vec3 velocity = glm::vec3(0);
	glm::vec3 rotation = glm::vec3(0);
	float volume = 1.0f;
	float inner_cone_angle = 360.0f;
	float outer_cone_angle = 360.0f;
};