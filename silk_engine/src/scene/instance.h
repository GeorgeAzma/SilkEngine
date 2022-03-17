#pragma once

#include "meshes/mesh.h"
#include "gfx/buffers/indirect_buffer.h"
#include "gfx/pipeline/graphics_pipeline.h"
#include "gfx/descriptors/descriptor_set.h"
#include "instance_images.h"

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

	bool operator==(const InstanceData& other) const;
};

struct RenderedInstance
{
	shared<Mesh> mesh;
	shared<GraphicsPipeline> material = nullptr;

	std::vector<shared<Image2D>> images;
	size_t instance_data_index = -1;
	size_t instance_batch_index = -1;

	bool operator==(const RenderedInstance& other) const;
};

struct InstanceBatch
{
	shared<RenderedInstance> instance;

	std::vector<InstanceData> instance_data;
	std::vector<shared<RenderedInstance>> instances;
	shared<VertexBuffer> instance_buffer = nullptr;
	InstanceImages instance_images;
	std::unordered_map<uint32_t, DescriptorSet> descriptor_sets;

	~InstanceBatch();

	void bind();

	bool needs_update = true;

	bool operator==(const RenderedInstance& instance) const;
};