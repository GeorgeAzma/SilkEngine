#include "vertex.h"

VkVertexInputBindingDescription Vertex::getBindingDescription()
{
	VkVertexInputBindingDescription binding_description{};
	binding_description.binding = 0;
	binding_description.stride = sizeof(Vertex);
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	
	return binding_description;
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