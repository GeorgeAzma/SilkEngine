#pragma once

class ImageView : NonCopyable
{
public:
	ImageView(VkImage image, VkFormat format, uint32_t mip_levels = 1);
	~ImageView();

	operator const VkImageView& () const { return image_view; }

private:
	VkImageView image_view;
};