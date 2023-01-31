#include "allocation.h"
#include "allocator.h"
#include "gfx/graphics.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

void Allocation::map(void** data) const
{
	if (!(*data = allocation->GetMappedData()))
		vmaMapMemory(*Graphics::allocator, allocation, data);
}

void* Allocation::getMappedData() const 
{ 
	return allocation->GetMappedData(); 
}

void Allocation::unmap() const
{
	if (allocation->GetMappedData() && !allocation->IsPersistentMap())
		vmaUnmapMemory(*Graphics::allocator, allocation);
}

bool Allocation::isHostVisible() const
{
	VkMemoryPropertyFlags memory_propery_flags;
	vmaGetAllocationMemoryProperties(*Graphics::allocator, allocation, &memory_propery_flags);
	return (memory_propery_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
}

bool Allocation::isPersistentMap() const
{
	return allocation->IsPersistentMap();
}

VkDeviceSize Allocation::getAlignment() const
{
	return allocation->GetAlignment();
}

void Allocation::setData(const void* data, VkDeviceSize size, VkDeviceSize offset) const
{
	if (!size || !data)
		return;
	void* destination;
	map(&destination);
	memcpy((uint8_t*)destination + offset, data, size);
	unmap();
}

void Allocation::getData(void* data, VkDeviceSize size, VkDeviceSize offset) const
{
	void* source;
	map(&source);
	memcpy(data, (const uint8_t*)source + offset, size);
	unmap();
}