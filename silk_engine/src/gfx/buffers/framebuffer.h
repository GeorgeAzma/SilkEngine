#pragma once

#include "gfx/images/image2D.h"
#include "core/event.h"

class Framebuffer : NonCopyable
{
public:
	Framebuffer(VkRenderPass render_pass, uint32_t width = 0, uint32_t height = 0);
	~Framebuffer();

	Framebuffer& addAttachment(ImageFormat format = ImageFormat::BGRA, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	//For adding swap chain image attachment ONLY
	Framebuffer& addAttachment(const shared<Image2D>& image);
	void build();

	const std::vector<shared<Image2D>>& getAttachments() const { return attachments; }
	operator const VkFramebuffer& () const { return framebuffer; }

private:
	VkFramebuffer framebuffer = VK_NULL_HANDLE;
	VkRenderPass render_pass = VK_NULL_HANDLE;
	uint32_t width = 0;
	uint32_t height = 0; 
	std::vector<shared<Image2D>> attachments;
};