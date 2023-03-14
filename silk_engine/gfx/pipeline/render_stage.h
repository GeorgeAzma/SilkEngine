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
	RenderStage(const shared<RenderPass>& render_pass, const std::vector<Attachment>& framebuffer_attachments, const ivec2& viewport = ivec2(0));

	void onResize(const SwapChain& swap_chain);

	const shared<RenderPass>& getRenderPass() const { return render_pass; }
	shared<RenderPass>& getRenderPass() { return render_pass; }
	const shared<Framebuffer>& getFramebuffer() const { return framebuffer; }

private:
	shared<RenderPass> render_pass = nullptr;
	shared<Framebuffer> framebuffer = nullptr;
	std::vector<Attachment> framebuffer_attachments;
	ivec2 viewport = ivec2(0);
};