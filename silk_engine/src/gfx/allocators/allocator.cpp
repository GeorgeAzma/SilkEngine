#include "allocator.h"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include "gfx/graphics.h"
#include "gfx/instance.h"
#include "gfx/devices/logical_device.h"
#include "gfx/devices/physical_device.h"

Allocator::Allocator()
{
	VmaAllocatorCreateInfo allocator_info{};
	allocator_info.vulkanApiVersion = EnumInfo::apiVersion(Graphics::API_VERSION);
	allocator_info.instance = (const vk::Instance)*Graphics::instance;
	allocator_info.physicalDevice = (const vk::PhysicalDevice&)*Graphics::physical_device;
	allocator_info.device = (const vk::Device&)*Graphics::logical_device;
	
	vmaCreateAllocator(&allocator_info, &allocator);
}

Allocator::~Allocator()
{
	vmaDestroyAllocator(allocator);
}
