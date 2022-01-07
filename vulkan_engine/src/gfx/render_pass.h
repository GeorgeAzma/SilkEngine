#pragma once

class RenderPass : NonCopyable
{
public:
	RenderPass();
	~RenderPass();

	void begin(VkFramebuffer framebuffer, VkCommandBuffer command_buffer);
	void end(VkCommandBuffer command_buffer);

	operator const VkRenderPass& () const { return render_pass; }

private:
	VkRenderPass render_pass;
};