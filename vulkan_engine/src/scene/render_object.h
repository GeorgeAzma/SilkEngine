#pragma once

#include "meshes/mesh.h"
#include "gfx/graphics_pipeline.h"
#include "gfx/descriptor_set.h"
#include "vertex.h"

struct Material
{
	std::shared_ptr<GraphicsPipeline> graphics_pipeline = nullptr;
	std::shared_ptr<DescriptorSet> descriptor_set = nullptr;
};

struct RenderObject
{
	std::shared_ptr<Mesh> mesh = nullptr;
	std::shared_ptr<Material> material = {};

	InstanceData instance_data = {};

	bool operator==(const RenderObject& other) const
	{
		return *mesh == *other.mesh;
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