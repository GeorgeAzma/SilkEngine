#include "render_stage.h"
#include "gfx/window/window.h"
#include "gfx/graphics.h"
#include "gfx/window/swap_chain.h"

RenderStage::RenderStage(const shared<RenderPass>& render_pass, const std::vector<Attachment>& framebuffer_attachments, const glm::ivec2& viewport)
	: render_pass(render_pass), viewport(viewport), framebuffer_attachments(framebuffer_attachments)
{
}

void RenderStage::update()
{
	glm::ivec2 last_render_area = render_area;

	if (viewport == glm::ivec2(0))
		render_area = glm::ivec2(Window::getWidth(), Window::getHeight());
	else
		render_area = viewport;

	if (render_area != last_render_area)
	{
		framebuffer = makeShared<Framebuffer>(VkRenderPass(*render_pass), render_area.x, render_area.y);
		for (const auto& attachment : framebuffer_attachments)
		{
			if (attachment.swap_chain)
				framebuffer->addAttachment(Graphics::swap_chain->getImages());
			else
				framebuffer->addAttachment(attachment.format, attachment.samples);
		}
		framebuffer->build();
	}
}

