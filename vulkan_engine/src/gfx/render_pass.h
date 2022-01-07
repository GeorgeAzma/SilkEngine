#pragma once

class RenderPass : NonCopyable
{
public:
	RenderPass();
	~RenderPass();

	void begin(VkFramebuffer framebuffer);
	void end();

	operator const VkRenderPass& () const { return render_pass; }

private:
	VkRenderPass render_pass;
};