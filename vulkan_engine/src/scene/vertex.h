#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

struct Vertex
{
	glm::vec2 position;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};
