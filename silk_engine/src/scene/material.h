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