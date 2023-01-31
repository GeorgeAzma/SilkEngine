#include "allocator.h"
#include "gfx/graphics.h"
#include "gfx/instance.h"
#include "gfx/devices/logical_device.h"
#include "gfx/devices/physical_device.h"

Allocator::Allocator()
{
	VmaAllocatorCreateInfo allocator_info{};
	allocator_info.vulkanApiVersion = Graphics::apiVersion(Graphics::API_VERSION);
	allocator_info.instance = *Graphics::instance;
	allocator_info.physicalDevice = *Graphics::physical_device;
	allocator_info.device = *Graphics::logical_device;	
	vmaCreateAllocator(&allocator_info, &allocator);
}

Allocator::~Allocator()
{
	vmaDestroyAllocator(allocator);
}