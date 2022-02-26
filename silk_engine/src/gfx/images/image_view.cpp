#include "image_view.h"
#include "image.h"
#include "gfx/graphics.h"
#include "gfx/enums.h"
#include "gfx/devices/logical_device.h"

ImageView::ImageView(vk::Image image, vk::Format format, uint32_t mip_levels, size_t layer_count, ImageViewType view_type)
{
	vk::ImageViewCreateInfo ci{};
	ci.image = image;
	vk::ImageViewType image_view_type;
	switch (view_type)
	{
		using enum vk::ImageViewType;
	case ImageViewType::IMAGE1D:
		image_view_type = layer_count == 1 ? e1D : e1DArray;
		break;
	case ImageViewType::IMAGE2D:
		image_view_type = layer_count == 1 ? e2D : e2DArray;
		break;
	case ImageViewType::IMAGE3D:
		SK_ASSERT(layer_count == 1, "Layer count({0}) can't be more than 1 for 3D images", layer_count);
		image_view_type = e3D;
		break;
	case ImageViewType::CUBEMAP:
		image_view_type = layer_count == 1 ? eCube : eCubeArray;
		break;
	default:
		SK_ERROR("Invalid view type: {0}", view_type);
		break;
	}
	ci.viewType = image_view_type;
	ci.format = format;
	ci.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	ci.subresourceRange.aspectMask = Image::getAspectFlags(format);
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
