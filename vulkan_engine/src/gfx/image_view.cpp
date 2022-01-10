#include "image_view.h"
#include "graphics.h"

ImageView::ImageView(VkImage image, VkFormat format, uint32_t mip_levels)
{
	VkImageViewCreateInfo view_info{};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = format;
	view_info.subresourceRange.aspectMask = EnumInfo::getAspectFlags(format);
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = mip_levels;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	Graphics::vulkanAssert(vkCreateImageView(*Graphics::logical_device, &view_info, nullptr, &image_view));
}

ImageView::~ImageView()
{
	vkDestroyImageView(*Graphics::logical_device, image_view, nullptr);
}
