#pragma once

class RenderPass : NonCopyable
{
public:
	RenderPass();
	~RenderPass();

	operator const VkRenderPass& () const { return render_pass; }

private:
	VkRenderPass render_pass;
};