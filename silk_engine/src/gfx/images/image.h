#pragma once

#include "gfx/buffers/staging_buffer.h"
#include "gfx/enums.h"
#include "image_view.h"
#include "sampler.h"

struct CubemapProps
{
	uint32_t width = 0;
	uint32_t height = 0;
	vk::Format format = vk::Format::eB8G8R8A8Unorm;
	vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
	vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
	vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
	bool mipmap = true;
	vk::Filter mipmap_filter = vk::Filter::eLinear;
	SamplerProps sampler_props{};
	bool create_view = true;
	bool create_sampler = true;
	const void* data = nullptr;
	VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	vk::ImageLayout initial_layout = vk::ImageLayout::eUndefined;
};

struct Image1DProps
{
	uint32_t width = 0;
	vk::Format format = vk::Format::eB8G8R8A8Unorm;
	vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
	vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
	vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
	bool mipmap = true;
	vk::Filter mipmap_filter = vk::Filter::eLinear;
	SamplerProps sampler_props{};
	bool create_view = true;
	bool create_sampler = true;
	const void* data = nullptr;
	VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	vk::ImageLayout initial_layout = vk::ImageLayout::eUndefined;
};

struct Image2DProps
{
	uint32_t width = 0;
	uint32_t height = 0;
	vk::Format format = vk::Format::eB8G8R8A8Unorm;
	vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
	vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
	vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
	bool mipmap = true;
	vk::Filter mipmap_filter = vk::Filter::eLinear;
	SamplerProps sampler_props{};
	bool create_view = true;
	bool create_sampler = true;
	const void* data = nullptr;
	VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	vk::ImageLayout initial_layout = vk::ImageLayout::eUndefined;
};

struct ImageArrayProps
{
	uint32_t width = 0;
	uint32_t height = 0;
	vk::Format format = vk::Format::eB8G8R8A8Unorm;
	vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
	vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
	vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
	bool mipmap = true;
	vk::Filter mipmap_filter = vk::Filter::eLinear;
	SamplerProps sampler_props{};
	bool create_view = true;
	bool create_sampler = true;
	const void* data = nullptr;
	uint32_t array_layers = 1;
	VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	vk::ImageLayout initial_layout = vk::ImageLayout::eUndefined;
};

struct ImageProps
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t depth = 1;
	vk::Format format = vk::Format::eB8G8R8A8Unorm;
	vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
	vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
	vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
	bool mipmap = true;
	vk::Filter mipmap_filter = vk::Filter::eLinear;
	SamplerProps sampler_props{};
	bool create_view = true;
	bool create_sampler = true;
	const void* data = nullptr;
	uint32_t array_layers = 1;
	VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	bool is_cubemap = false;
	bool is_1D = false;
	vk::ImageLayout initial_layout = vk::ImageLayout::eUndefined;

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

	size_t size() const { return width * height * channels * sizeof(uint8_t); }
};

class Image : NonCopyable
{
public:
	~Image();

	uint32_t getWidth() const { return props.width; }
	uint32_t getHeight() const { return props.height; }
	vk::Format getFormat() const { return props.format; }
	size_t getSize() const { return props.width * props.height * props.depth * props.array_layers * formatSize(props.format); }
	uint32_t mipLevels() const { return mip_levels; }
	const ImageProps& getProps() const { return props; }
	const vk::DescriptorImageInfo& getDescriptorInfo() const { return descriptor_image_info; }
	void setLayout(vk::ImageLayout layout) { descriptor_image_info.imageLayout = layout; }
	VmaAllocation getAllocation() const { return allocation; }
	vk::ImageAspectFlags getAspectFlags() const { return getAspectFlags(props.format); }
	bool isMapped() const { return mapped; }

	static void align4(Bitmap& image);

	void setData(void* data, uint32_t base_array_layer = 0, uint32_t array_layers = 1);
	void getData(void* data, uint32_t base_array_layer = 0, uint32_t array_layers = 1);
	bool copyImage(shared<Image> destination, uint32_t array_layer = 0);
	void transitionLayout(vk::ImageLayout new_layout);
	void insertMemoryBarrier(vk::AccessFlags source_access_mask, vk::AccessFlags destination_access_mask, vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::PipelineStageFlags source_stage_mask = vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlags destination_stage_mask = vk::PipelineStageFlagBits::eAllCommands, uint32_t base_mip_level = 0, uint32_t base_array_layer = 0);
	bool isFeatureSupported(vk::FormatFeatureFlags feature) const;
	void copyFromBuffer(vk::Buffer buffer);
	void copyToBuffer(vk::Buffer buffer, uint32_t base_array_layer = 0, uint32_t array_layers = 1);
	
	void map(void** data) const;
	void unmap() const;

	operator const vk::Image& () const { return image; }
	operator const vk::DescriptorImageInfo& () const { return descriptor_image_info; }

protected:
	void create(const ImageProps& props);
	void generateMipmaps();

protected:
	static void insertMemoryBarrier(const vk::Image& image, vk::AccessFlags source_access_mask, vk::AccessFlags destination_access_mask, vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::PipelineStageFlags source_stage_mask, vk::PipelineStageFlags destination_stage_mask, vk::ImageAspectFlags aspect, uint32_t mip_levels, uint32_t base_mip_level, uint32_t array_layers, uint32_t base_array_layer);

public:
	static vk::Format getDefaultFormatFromChannelCount(int channels);
	static bool hasStencil(vk::Format format);
	static bool hasDepth(vk::Format format);
	static vk::ImageAspectFlags getAspectFlags(vk::Format format);
	static size_t channelCount(vk::Format format);
	static Type formatToType(vk::Format format);
	static size_t formatSize(vk::Format format);

protected:
	vk::Image image = VK_NULL_HANDLE;
	vk::DescriptorImageInfo descriptor_image_info = vk::DescriptorImageInfo(VkDescriptorImageInfo{ VK_NULL_HANDLE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED });
	unique<Sampler> sampler;
	unique<ImageView> view = nullptr;
	VmaAllocation allocation = VK_NULL_HANDLE;
	ImageProps props = {};
	uint32_t mip_levels = 1;
	mutable bool mapped = false;
	bool needs_staging = false;
};