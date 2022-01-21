#include "descriptor_pool.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(*Graphics::logical_device, descriptor_pool, nullptr);
}

DescriptorPool& DescriptorPool::addSize(VkDescriptorType type, uint32_t count)
{
	VkDescriptorPoolSize size{};
	size.type = type;
	size.descriptorCount = count;

	sizes.emplace_back(std::move(size));

	return *this;
}

DescriptorPool& DescriptorPool::setMaxSets(uint32_t count)
{
	max_sets = count;
	return *this;
}

void DescriptorPool::build()
{
	VkDescriptorPoolCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	create_info.poolSizeCount = sizes.size();
	create_info.pPoolSizes = sizes.data();
	create_info.maxSets = max_sets;

	Graphics::vulkanAssert(vkCreateDescriptorPool(*Graphics::logical_device, &create_info, nullptr, &descriptor_pool));
}
