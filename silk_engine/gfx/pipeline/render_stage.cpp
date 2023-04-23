#include "render_stage.h"
#include "gfx/window/window.h"
#include "gfx/render_context.h"
#include "gfx/window/swap_chain.h"

RenderStage::RenderStage(RenderPass& render_pass, const ivec2& viewport)
	: render_pass(render_pass), viewport(viewport)
{
}

void RenderStage::onResize(const SwapChain& swap_chain)
{
	framebuffer = makeShared<Framebuffer>(swap_chain, render_pass, viewport.x ? viewport.x : swap_chain.getWidth(), viewport.y ? viewport.y : swap_chain.getHeight());
}

