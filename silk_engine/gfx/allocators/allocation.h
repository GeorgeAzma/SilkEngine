#pragma once

#include <vk_mem_alloc.h>

class Allocation
{
public:
	typedef VmaAllocationCreateFlags Flags;
	enum FlagBits : Flags
	{
		MAPPED = VMA_ALLOCATION_CREATE_MAPPED_BIT,
		ALLOW_TRANSFER_INSTEAD = VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT,
		DEDICATED_MEMORY = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
		RANDOM_ACCESS = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
		SEQUENTIAL_WRITE = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
	};
	
	enum class Device
	{
		AUTO = VMA_MEMORY_USAGE_AUTO,
		GPU = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		CPU = VMA_MEMORY_USAGE_AUTO_PREFER_HOST
	};

	struct Props
	{
		Flags flags = 0;
		Device preferred_device = Device::AUTO;
		float priority = 0.0f;
	};

public:
	Allocation(VmaAllocation allocation = nullptr);

	void map(void** data) const;
	void* getMappedData() const;
	void unmap() const;
	bool isHostVisible() const;
	bool isPersistentMap() const;
	VkDeviceSize getAlignment() const;
	void setData(const void* data, VkDeviceSize size, VkDeviceSize offset = 0) const;
	void getData(void* data, VkDeviceSize size, VkDeviceSize offset = 0) const;

	operator const VmaAllocation& () const { return allocation; }

private:
	VmaAllocation allocation = nullptr;
};