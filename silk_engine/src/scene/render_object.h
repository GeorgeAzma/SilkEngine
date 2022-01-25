#pragma once

#include "meshes/mesh.h"
#include "gfx/pipeline/graphics_pipeline.h"
#include "gfx/pipeline/compute_pipeline.h"
#include "gfx/descriptors/descriptor_set.h"
#include "gfx/descriptors/descriptor_set_layout.h"
#include "vertex.h"

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

struct CullData
{
	glm::vec3 min;
	uint32_t index;
	glm::vec3 max;
	uint32_t count;
	std::array<glm::vec4, 6> planes;
};

struct RenderObject
{
	shared<Mesh> mesh;
	shared<MaterialData> material_data;

	InstanceData instance_data;

	bool operator==(const RenderObject& other) const
	{
		return (*mesh == *other.mesh) 
			&& (*other.material_data->material == *material_data->material);
	}
};

struct IndirectBatch
{
	RenderObject render_object;

	std::vector<InstanceData> instance_datas;

	bool needs_update = true;

	bool operator==(const RenderObject& render_object) const
	{
		return this->render_object == render_object;
	}
};