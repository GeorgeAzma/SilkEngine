#include "descriptor_pool.h"
#include "gfx/render_context.h"
#include "gfx/devices/logical_device.h"

DescriptorPool::DescriptorPool(uint32_t max_sets, const std::vector<VkDescriptorPoolSize>& sizes)
{
	VkDescriptorPoolCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	ci.poolSizeCount = sizes.size();
	ci.pPoolSizes = sizes.data();
	ci.maxSets = max_sets;
	descriptor_pool = RenderContext::getLogicalDevice().createDescriptorPool(ci);
}

DescriptorPool::~DescriptorPool()
{
	RenderContext::getLogicalDevice().destroyDescriptorPool(descriptor_pool);
}

VkResult DescriptorPool::allocate(VkDescriptorSet& descriptor_set, const VkDescriptorSetLayout& descriptor_set_layout) const
{
	++allocated_descriptor_sets;
	VkDescriptorSetAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = descriptor_pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &descriptor_set_layout;
	return RenderContext::getLogicalDevice().allocateDescriptorSets(alloc_info, descriptor_set);
}

void DescriptorPool::deallocate() const
{
	SK_VERIFY(allocated_descriptor_sets > 0, "Can't deallocate pool's descriptor set when it doesn't have any");
	--allocated_descriptor_sets;
}
