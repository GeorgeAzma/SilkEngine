#pragma once

#include "meshes/mesh.h"
#include "gfx/buffers/indirect_buffer.h"
#include "material.h"

struct CullData
{
	glm::vec3 min;
	uint32_t index;
	glm::vec3 max;
	uint32_t count;
	std::array<glm::vec4, 6> planes;
};

struct InstanceData
{
	glm::mat4 transform = glm::mat4(1);
	uint32_t texture_index = 0;
	glm::vec4 color = glm::vec4(1);

	bool operator==(const InstanceData& other) const
	{
		return transform == other.transform
			&& texture_index == other.texture_index;
	}
};

struct RenderedInstance
{
	shared<Mesh> mesh;
	shared<MaterialData> material_data = nullptr;

	InstanceData* instance_data = nullptr;
	bool batched = false;

	bool operator==(const RenderedInstance& other) const
	{
		return (*mesh == *other.mesh) && (*other.material_data->material == *material_data->material);
	}
};

struct InstanceBatch
{
	shared<RenderedInstance> instance;

	std::vector<InstanceData> instance_data;

	bool needs_update = true;

	bool operator==(const RenderedInstance& instance) const
	{
		return *this->instance == instance;
	}
};