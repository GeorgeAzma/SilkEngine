#pragma once

#include "gfx/enums.h"

class BufferLayout 
{
public:
	BufferLayout(const std::initializer_list<Type>& elements = {});

	const std::vector<VkVertexInputBindingDescription>& getBindingDescriptions() const { return binding_descriptions; };
	const std::vector<VkVertexInputAttributeDescription>& getAttributeDescriptions() const { return attribute_descriptions; };
private:
	std::vector<VkVertexInputBindingDescription> binding_descriptions;
	std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
};