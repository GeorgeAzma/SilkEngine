#pragma once

#include "meshes/mesh.h"
#include "gfx/pipeline/graphics_pipeline.h"
#include "gfx/descriptors/descriptor_set.h"
#include "instance_images.h"

struct CullData
{
	vec3 min;
	uint32_t index;
	vec3 max;
	uint32_t count;
	std::array<vec4, 6> planes;
};

struct InstanceData
{
	mat4 transform = mat4(1);
	uint32_t image_index = 0;
	vec4 color = vec4(1);

	bool operator==(const InstanceData& other) const;
};

struct RenderedInstance
{
	shared<GraphicsPipeline> pipeline = nullptr;
	std::vector<shared<Image>> images;
	size_t instance_data_index = std::numeric_limits<size_t>::max();
	size_t instance_batch_index = std::numeric_limits<size_t>::max();

	bool operator==(const RenderedInstance& other) const;
};

struct InstanceBatch
{
	shared<Mesh> mesh = nullptr;
	shared<RenderedInstance> instance = nullptr;

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