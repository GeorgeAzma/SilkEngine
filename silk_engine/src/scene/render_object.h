#pragma once

#include "meshes/mesh.h"
#include "material.h"

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

struct BatchData
{
	glm::vec3 position;
	glm::vec2 texture_coordinates;
	glm::vec3 normal;
	glm::vec4 color;
	uint32_t texture_index;
};