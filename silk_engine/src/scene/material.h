#pragma once

#include "gfx/pipeline/graphics_pipeline.h"
#include "gfx/pipeline/compute_pipeline.h"
#include "gfx/descriptors/descriptor_set.h"

struct ShaderEffect
{
	shared<GraphicsPipeline> pipeline;

	bool operator==(const ShaderEffect& other) const
	{
		return pipeline.get() == other.pipeline.get();
	}
};

struct ComputeShaderEffect
{
	shared<ComputePipeline> pipeline;

	bool operator==(const ComputeShaderEffect& other) const
	{
		return pipeline.get() == other.pipeline.get();
	}
};

//TODO:
struct Material
{
	shared<ShaderEffect> shader_effect;
	std::vector<shared<DescriptorSet>> descriptor_sets;

	void bind(size_t first_set = 0)
	{
		for (size_t i = 0; i < descriptor_sets.size(); ++i)
		{
			descriptor_sets[i]->bind(i + first_set);
		}
	}
};

struct ComputeMaterialData
{
	shared<ComputeShaderEffect> material;
	std::vector<shared<DescriptorSet>> descriptor_sets;

	void bind(size_t first_set = 0)
	{
		for (size_t i = 0; i < descriptor_sets.size(); ++i)
		{
			descriptor_sets[i]->bind(i + first_set);
		}
	}
};