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
	ImageView(vk::Image image, vk::Format format, uint32_t mip_levels = 1, size_t layer_count = 1, ImageViewType view_type = ImageViewType::IMAGE2D);
	~ImageView();

	operator const vk::ImageView& () const { return image_view; }

private:
	vk::ImageView image_view;
};