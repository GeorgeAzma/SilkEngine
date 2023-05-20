#pragma once

#include "gfx/allocators/allocation.h"
#include "sampler.h"

class ImageView;

class Image : NoCopy
{
public:
	enum class Type : std::underlying_type_t<VkImageViewType>
	{
		_1D = VK_IMAGE_VIEW_TYPE_1D,
		_2D = VK_IMAGE_VIEW_TYPE_2D,
		_3D = VK_IMAGE_VIEW_TYPE_3D,
		CUBE = VK_IMAGE_VIEW_TYPE_CUBE,
		_1D_ARRAY = VK_IMAGE_VIEW_TYPE_1D_ARRAY,
		_2D_ARRAY = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
		CUBE_ARRAY = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY
	};

	static VkImageType getVulkanTypeFromType(Type type);

	enum class Format : std::underlying_type_t<VkFormat>
	{
		RED = VK_FORMAT_R8_UNORM,
		RG = VK_FORMAT_R8G8_UNORM,
		RGB = VK_FORMAT_R8G8B8_UNORM,
		RGBA = VK_FORMAT_R8G8B8A8_UNORM,
		BGRA = VK_FORMAT_B8G8R8A8_UNORM,
		DEPTH16 = VK_FORMAT_D16_UNORM,
		DEPTH = VK_FORMAT_D32_SFLOAT,
		STENCIL = VK_FORMAT_S8_UINT,
		DEPTH16_STENCIL = VK_FORMAT_D16_UNORM_S8_UINT,
		DEPTH24_STENCIL = VK_FORMAT_D24_UNORM_S8_UINT,
		DEPTH_STENCIL = VK_FORMAT_D32_SFLOAT_S8_UINT
	};

	typedef VkBufferUsageFlags Usage;
	enum UsageBits : Usage
	{
		TRANSFER_SRC = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		TRANSFER_DST = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		SAMPLED = VK_IMAGE_USAGE_SAMPLED_BIT,
		STORAGE = VK_IMAGE_USAGE_STORAGE_BIT,
		COLOR_ATTACHMENT = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		DEPTH_STENCIL_ATTACHMENT = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		TRANSIENT_ATTACHMENT = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
		INPUT_ATTACHMENT = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
	};

	static Format getFormatFromChannelCount(uint8_t channels);
	static VkImageAspectFlags getFormatVulkanAspectFlags(Format format);
	static uint8_t getFormatChannelCount(Format format);
	static size_t getFormatSize(Format format);
	static bool isStencilFormat(Format format)
	{
		return format >= Format::STENCIL && format <= Format::DEPTH_STENCIL;
	}
	static bool isDepthFormat(Format format)
	{
		return format >= Format::DEPTH16 && format <= Format::DEPTH_STENCIL && format != Format::STENCIL;
	}
	static bool isDepthStencilFormat(Format format)
	{
		return format >= Format::DEPTH16 && format <= Format::DEPTH_STENCIL;
	}
	static bool isStencilOnlyFormat(Format format)
	{
		return format == Format::STENCIL;
	}
	static bool isDepthOnlyFormat(Format format)
	{
		return format == Format::DEPTH || format == Format::DEPTH16;
	}
	static bool isDepthStencilOnlyFormat(Format format)
	{
		return format == Format::DEPTH24_STENCIL || format == Format::DEPTH16_STENCIL;
	}
	static bool isColorFormat(Format format)
	{
		return !isDepthStencilFormat(format);
	}
	static uint32_t calculateMipLevels(uint32_t width, uint32_t height = 1, uint32_t depth = 1);

	struct Props
	{
		uint32_t width = 1;
		uint32_t height = 1;
		uint32_t depth = 1;
		uint32_t layers = 1;
		Format format = Format::BGRA;
		Usage usage = TRANSFER_DST | SAMPLED;
		Allocation::Props allocation_props{};
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		Sampler::Props sampler_props{};
		bool linear_tiling = false;
		Type type = Type::_2D;

