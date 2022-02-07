#pragma once

#include "gfx/images/image.h"

class Framebuffer : NonCopyable
{
public:
	Framebuffer(VkRenderPass render_pass, const std::vector<shared<Image>>& attachments, uint32_t width, uint32_t height);
	~Framebuffer();

	operator const VkFramebuffer& () const { return framebuffer; }

private:
	VkFramebuffer framebuffer = VK_NULL_HANDLE;
	uint32_t width = 0;
	uint32_t height = 0;
};