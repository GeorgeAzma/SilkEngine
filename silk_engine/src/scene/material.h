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

//TODO: Support multiple passes, or do that in shader_effect and remove this struct
struct Material
{
	shared<ShaderEffect> shader_effect;

	void bind(size_t first_set = 0)
	{
		shader_effect->pipeline->bind();
	}

	bool operator==(const Material& other) const
	{
		return shader_effect == other.shader_effect;
	}
};

//TODO:
struct ComputeMaterial
{
	shared<ComputeShaderEffect> compute_shader_effect;
	std::vector<DescriptorSet> descriptor_sets;
	
	void update()
	{
		for (size_t i = 0; i < descriptor_sets.size(); ++i)
			descriptor_sets[i].update();
	}

	void bind(size_t first_set = 0)
	{
		compute_shader_effect->pipeline->bind();
		for (size_t i = 0; i < descriptor_sets.size(); ++i)
		{
			descriptor_sets[i].bind(i + first_set);
		}
	}
};