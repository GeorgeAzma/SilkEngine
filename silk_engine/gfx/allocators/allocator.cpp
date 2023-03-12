#include "allocator.h"
#include "gfx/graphics.h"
#include "gfx/instance.h"
#include "gfx/devices/logical_device.h"
#include "gfx/devices/physical_device.h"

Allocator::Allocator()
{
	VmaAllocatorCreateInfo allocator_info{};
	allocator_info.vulkanApiVersion = uint32_t(Graphics::API_VERSION);
	allocator_info.instance = *Graphics::instance;
	allocator_info.physicalDevice = *Graphics::physical_device;
	allocator_info.device = *Graphics::logical_device;
	allocator_info.flags = Graphics::physical_device->supportsExtension("VK_EXT_memory_priority") * VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
	vmaCreateAllocator(&allocator_info, &allocator);
}

Allocator::~Allocator()
{
	vmaDestroyAllocator(allocator);
}