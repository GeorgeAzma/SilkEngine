#pragma once

#include "buffers/staging_buffer.h"
#include "image_view.h"
#include "sampler.h"

struct ImageProps
{
	uint32_t width = 0;
	uint32_t height = 0;
	VkFormat format = VK_FORMAT_B8G8R8A8_SRGB;

	VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	bool create_view = true;
	bool create_sampler = true;
};

class Image : NonCopyable
{
public:
	Image(const std::string& file);
	Image(const ImageProps& props);

	~Image();

	uint32_t getWidth() const { return props.width; }
	uint32_t getHeight() const { return props.height; }
	VkFormat getFormat() const { return props.format; }
	const ImageProps& getProps() const { return props; }
	const VkDescriptorImageInfo& getDescriptorInfo() const { return descriptor_image_info;  }

private:
	void load(const std::string& file);
	void create(const ImageProps& props);
	void transitionLayout(VkImageLayout newLayout);
	void copyBufferToImage();

public:
	static VkFormat getDefaultFormatFromChannelCount(int channels);

private:
	VkImage image;
	VkDescriptorImageInfo descriptor_image_info = {};
	std::unique_ptr<Sampler> sampler;
	std::unique_ptr<ImageView> view = nullptr;
	VkDeviceMemory memory;
	std::unique_ptr<StagingBuffer> staging_buffer = nullptr;
	ImageProps props = {};
};