#pragma once

enum class ImageFormat
{
    RED,
    RG,
    RGB,
    RGBA,
    BGRA,
    DEPTH_STENCIL,
    DEPTH,
    STENCIL
};

class ImageFormatEnum
{
public:
    static ImageFormat fromVulkanType(VkFormat vulkan_format);
    static ImageFormat fromChannelCount(uint8_t channels);
    static VkFormat toVulkanType(ImageFormat format);
    static VkImageAspectFlags getVulkanAspectFlags(ImageFormat format);
    static uint8_t getChannelCount(ImageFormat format);
    static size_t getSize(ImageFormat format);
    static bool hasStencil(ImageFormat format);
    static bool hasDepth(ImageFormat format);
};