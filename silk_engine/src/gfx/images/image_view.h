#pragma once

enum class ImageViewType
{
	IMAGE1D,
	IMAGE2D,
	IMAGE3D,
	CUBEMAP,
};

class ImageView : NonCopyable
{
public:
	ImageView(VkImage image, VkFormat format, uint32_t mip_levels = 1, size_t layer_count = 1, ImageViewType view_type = ImageViewType::IMAGE2D);
	~ImageView();

	operator const VkImageView& () const { return image_view; }

private:
	VkImageView image_view;
};