#pragma once

class ImageView : NonCopyable
{
public:
	ImageView(VkImage image, VkFormat format);
	~ImageView();

	operator const VkImageView& () const { return image_view; }

private:
	VkImageView image_view;
};