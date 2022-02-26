#pragma once

#include "gfx/enums.h"

struct BufferElement
{
	Type type;
	bool instanced = false;
};

class BufferLayout 
{
public:
	BufferLayout(const std::initializer_list<BufferElement>& elements = {});

	const std::vector<vk::VertexInputBindingDescription>& getBindingDescriptions() const { return binding_descriptions; };
	const std::vector<vk::VertexInputAttributeDescription>& getAttributeDescriptions() const { return attribute_descriptions; };
private:
	std::vector<vk::VertexInputBindingDescription> binding_descriptions;
	std::vector<vk::VertexInputAttributeDescription> attribute_descriptions;
	std::vector<vk::VertexInputBindingDivisorDescriptionEXT> instance_descriptions;
};