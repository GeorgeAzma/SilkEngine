#include "instance.h"
#include "gfx/graphics.h"
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
	return (*mesh == *other.mesh && material == other.material);
}

InstanceBatch::~InstanceBatch()
{
	Graphics::logical_device->waitIdle();
}

void InstanceBatch::bind()
{
	instance->material->bind();
	for (auto& descriptor_set : descriptor_sets)
		descriptor_set.second.bind(descriptor_set.first);
	instance->mesh->getVertexArray()->bind();
	instance_buffer->bind(1);
}

bool InstanceBatch::operator==(const RenderedInstance& instance) const
{
	return *this->instance == instance;
}
