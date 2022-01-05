#pragma once

class RenderPass
{
public:
	RenderPass();
	~RenderPass();

private:
	VkRenderPass render_pass;
};