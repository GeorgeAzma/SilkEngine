#pragma once

#include "gfx/images/image2D.h"

class Framebuffer : NonCopyable
{
public:
	Framebuffer(vk::RenderPass render_pass, const std::vector<shared<Image2D>>& attachments, uint32_t width, uint32_t height);
	~Framebuffer();

	operator const vk::Framebuffer& () const { return framebuffer; }

private:
	vk::Framebuffer framebuffer = VK_NULL_HANDLE;
	uint32_t width = 0;
	uint32_t height = 0; 
	std::vector<shared<Image2D>> attachments;
};