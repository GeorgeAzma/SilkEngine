#pragma once

class Framebuffer
{
public:
	Framebuffer(VkRenderPass render_pass, const std::vector<VkImageView>& attachments);
	~Framebuffer();

private:
	VkFramebuffer framebuffer;
	VkDevice logical_device;
};