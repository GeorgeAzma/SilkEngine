#pragma once

class Framebuffer : NonCopyable
{
public:
	Framebuffer(VkRenderPass render_pass, const std::vector<VkImageView>& attachments, uint32_t width, uint32_t height);
	~Framebuffer();

	operator const VkFramebuffer& () const { return framebuffer; }

private:
	VkFramebuffer framebuffer;
};