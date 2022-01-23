#pragma once

struct ImageArrayProps
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

	bool create_view = true;
	bool create_sampler = true;
};

class ImageArray
{
public:
	ImageArray(const std::vector<std::string>& files);

private:
	void load(const std::vector<std::string>& files);

private:
	ImageArrayProps props{};
	size_t layer_count;
};