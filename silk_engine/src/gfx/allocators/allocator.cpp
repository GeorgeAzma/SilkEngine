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

bool Allocator::needsStaging(VmaMemoryUsage usage)
{
	switch (usage)
	{
	case VMA_MEMORY_USAGE_AUTO:
	case VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE:
	case VMA_MEMORY_USAGE_CPU_COPY:
	case VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED:
	case VMA_MEMORY_USAGE_GPU_ONLY:
		return true;
	}

	return false;
}