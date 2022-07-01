#pragma once

#include "bitmap.h"
#include "image_format.h"
#include "gfx/buffers/staging_buffer.h"
#include "image_view.h"
#include "sampler.h"
#include "gfx/device_type.h"

struct ImageProps
{
	uint32_t width = 1;
	uint32_t height = 1;
	uint32_t depth = 1;
	uint32_t layers = 1;
	ImageFormat format = ImageFormat::BGRA;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	bool mipmap = false;
	VkFilter mipmap_filter = VK_FILTER_LINEAR;
	SamplerProps sampler_props{};
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	bool create_view = true;
	bool create_sampler = true;
	const void* data = nullptr;
	bool is_cubemap = false;
	bool is_1D = false;
};

class Image : NonCopyable
{
public:
	~Image();

	uint32_t getWidth() const { return props.width; }
	uint32_t getHeight() const { return props.height; }
	ImageFormat getFormat() const { return props.format; }
	size_t getSize() const { return props.width * props.height * props.depth * props.layers * ImageFormatEnum::getSize(props.format); }
	uint32_t getMipLevels() const { return mip_levels; }
	const ImageProps& getProps() const { return props; }
	const VkDescriptorImageInfo& getDescriptorInfo() const 
	{
		VkDescriptorImageInfo descriptor_image_info{};
		descriptor_image_info.imageLayout = layout;
		descriptor_image_info.imageView = *view;
		descriptor_image_info.sampler = *sampler;
		return descriptor_image_info; 
	}
	const VkImageLayout& getLayout() const { return layout; }
	const VkImageView& getView() const { return *view; }
	const VkSampler& getSampler() const { return *sampler; }
	VkSampleCountFlagBits getSamples() const { return props.samples; }
	VkImageAspectFlags getAspectFlags() const { return ImageFormatEnum::getVulkanAspectFlags(props.format); }
	VmaAllocation getAllocation() const { return allocation; }
	bool isMapped() const { return mapped; }

	void setData(void* data, uint32_t base_layer = 0, uint32_t layers = 1);
	void getData(void* data, uint32_t base_layer = 0, uint32_t layers = 1);
	bool copyImage(const shared<Image>& destination, uint32_t layer = 0);
	void transitionLayout(VkImageLayout new_layout);
	void insertMemoryBarrier(VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkImageLayout new_layout, VkPipelineStageFlags source_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags destination_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, uint32_t base_mip_level = 0, uint32_t base_layer = 0);
	bool isFeatureSupported(VkFormatFeatureFlags feature) const;
	void copyFromBuffer(VkBuffer buffer);
	void copyToBuffer(VkBuffer buffer, uint32_t base_layer = 0, uint32_t layers = 1);
	// Resize without preserving texture data
	// any parameter of 0 means "same size as before"
	// Note: function can't change type of the image, swap chain images can't be reallocated
	void reallocate(uint32_t width = 0, uint32_t height = 0, uint32_t depth = 0, uint32_t layers = 0);
	
	void map(void** data) const;
	void unmap() const;

	operator const VkImage& () const { return image; }
	
protected:
	void create(const ImageProps& props);
	void generateMipmaps();

protected:
	static void insertMemoryBarrier(const VkImage& image, VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkImageLayout old_layout, VkImageLayout new_layout, VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkImageAspectFlags aspect, uint32_t mip_levels, uint32_t base_mip_level, uint32_t layers, uint32_t base_layer);

protected:
	VkImage image = VK_NULL_HANDLE;
	unique<Sampler> sampler = nullptr;
	unique<ImageView> view = nullptr;
	VkImageLayout layout = VK_IMAGE_LAYOUT_MAX_ENUM;
	VmaAllocation allocation = VK_NULL_HANDLE;
	ImageProps props = {};
	uint32_t mip_levels = 1;
	mutable bool mapped = false;
	bool needs_staging = false;
	bool swap_chain_property = false;
};