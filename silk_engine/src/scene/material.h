#pragma once

#include "gfx/pipeline/graphics_pipeline.h"
#include "gfx/pipeline/compute_pipeline.h"
#include "gfx/descriptors/descriptor_set.h"

struct Material
{
	shared<GraphicsPipeline> pipeline;

	std::string name;

	bool operator==(const Material& other) const
	{
		return name == other.name;
	}
};

struct MaterialData
{
	shared<Material> material;
	std::vector<shared<DescriptorSet>> descriptor_sets;
};

struct ComputeMaterial
{
	shared<ComputePipeline> pipeline;

	std::string name;

	bool operator==(const ComputeMaterial& other) const
	{
		return name == other.name;
	}
};

struct ComputeMaterialData
{
	shared<ComputeMaterial> material;
	std::vector<shared<DescriptorSet>> descriptor_sets;
};