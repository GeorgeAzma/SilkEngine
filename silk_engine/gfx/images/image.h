#pragma once

#include "silk_engine/gfx/allocators/allocation.h"
#include "sampler.h"

class ImageView;
template <typename T>
class RawImage;

class Image : NoCopy
{
public:
	struct Props
	{
		uint32_t width = 1;
		uint32_t height = 1;
		uint32_t depth = 1;
		uint32_t layers = 1;
		Format format = Format::BGRA;
		ImageUsage usage = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;
		Allocation::Props allocation_props{};
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		Sampler::Props sampler_props{};
		bool linear_tiling = false;
		ImageViewType view_type = ImageViewType::_2D;

		uint32_t getChannels() const { return getFormatChannels(format); }
		size_t getPixelCount() const { return width * height * depth * layers; }
		size_t getSize() const { return getPixelCount() * getFormatSize(format); }
	};

public:
	Image(const Props& props);
	Image(const RawImage<uint8_t>& raw_image, const Props& props = {});
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
	ImageUsage getUsage() const { return props.usage; }
	VkSampleCountFlagBits getSamples() const { return props.samples; }
	ImageType getType() const { return getImageViewTypeImageType(props.view_type); }
	ImageViewType getViewType() const { return props.view_type; }

	bool isSampled() const { return bool(props.usage & ImageUsage::SAMPLED); }
	uint32_t getChannels() const { return props.getChannels(); }
	size_t getPixelCount() const { return props.getPixelCount(); }
	float getAspectRatio() const { return float(getWidth()) / getHeight(); }
	size_t getSize() const { return props.getSize(); }
	uint32_t getMipLevels() const { return mip_levels; }
	ImageAspect getAspect() const { return getFormatImageAspect(props.format); }

	const VkImageLayout& getLayout(uint32_t base_mip_level = 0, uint32_t base_layer = 0) const { return layouts[base_layer * mip_levels + base_mip_level]; }
	const VkImageView& getView() const;
	const VkSampler& getSampler() const { return *sampler; }
	VkDescriptorImageInfo getDescriptorInfo() const;
	const Allocation& getAllocation() const { return allocation; }
	operator const VkImage& () const { return image; }

	void setLayout(VkImageLayout layout, uint32_t mip_levels = 0, uint32_t base_mip_level = 0, uint32_t layers = 0, uint32_t base_layer = 0) // Use sparingly
	{ 
		if (!mip_levels) mip_levels = this->mip_levels - base_mip_level;
		if (!layers) layers = props.layers - base_layer;
		for (uint32_t layer = 0; layer < layers; ++layer)
			for (uint32_t mip = 0; mip < mip_levels; ++mip)
				getLayout(mip + base_mip_level, layer + base_layer) = layout;
	}
	void setData(const void* data, uint32_t base_layer = 0, uint32_t layers = 1);
	void getData(void* data, uint32_t base_layer = 0, uint32_t layers = 1);
	bool copyToImage(Image& destination);
	void copyFromBuffer(VkBuffer buffer, uint32_t base_layer = 0, uint32_t layers = 1);
	void copyToBuffer(VkBuffer buffer, uint32_t base_layer = 0, uint32_t layers = 1);
	void transitionLayout(VkImageLayout new_layout, VkDependencyFlags dependency = 0, uint32_t mip_levels = 0, uint32_t base_mip_level = 0, uint32_t layers = 0, uint32_t base_layer = 0);
	bool isFeatureSupported(VkFormatFeatureFlags feature) const;
	
private:
	VkImageLayout& getLayout(uint32_t base_mip_level = 0, uint32_t base_layer = 0) { return layouts[base_layer * mip_levels + base_mip_level]; }

	void create();
	void generateMipmaps();

private:
	static void insertMemoryBarrier(const VkImage& image, VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkImageLayout old_layout, VkImageLayout new_layout, PipelineStage source_stage, PipelineStage destination_stage, VkDependencyFlags depencency = 0, ImageAspect aspect = ImageAspect::COLOR, uint32_t mip_levels = 1, uint32_t base_mip_level = 0, uint32_t layers = 1, uint32_t base_layer = 0);
	static void insertMemoryBarrier(const VkImage& image, VkImageLayout old_layout, VkImageLayout new_layout, VkDependencyFlags dependency = 0, ImageAspect aspect = ImageAspect::COLOR, uint32_t mip_levels = 1, uint32_t base_mip_level = 0, uint32_t layers = 1, uint32_t base_layer = 0);
	static uint32_t calculateMipLevels(uint32_t width, uint32_t height = 1, uint32_t depth = 1)
	{
		return floor(std::log2(std::max({ width, height, depth }))) + 1;
	}

private:
	VkImage image = nullptr;
	shared<Sampler> sampler = nullptr;
	shared<ImageView> view = nullptr;
	std::vector<VkImageLayout> layouts = {}; // layout / mip / layer
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