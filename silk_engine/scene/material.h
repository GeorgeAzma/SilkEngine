#pragma once

#include "gfx/pipeline/graphics_pipeline.h"
#include "gfx/descriptors/descriptor_set.h"


// TODO:
struct Material : NonCopyable
{
	shared<GraphicsPipeline> pipeline;
	std::unordered_map<uint32_t, DescriptorSet> descriptor_sets;

	void bind();
};