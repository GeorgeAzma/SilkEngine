#pragma once

class Framebuffer
{
public:
	Framebuffer(VkRenderPass render_pass, const std::vector<VkImageView>& attachments);
	~Framebuffer();

	operator const VkFramebuffer& () const { return framebuffer; }

private:
	VkFramebuffer framebuffer;
};