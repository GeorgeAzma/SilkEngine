#pragma once

#include "gfx/buffers/staging_buffer.h"
#include "image_view.h"
#include "sampler.h"
#include <vk_mem_alloc.h>

struct CubemapProps
{
	uint32_t width = 0;
	uint32_t height = 0;
	VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	bool mipmap = true;
	VkFilter mipmap_filter = VK_FILTER_LINEAR;
	SamplerProps sampler_props{};
	bool create_view = true;
	bool create_sampler = true;
	const void* data = nullptr;
	VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct Image1DProps
{
	uint32_t width = 0;
	VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	bool mipmap = true;
	VkFilter mipmap_filter = VK_FILTER_LINEAR;
	SamplerProps sampler_props{};
	bool create_view = true;
	bool create_sampler = true;
	const void* data = nullptr;
	VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct Image2DProps
{
	uint32_t width = 0;
	uint32_t height = 0;
	VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	bool mipmap = true;
	VkFilter mipmap_filter = VK_FILTER_LINEAR;
	SamplerProps sampler_props{};
	bool create_view = true;
	bool create_sampler = true;
	const void* data = nullptr;
	VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct ImageArrayProps
{
	uint32_t width = 0;
	uint32_t height = 0;
	VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	bool mipmap = true;
	VkFilter mipmap_filter = VK_FILTER_LINEAR;
	SamplerProps sampler_props{};
	bool create_view = true;
	bool create_sampler = true;
	const void* data = nullptr;
	uint32_t array_layers = 1;
	VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct ImageProps
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t depth = 1;
	VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	bool mipmap = true;
	VkFilter mipmap_filter = VK_FILTER_LINEAR;
	SamplerProps sampler_props{};
	bool create_view = true;
	bool create_sampler = true;
	const void* data = nullptr;
	uint32_t array_layers = 1;
	VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY; //Currently has no use and should always be GPU_ONLY
	bool is_cubemap = false;
	bool is_1D = false;
	VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;

	ImageProps& operator=(const Image1DProps& props)
	{
		width = props.width;
		format = props.format;
		usage = props.usage;
		samples = props.samples;
		tiling = props.tiling;
		layout = props.layout;
		mipmap = props.mipmap;
		mipmap_filter = props.mipmap_filter;
		sampler_props = props.sampler_props;
		create_view = props.create_view;
		create_sampler = props.create_sampler;
		data = props.data;
		memory_usage = props.memory_usage;
		height = 1;
		array_layers = 1;
		depth = 1;
		is_cubemap = false;
		is_1D = false;
		initial_layout = props.initial_layout;

		return *this;
	}

	ImageProps& operator=(const Image2DProps& props)
	{
		width = props.width;
		height = props.height;
		format = props.format;
		usage = props.usage;
		samples = props.samples;
		tiling = props.tiling;
		layout = props.layout;
		mipmap = props.mipmap;
		mipmap_filter = props.mipmap_filter;
		sampler_props = props.sampler_props;
		create_view = props.create_view;
		create_sampler = props.create_sampler;
		data = props.data;
		memory_usage = props.memory_usage;
		array_layers = 1;
		depth = 1;
		is_cubemap = false;
		is_1D = false;
		initial_layout = props.initial_layout;

		return *this;
	}

	ImageProps& operator=(const CubemapProps& props)
	{
		width = props.width;
		height = props.height;
		format = props.format;
		usage = props.usage;
		samples = props.samples;
		tiling = props.tiling;
		layout = props.layout;
		mipmap = props.mipmap;
		mipmap_filter = props.mipmap_filter;
		sampler_props = props.sampler_props;
		create_view = props.create_view;
		create_sampler = props.create_sampler;
		data = props.data;
		memory_usage = props.memory_usage;
		array_layers = 1;
		depth = 1;
		is_cubemap = true;
		is_1D = false;
		initial_layout = props.initial_layout;

		return *this;
	}

	ImageProps& operator=(const ImageArrayProps& props)
	{
		width = props.width;
		height = props.height;
		format = props.format;
		usage = props.usage;
		samples = props.samples;
		tiling = props.tiling;
		layout = props.layout;
		mipmap = props.mipmap;
		mipmap_filter = props.mipmap_filter;
		sampler_props = props.sampler_props;
		create_view = props.create_view;
		create_sampler = props.create_sampler;
		data = props.data;
		array_layers = props.array_layers;
		memory_usage = props.memory_usage;
		depth = 1;
		is_cubemap = false;
		is_1D = false;
		initial_layout = props.initial_layout;

		return *this;
	}
};

struct Bitmap
{
	int width;
	int height;
	int channels;
	std::vector<uint8_t> data;
};

class Image : NonCopyable
{
public:
	~Image();

	uint32_t getWidth() const { return props.width; }
	uint32_t getHeight() const { return props.height; }
	VkFormat getFormat() const { return props.format; }
	uint32_t mipLevels() const { return mip_levels; }
	const ImageProps& getProps() const { return props; }
	const VkDescriptorImageInfo& getDescriptorInfo() const { return descriptor_image_info; }
	void setLayout(VkImageLayout layout) { descriptor_image_info.imageLayout = layout; }
	VmaAllocation getAllocation() const { return allocation; }

	static void align4(Bitmap& image);

	void setData(void* data, uint32_t array_layers, uint32_t base_array_layer);
	//Copies data from the image to the pointer provided
	void getData(void* data, uint32_t array_layer = 0);
	bool copyImage(shared<Image> destination, uint32_t array_layer = 0);
	void transitionLayout(VkImageLayout new_layout);
	void copyBufferToImage(VkBuffer buffer);
	
	void map(void** data) const;
	void unmap() const;

	operator const VkImage& () const { return image; }
	operator const VkDescriptorImageInfo& () const { return descriptor_image_info; }

protected:
	void create(const ImageProps& props);
	void generateMipmaps();

protected:
	static void insertMemoryBarrier(const VkImage& image, VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkImageLayout old_layout, VkImageLayout new_layout, VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkImageAspectFlags aspect, uint32_t mip_levels, uint32_t base_mip_level, uint32_t array_layers, uint32_t base_array_layer);

public:
	static VkFormat getDefaultFormatFromChannelCount(int channels);
	static bool hasStencil(VkFormat format);
	static bool hasDepth(VkFormat format);
	static VkImageAspectFlags getAspectFlags(VkFormat format);
	static size_t channelCount(VkFormat format);

protected:
	VkImage image = VK_NULL_HANDLE;
	VkDescriptorImageInfo descriptor_image_info = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED };
	unique<Sampler> sampler;
	unique<ImageView> view = nullptr;
	VmaAllocation allocation = VK_NULL_HANDLE;
	ImageProps props = {};
	uint32_t mip_levels = 1;
	mutable bool mapped = false;
};