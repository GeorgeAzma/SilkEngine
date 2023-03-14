#include "render_stage.h"
#include "gfx/window/window.h"
#include "gfx/graphics.h"
#include "gfx/window/swap_chain.h"

RenderStage::RenderStage(const shared<RenderPass>& render_pass, const std::vector<Attachment>& framebuffer_attachments, const ivec2& viewport)
	: render_pass(render_pass), viewport(viewport), framebuffer_attachments(framebuffer_attachments)
{
}

void RenderStage::onResize(const SwapChain& swap_chain)
{
	ivec2 render_area = viewport == ivec2(0) ? ivec2(swap_chain.getWidth(), swap_chain.getHeight()) : viewport;

	framebuffer = makeShared<Framebuffer>(swap_chain, VkRenderPass(*render_pass), render_area.x, render_area.y);
	for (const auto& attachment : framebuffer_attachments)
	{
		if (attachment.swap_chain)
			framebuffer->addSwapchainAttachments();
		else
			framebuffer->addAttachment(attachment.format, attachment.samples);
	}
	framebuffer->build();
}

