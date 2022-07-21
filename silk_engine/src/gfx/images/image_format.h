#pragma once

enum class ImageFormat
{
    RED = VK_FORMAT_R8_UNORM,
    RG = VK_FORMAT_R8G8_UNORM,
    RGB = VK_FORMAT_R8G8B8_UNORM,
    RGBA = VK_FORMAT_R8G8B8A8_UNORM,
    BGRA = VK_FORMAT_B8G8R8A8_UNORM,
    DEPTH_STENCIL = VK_FORMAT_D24_UNORM_S8_UINT,
    DEPTH = VK_FORMAT_D32_SFLOAT,
    STENCIL = VK_FORMAT_S8_UINT
};

class ImageFormatEnum
{
public:
    static ImageFormat fromChannelCount(uint8_t channels);
    static VkImageAspectFlags getVulkanAspectFlags(ImageFormat format);
    static uint8_t getChannelCount(ImageFormat format);
    static size_t getSize(ImageFormat format);
    static bool hasStencil(ImageFormat format);
    static bool hasDepth(ImageFormat format);
};