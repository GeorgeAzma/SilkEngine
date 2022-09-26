#pragma once

#include "gfx/allocators/allocation.h"
#include "sampler.h"

class ImageView;

class Image : NonCopyable
{
public:
	enum class Type
	{
		_1D,
		_2D,
		_3D,
		CUBEMAP,
		_1D_ARRAY,
		_2D_ARRAY,
		CUBEMAP_ARRAY
	};

	static VkImageType getVulkanTypeFromType(Type type);
	static VkImageViewType getVulkanViewTypeFromType(Type type);

	enum class Format
	{
		RED = VK_FORMAT_R8_UNORM,
		RG = VK_FORMAT_R8G8_UNORM,
		RGB = VK_FORMAT_R8G8B8_UNORM,
		RGBA = VK_FORMAT_R8G8B8A8_UNORM,
		BGRA = VK_FORMAT_B8G8R8A8_UNORM,
		DEPTH_STENCIL = VK_FORMAT_D24_UNORM_S8_UINT,
		DEPTH = VK_FORMAT_D32_SFLOAT,
		STENCIL = VK_FORMAT_S8_UINT
	};

	static Format getFormatFromChannelCount(uint8_t channels);
	static VkImageAspectFlags getFormatVulkanAspectFlags(Format format);
	static uint8_t getFormatChannelCount(Format format);
	static size_t getFormatSize(Format format);
	static bool isStencilFormat(Format format);
	static bool isDepthFormat(Format format);
	static uint32_t calculateMipLevels(uint32_t width, uint32_t height = 1, uint32_t depth = 1);

	struct Props
	{
		uint32_t width = 1;
		uint32_t height = 1;
		uint32_t depth = 1;
		uint32_t layers = 1;
		Format format = Format::BGRA;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		Allocation::Props allocation_props{};
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		bool mipmap = false;
		VkFilter mipmap_filter = VK_FILTER_LINEAR;
		Sampler::Props sampler_props{};
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		bool create_view = true;
		bool create_sampler = true;
		const void* data = nullptr;
		Type type = Type::_2D;
	};

public:
	Image(const Props& props);
	Image(uint32_t width, Format format = Format::BGRA); // 1D
	Image(uint32_t width, uint32_t height, Format format = Format::BGRA); // 2D
	Image(std::string_view file, const Props& props = {}); // 2D
	Image(const std::array<std::string, 6>& files, const Props& props = {}); // Cubemap
	Image(const std::vector<std::string>& files, const Props& props = {}); // 2D Array
	Image(const std::vector<std::array<std::string, 6>>& files, const Props& props = {}); // Cubemap Array
	Image(VkImage image, Format format); // Constructor used for swap chain image creation ONLY
	~Image();

	uint32_t getWidth() const { return props.width; }
	uint32_t getHeight() const { return props.height; }
	float getAspectRatio() const { return float(getWidth()) / getHeight(); }
	uint32_t getChannelCount() const { return getFormatChannelCount(getFormat()); }
	uint32_t getLayers() const { return props.layers; }
	Format getFormat() const { return props.format; }
	uint32_t getPixelCount() const { return props.width * props.height * props.depth * props.layers; }
	size_t getSize() const { return getPixelCount() * getFormatSize(props.format); }
	uint32_t getMipLevels() const { return mip_levels; }
	const Props& getProps() const { return props; }
	Type getType() const { return props.type; }
	VkDescriptorImageInfo getDescriptorInfo() const;
	const VkImageLayout& getLayout() const { return layout; }
	const VkImageView& getView() const;
	const VkSampler& getSampler() const { return *sampler; }
	VkSampleCountFlagBits getSamples() const { return props.samples; }
	VkImageAspectFlags getAspectFlags() const { return getFormatVulkanAspectFlags(props.format); }
	Allocation getAllocation() const { return allocation; }

	void setData(const void* data, uint32_t base_layer = 0, uint32_t layers = 1);
	void getData(void* data, uint32_t base_layer = 0, uint32_t layers = 1);
	bool copyImage(const shared<Image>& destination, uint32_t layer = 0);
	void transitionLayout(VkImageLayout new_layout);
	void insertMemoryBarrier(VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkImageLayout new_layout, VkPipelineStageFlags source_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags destination_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, uint32_t base_mip_level = 0, uint32_t base_layer = 0);
	bool isFeatureSupported(VkFormatFeatureFlags feature) const;
	void copyFromBuffer(VkBuffer buffer, uint32_t base_layer = 0, uint32_t layers = 1);
	void copyToBuffer(VkBuffer buffer, uint32_t base_layer = 0, uint32_t layers = 1);

	operator const VkImage& () const { return image; }
	
protected:
	void create();
	void generateMipmaps();

protected:
	static void insertMemoryBarrier(const VkImage& image, VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkImageLayout old_layout, VkImageLayout new_layout, VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkImageAspectFlags aspect, uint32_t mip_levels, uint32_t base_mip_level, uint32_t layers, uint32_t base_layer);

protected:
	VkImage image = nullptr;
	unique<Sampler> sampler = nullptr;
	unique<ImageView> view = nullptr;
	VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
	Allocation allocation{};
	Props props = {};
	uint32_t mip_levels = 1;
};