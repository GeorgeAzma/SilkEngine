#pragma once

#include "meshes/mesh.h"
#include "gfx/graphics_pipeline.h"
#include "gfx/descriptor_set.h"
#include "vertex.h"

struct Material
{
	std::shared_ptr<DescriptorSetLayout> descriptor_set_layout;
	std::shared_ptr<GraphicsPipeline> graphics_pipeline;
};

struct MaterialData
{
	std::shared_ptr<Material> material;
	std::shared_ptr<DescriptorSet> descriptor_set;
};

struct RenderObject
{
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<MaterialData> material_data;

	bool operator==(const RenderObject& other) const
	{
		return *mesh == *other.mesh; //TODO: compare materials too
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