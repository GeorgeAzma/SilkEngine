#include "dynamic_buffer.h"
#include "gfx/graphics.h"

template<typename T>
DynamicBuffer<T>::DynamicBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage vma_usage)
	: Buffer(size * sizeof(T), usage, vma_usage), buffer_data(size)
{
}

template<typename T>
void DynamicBuffer<T>::resize(VkDeviceSize size)
{
	if (size * sizeof(T) == this->size)
		return;
	this->size = size * sizeof(T);
	ci.size = size * sizeof(T);
	vmaDestroyBuffer(*Graphics::allocator, buffer, allocation);
	Graphics::vulkanAssert(vmaCreateBuffer(*Graphics::allocator, &ci, &allocation_ci, &buffer, &allocation, nullptr));
	buffer_data.resize(size);
	update();
	SK_TRACE("Buffer resized: {}", size);
}

template<typename T>
void DynamicBuffer<T>::update(size_t size, size_t offset)
{
	Buffer::setData(buffer_data.data(), (size ? size : buffer_data.size()) * sizeof(T), offset);
}
