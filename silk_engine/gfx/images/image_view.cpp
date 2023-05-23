#include "image_view.h"
#include "image.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/devices/logical_device.h"

ImageView::ImageView(VkImage image, Image::Format format, size_t base_mip_level, uint32_t mip_levels, size_t base_layer, size_t layers, Image::Type image_type, VkComponentMapping components)
{
	VkImageViewCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ci.image = image;
	ci.viewType = VkImageViewType(image_type);
	ci.format = VkFormat(format);
	ci.components = components;
	ci.subresourceRange.aspectMask = Image::getFormatVulkanAspectFlags(format);
	ci.subresourceRange.baseMipLevel = base_mip_level;
	ci.subresourceRange.levelCount = mip_levels;
	ci.subresourceRange.baseArrayLayer = base_layer;
	ci.subresourceRange.layerCount = layers;
	image_view = RenderContext::getLogicalDevice().createImageView(ci);
}

ImageView::ImageView(const Image& image, size_t base_mip_level, uint32_t mip_levels, size_t base_layer, size_t layers, VkComponentMapping components)
	: ImageView(VkImage(image), image.getFormat(), base_mip_level, mip_levels ? mip_levels : (image.getMipLevels() - base_mip_level), base_layer, layers ? layers : (image.getLayers() - base_layer), image.getType(), components)
{
}

ImageView::~ImageView()
{
	RenderContext::getLogicalDevice().destroyImageView(image_view);
}
