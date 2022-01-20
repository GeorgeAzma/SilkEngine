#include "allocator.h"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include "graphics.h"

Allocator::Allocator()
{
	VmaAllocatorCreateInfo allocator_info{};
	allocator_info.vulkanApiVersion = Graphics::API_VERSION;
	allocator_info.instance = *Graphics::instance;
	allocator_info.physicalDevice = *Graphics::physical_device;
	allocator_info.device = *Graphics::logical_device;
	
	vmaCreateAllocator(&allocator_info, &allocator);
}

Allocator::~Allocator()
{
	vmaDestroyAllocator(allocator);
}
