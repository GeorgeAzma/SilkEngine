#pragma once

#include "gfx/buffers/staging_buffer.h"
#include "image_view.h"
#include "sampler.h"
#include <vk_mem_alloc.h>

struct ImageProps
{
	uint32_t width = 0;
	uint32_t height = 0;
	VkFormat format = VK_FORMAT_B8G8R8A8_SRGB;

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
};

struct StagedImageData
{
	int width;
	int height;
	int channels;
	shared<StagingBuffer> staging_buffer;
};

struct ImageData
{
	int width;
	int height;
	int channels;
	std::vector<uint8_t> data;
};

class Image : NonCopyable
{
public:
	Image(const std::string& file, const ImageProps& props = {});
	Image(const ImageProps& props);
	Image(VkImage image, const ImageProps& props);

	~Image();

	uint32_t getWidth() const { return props.width; }
	uint32_t getHeight() const { return props.height; }
	VkFormat getFormat() const { return props.format; }
	uint32_t mipLevels() const { return mip_levels; }
	std::string getPath() const { return path; }
	const ImageProps& getProps() const { return props; }
	const VkDescriptorImageInfo& getDescriptorInfo() const { return descriptor_image_info; }

	static ImageData load(const std::string& file);
	static void align4(ImageData& image);
	
	operator const VkImage& () const { return image; }
	operator const VkDescriptorImageInfo& () const { return descriptor_image_info; }

private:
	void create(const ImageProps& props);
	void transitionLayout(VkImageLayout new_layout);
	void copyBufferToImage();
	void generateMipmaps();

public:
	static VkFormat getDefaultFormatFromChannelCount(int channels);

private:
	VkImage image = VK_NULL_HANDLE;
	VkDescriptorImageInfo descriptor_image_info = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED };
	unique<Sampler> sampler;
	unique<ImageView> view = nullptr;
	VmaAllocation allocation = VK_NULL_HANDLE;
	shared<StagingBuffer> staging_buffer = nullptr;
	ImageProps props = {};
	uint32_t mip_levels = 1;
	std::string path;
};