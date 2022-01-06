#pragma once

class Framebuffer : NonCopyable
{
public:
	Framebuffer(VkRenderPass render_pass, const std::vector<VkImageView>& attachments);
	~Framebuffer();

	operator const VkFramebuffer& () const { return framebuffer; }

private:
	VkFramebuffer framebuffer;
};