#include "render_stage.h"
#include "gfx/window/window.h"
#include "gfx/render_context.h"
#include "gfx/window/swap_chain.h"

RenderStage::RenderStage(const shared<RenderPass>& render_pass, const ivec2& viewport)
	: render_pass(render_pass), viewport(viewport)
{
}

void RenderStage::onResize(const SwapChain& swap_chain)
{
	ivec2 render_area = viewport == ivec2(0) ? ivec2(swap_chain.getWidth(), swap_chain.getHeight()) : viewport;

	framebuffer = makeShared<Framebuffer>(swap_chain, VkRenderPass(*render_pass), render_area.x, render_area.y);
	for (const auto& render_target : render_pass->getRenderTargetInfo())
	{
		if (render_target.presented)
			framebuffer->addSwapchainAttachments();
		else
			framebuffer->addAttachment(render_target.format, render_target.samples);
	}
	framebuffer->build();
}

