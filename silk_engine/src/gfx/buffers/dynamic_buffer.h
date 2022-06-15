#pragma once

#include "buffer.h"

template<typename T = uint8_t>
class DynamicBuffer : public Buffer
{
public:
	DynamicBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage vma_usage = VMA_MEMORY_USAGE_CPU_TO_GPU);

	void resize(VkDeviceSize size);

	void update(size_t size = 0, size_t offset = 0);
	T& operator[](size_t index) { return buffer_data[index]; }
	T* data() { return buffer_data.data(); }
	size_t getSize() const { return buffer_data.size(); }
	std::vector<T>::iterator begin() { return buffer_data.begin(); }
	std::vector<T>::iterator end() { return buffer_data.end(); }
	std::vector<T>::const_iterator cbegin() const { return buffer_data.cbegin(); }
	std::vector<T>::const_iterator cend() const { return buffer_data.cend(); }

private:
	std::vector<T> buffer_data{};

private:
	using Buffer::map;
	using Buffer::unmap;
	using Buffer::setData;
	using Buffer::getData;
	using Buffer::isMapped;
};