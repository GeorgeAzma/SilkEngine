#pragma once

#include "meshes/mesh.h"
#include "gfx/graphics_pipeline.h"
#include "gfx/descriptor_set.h"
#include "vertex.h"

struct Material
{
	shared<DescriptorSetLayout> descriptor_set_layout;
	shared<GraphicsPipeline> graphics_pipeline;

	std::string name;

	bool operator==(const Material& other) const
	{
		return name == other.name;
	}
};

struct MaterialData
{
	shared<Material> material;
	shared<DescriptorSet> descriptor_set;
};

struct RenderObject
{
	shared<Mesh> mesh;
	shared<MaterialData> material_data;

	InstanceData instance_data;

	bool operator==(const RenderObject& other) const
	{
		return *mesh == *other.mesh && *other.material_data->material == *material_data->material;
	}
};

struct IndirectBatch
{
	RenderObject render_object;
	size_t first = 0;
	size_t count = 0;

	bool operator==(const RenderObject& render_object) const
	{
		return this->render_object == render_object;
	}
};