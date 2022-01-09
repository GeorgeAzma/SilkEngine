#pragma once

#include "buffers/staging_buffer.h"

struct ImageProps
{
	uint32_t width = 0;
	uint32_t height = 0;
	VkFormat format = VK_FORMAT_B8G8R8A8_SRGB;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
};

class Image : NonCopyable
{
public:
	Image(const std::string& file);
	Image(const ImageProps& props);

	~Image();

	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }

private:
	void load(const std::string& file);
	void create(const ImageProps& props);
	
public:
	static VkFormat getDefaultFormatFromChannelCount(int channels);
	static VkFormat getChannelCountFromFormat(int channels);

private:
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t channels = 0;
	VkImage image;
	VkDeviceMemory memory;
	std::unique_ptr<StagingBuffer> staging_buffer = nullptr;
};