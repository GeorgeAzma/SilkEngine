#pragma once

#include "meshes/mesh.h"
#include "gfx/images/image2D.h"
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
	uint32_t image_index = 0;
	glm::vec4 color = glm::vec4(1);

	bool operator==(const InstanceData& other) const
	{
		return transform == other.transform
			&& image_index == other.image_index
			&& color == color;
	}
};

struct InstanceBatch;

struct RenderedInstance
{
	shared<Mesh> mesh;
	shared<ShaderEffect> material = nullptr;

	std::vector<shared<Image2D>> images;
	size_t instance_data_index = -1;
	size_t instance_batch_index = -1;

	bool operator==(const RenderedInstance& other) const
	{
		return (*mesh == *other.mesh);
	}
};

struct InstanceBatch
{
	shared<RenderedInstance> instance;

	std::vector<InstanceData> instance_data;
	std::vector<shared<RenderedInstance>> instances;
	shared<VertexBuffer> instance_buffer = nullptr;
	std::vector<shared<Image>> images;
	std::vector<uint32_t> image_owners;
	std::unordered_map<uint32_t, DescriptorSet> descriptor_sets;

	void bind();
	uint32_t addImages(const std::vector<shared<Image2D>>& new_images);
	void removeImages(size_t index, size_t count) 
	{ 
		for (size_t i = 0; i < count; ++i)
			--image_owners[index + i];
	}

	bool needs_update = true;
	bool images_need_update = true;

	bool operator==(const RenderedInstance& instance) const
	{
		return *this->instance == instance;
	}
};