		uint32_t getChannelCount() const { return getFormatChannelCount(format); }
		size_t getPixelCount() const { return width * height * depth * layers; }
		size_t getSize() const { return getPixelCount() * getFormatSize(format); }
	};

public:
	Image(const Props& props);
	Image(uint32_t width, Format format = Format::BGRA); // 1D
	Image(uint32_t width, uint32_t height, Format format = Format::BGRA); // 2D
	Image(const fs::path& file, const Props& props = {}); // 2D
	Image(const std::array<fs::path, 6>& files, const Props& props = {}); // Cubemap
	Image(std::span<const fs::path> files, const Props& props = {}); // 2D Array
	Image(std::span<const std::array<fs::path, 6>> files, const Props& props = {}); // Cubemap Array
	Image(uint32_t width, uint32_t height, Format format, VkImage img); // Constructor used for swap chain image creation ONLY
	~Image();

	uint32_t getWidth() const { return props.width; }
	uint32_t getHeight() const { return props.height; }
	uint32_t getDepth() const { return props.depth; }
	uint32_t getLayers() const { return props.layers; }
	Format getFormat() const { return props.format; }
	Usage getUsage() const { return props.usage; }
	VkSampleCountFlagBits getSamples() const { return props.samples; }
	Type getType() const { return props.type; }

	bool isSampled() const { return props.usage == VK_IMAGE_USAGE_SAMPLED_BIT; }
	uint32_t getChannelCount() const { return props.getChannelCount(); }
	size_t getPixelCount() const { return props.getPixelCount(); }
	float getAspectRatio() const { return float(getWidth()) / getHeight(); }
	size_t getSize() const { return props.getSize(); }
	uint32_t getMipLevels() const { return mip_levels; }
	VkImageAspectFlags getAspectFlags() const { return getFormatVulkanAspectFlags(props.format); }

	const VkImageLayout& getLayout() const { return layout; }
	const VkImageView& getView() const;
	const VkSampler& getSampler() const { return *sampler; }
	VkDescriptorImageInfo getDescriptorInfo() const;
	const Allocation& getAllocation() const { return allocation; }
	operator const VkImage& () const { return image; }

	void setLayout(VkImageLayout layout) { this->layout = layout; } // Use sparingly
	void setData(const void* data, uint32_t base_layer = 0, uint32_t layers = 1);
	void getData(void* data, uint32_t base_layer = 0, uint32_t layers = 1);
	bool copyToImage(Image& destination);
	void copyFromBuffer(VkBuffer buffer, uint32_t base_layer = 0, uint32_t layers = 1);
	void copyToBuffer(VkBuffer buffer, uint32_t base_layer = 0, uint32_t layers = 1);
	void transitionLayout(VkImageLayout new_layout, VkDependencyFlags dependency = 0, uint32_t mip_levels = 0, uint32_t base_mip_level = 0, uint32_t layers = 0, uint32_t base_layer = 0);
	bool isFeatureSupported(VkFormatFeatureFlags feature) const;
	
protected:
	void create();
	void generateMipmaps();

protected:
	static void insertMemoryBarrier(const VkImage& image, VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkImageLayout old_layout, VkImageLayout new_layout, VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkDependencyFlags depencency = 0, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mip_levels = 1, uint32_t base_mip_level = 0, uint32_t layers = 1, uint32_t base_layer = 0);
	static void insertMemoryBarrier(const VkImage& image, VkImageLayout old_layout, VkImageLayout new_layout, VkDependencyFlags dependency = 0, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mip_levels = 1, uint32_t base_mip_level = 0, uint32_t layers = 1, uint32_t base_layer = 0);

protected:
	VkImage image = nullptr;
	shared<Sampler> sampler = nullptr;
	unique<ImageView> view = nullptr;
	VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
	Allocation allocation{};
	Props props = {};
	uint32_t mip_levels = 1;

public:
	static shared<Image> get(std::string_view name) 
	{ 
		if (auto it = images.find(name); it != images.end()) 
			return it->second; 
		return nullptr;
	}
	static shared<Image> add(std::string_view name, const shared<Image>& image);
	static void destroy() { images.clear(); }

private:
	static inline std::unordered_map<std::string_view, shared<Image>> images{};
};