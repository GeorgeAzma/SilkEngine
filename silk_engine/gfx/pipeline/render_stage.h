#pragma once

#include "render_pass.h"
#include "gfx/buffers/framebuffer.h"

struct Attachment
{
	Image::Format format = Image::Format::BGRA;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	bool swap_chain = false;
};

class RenderStage
{
public:
	RenderStage(RenderPass& render_pass, const ivec2& viewport = ivec2(0));

	void onResize(const SwapChain& swap_chain);

	const RenderPass& getRenderPass() const { return render_pass; }
	RenderPass& getRenderPass() { return render_pass; }
	const shared<Framebuffer>& getFramebuffer() const { return framebuffer; }

private:
	RenderPass& render_pass;
	shared<Framebuffer> framebuffer = nullptr;
	ivec2 viewport = ivec2(0);
};