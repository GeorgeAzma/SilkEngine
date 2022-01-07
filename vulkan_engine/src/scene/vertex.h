#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

struct Vertex
{
	glm::vec2 position;
	glm::vec3 color;

	static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};
