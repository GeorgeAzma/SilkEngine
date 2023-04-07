#include "instance.h"
#include "gfx/render_context.h"
#include "gfx/window/swap_chain.h"
#include "gfx/devices/logical_device.h"

bool InstanceData::operator==(const InstanceData& other) const
{
	return transform == other.transform
		&& image_index == other.image_index
		&& color == color;
}

bool RenderedInstance::operator==(const RenderedInstance& other) const
{
	return pipeline == other.pipeline;
}

InstanceBatch::~InstanceBatch()
{
}

void InstanceBatch::bind()
{
	instance->pipeline->bind();
	for (auto& descriptor_set : descriptor_sets)
		descriptor_set.second.bind(descriptor_set.first);
	mesh->getVertexArray()->bind();
	instance_buffer->bind(1);
}

bool InstanceBatch::operator==(const RenderedInstance& instance) const
{
	return *this->instance == instance;
}
