#pragma once

#include "gfx/gpu_type.h"

struct BufferElement
{
	GpuType type;
	bool instanced = false;
};

class BufferLayout 
{
public:
	BufferLayout(const std::vector<BufferElement>& elements = {});

	const std::vector<VkVertexInputBindingDescription>& getBindingDescriptions() const { return binding_descriptions; }
	const std::vector<VkVertexInputAttributeDescription>& getAttributeDescriptions() const { return attribute_descriptions; }

private:
	std::vector<VkVertexInputBindingDescription> binding_descriptions;
	std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
	std::vector<VkVertexInputBindingDivisorDescriptionEXT> instance_descriptions;
};