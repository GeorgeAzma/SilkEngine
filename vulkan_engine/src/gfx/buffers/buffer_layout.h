#pragma once

#include "gfx/enums.h"

struct BufferElement
{
public:
	BufferElement(Type type) 
		: type{type} {}

public:
	Type type;

private:
	size_t offset = 0;
	friend class BufferLayout;
};

class BufferLayout 
{
public:
	BufferLayout(const std::initializer_list<BufferElement>& elements = {});

	const std::vector<VkVertexInputBindingDescription>& getBindingDescriptions() const { return binding_descriptions; };
	const std::vector<VkVertexInputAttributeDescription>& getAttributeDescriptions() const { return attribute_descriptions; };
private:
	std::vector<VkVertexInputBindingDescription> binding_descriptions;
	std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
};