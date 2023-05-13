#include "render_pipeline.h"
#include "gfx/render_context.h"
#include "render_pass.h"
#include "gfx/buffers/framebuffer.h"
#include "gfx/window/window.h"

void RenderPipeline::render()
{
	CommandBuffer& cb = RenderContext::getCommandBuffer();

	for (uint32_t render_pass_index = 0; render_pass_index < render_passes.size(); ++render_pass_index)
	{
		auto& render_pass = render_passes[render_pass_index];
		float width = render_pass->getFramebuffer()->getWidth();
		float height = render_pass->getFramebuffer()->getHeight();

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = height;
		viewport.width = width;
		viewport.height = -height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		cb.setViewport({ viewport });

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = { (uint32_t)width, (uint32_t)height };
		cb.setScissor({ scissor });

		render_pass->render();
	}
}

void RenderPipeline::resize()
{
	for (auto& render_pass : render_passes)
		render_pass->resize(Window::getActive().getSwapChain());
}
