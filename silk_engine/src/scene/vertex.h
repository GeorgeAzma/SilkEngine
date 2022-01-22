#pragma once

struct Vertex
{
	glm::vec3 position;
	glm::vec2 texture_coordinates;
	glm::vec3 normal;
};

struct InstanceData
{
	glm::mat4 transform = glm::mat4(1);
	uint32_t texture = 0;

	bool operator==(const InstanceData& other) const
	{
		return transform == other.transform 
			&& texture == other.texture;
	}
};