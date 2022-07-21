#include "image_view.h"
#include "image.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

ImageView::ImageView(VkImage image, ImageFormat format, uint32_t mip_levels, size_t layer_count, ImageViewType view_type)
{
	VkImageViewCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ci.image = image;
	VkImageViewType image_view_type;
	switch (view_type)
	{
	case ImageViewType::IMAGE1D:
		image_view_type = layer_count == 1 ? VK_IMAGE_VIEW_TYPE_1D : VK_IMAGE_VIEW_TYPE_1D_ARRAY;
		break;
	case ImageViewType::IMAGE2D:
		image_view_type = layer_count == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		break;
	case ImageViewType::IMAGE3D:
		SK_ASSERT(layer_count == 1, "Layer count({0}) can't be more than 1 for 3D images", layer_count);
		image_view_type = VK_IMAGE_VIEW_TYPE_3D;
		break;
	case ImageViewType::CUBEMAP:
		image_view_type = layer_count == 1 ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
		break;
	default:
		SK_ERROR("Invalid view type: {0}", view_type);
		break;
	}
	ci.viewType = image_view_type;
	ci.format = VkFormat(format);
	ci.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	ci.subresourceRange.aspectMask = ImageFormatEnum::getVulkanAspectFlags(format);
	ci.subresourceRange.baseMipLevel = 0;
	ci.subresourceRange.levelCount = mip_levels;
	ci.subresourceRange.baseArrayLayer = 0;
	ci.subresourceRange.layerCount = layer_count;
	image_view = Graphics::logical_device->createImageView(ci);
}

ImageView::~ImageView()
{
	Graphics::logical_device->destroyImageView(image_view);
}
