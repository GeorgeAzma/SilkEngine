#include "vertex.h"

#define INTERLEAVED 0

std::vector<VkVertexInputBindingDescription> Vertex::getBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> binding_descriptions(1, VkVertexInputBindingDescription{});
	binding_descriptions[0].binding = 0;
	binding_descriptions[0].stride = sizeof(Vertex);
	binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return binding_descriptions;
}

std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attribute_descriptions(2, VkVertexInputAttributeDescription{});
	attribute_descriptions[0].binding = 0;
	attribute_descriptions[0].location = 0;
	attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attribute_descriptions[0].offset = offsetof(Vertex, position);

	attribute_descriptions[1].binding = 0;
	attribute_descriptions[1].location = 1;
	attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptions[1].offset = offsetof(Vertex, color);

	return attribute_descriptions;
}