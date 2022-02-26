#include "descriptor_pool.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

DescriptorPool::~DescriptorPool()
{
	Graphics::logical_device->destroyDescriptorPool(descriptor_pool);
}

DescriptorPool& DescriptorPool::addSize(vk::DescriptorType type, uint32_t count)
{
	sizes.emplace_back(type, count);
	return *this;
}

DescriptorPool& DescriptorPool::setMaxSets(uint32_t count)
{
	max_sets = count;
	return *this;
}

void DescriptorPool::build()
{
	vk::DescriptorPoolCreateInfo ci{};
	ci.poolSizeCount = sizes.size();
	ci.pPoolSizes = sizes.data();
	ci.maxSets = max_sets;
	descriptor_pool = Graphics::logical_device->createDescriptorPool(ci);
}
