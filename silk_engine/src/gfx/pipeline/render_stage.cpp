#include "render_stage.h"
#include "gfx/window/window.h"
#include "gfx/graphics.h"
#include "gfx/window/swap_chain.h"

RenderStage::RenderStage(const SwapChain& swap_chain, const shared<RenderPass>& render_pass, const std::vector<Attachment>& framebuffer_attachments, const ivec2& viewport)
	: swap_chain(swap_chain), render_pass(render_pass), viewport(viewport), framebuffer_attachments(framebuffer_attachments)
{
}

void RenderStage::update()
{
	ivec2 last_render_area = render_area;

	if (viewport == ivec2(0))
		render_area = ivec2(Window::getActive().getWidth(), Window::getActive().getHeight());
	else
		render_area = viewport;

	if (render_area != last_render_area)
	{
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
}

