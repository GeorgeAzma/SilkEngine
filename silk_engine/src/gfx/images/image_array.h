#pragma once

#include "gfx/buffers/staging_buffer.h"
#include "image.h"
#include "image_view.h"
#include "sampler.h"
#include <vk_mem_alloc.h>

class ImageArray : NonCopyable
{
public:
	ImageArray(const std::vector<std::string>& files, const ImageProps& props = {});
	ImageArray(const ImageProps& props, size_t layer_count);

	~ImageArray();

	uint32_t getWidth() const { return props.width; }
	uint32_t getHeight() const { return props.height; }
	VkFormat getFormat() const { return props.format; }
	const ImageProps& getProps() const { return props; }
	const VkDescriptorImageInfo& getDescriptorInfo() const { return descriptor_image_info;  }

	static ImageData load(const std::vector<std::string>& files);

	operator const VkImage& () const { return image; }
	operator const VkDescriptorImageInfo& () const { return descriptor_image_info; }

private:
	struct TransitionInfo
	{
		VkPipelineStageFlags source_stage;
		VkPipelineStageFlags destination_stage;
		VkAccessFlags source_access_mask;
		VkAccessFlags destination_access_mask;
	};
private:
	void create(const ImageProps& props);
	void transitionLayout(VkImageLayout newLayout);
	void copyBufferToImageArray();
	void generateMipmaps();
	TransitionInfo getTransitionInfo(VkImageLayout oldLayout, VkImageLayout newLayout);

private:
	VkImage image = VK_NULL_HANDLE;
	VkDescriptorImageInfo descriptor_image_info = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED };
	unique<Sampler> sampler;
	unique<ImageView> view = nullptr;
	VmaAllocation allocation = VK_NULL_HANDLE;
	shared<StagingBuffer> staging_buffer = nullptr;
	ImageProps props = {};
	uint32_t mip_levels = 1;
	size_t layer_count = 1;
};