#include "render_pipeline.h"
#include "gfx/render_context.h"
#include "render_pass.h"
#include "gfx/buffers/framebuffer.h"
#include "gfx/window/window.h"

void RenderPipeline::render()
{
	RenderContext::record([&](CommandBuffer& cb)
		{
			PipelineStage pipeline_stage{};
			for (auto& render_pass : getRenderPasses())
			{
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

				render_pass->begin();
				for (size_t i = 0; i < render_pass->getSubpassCount(); ++i)
				{
					pipeline_stage.subpass = i;

					for (const auto& [tid, stage, subrender] : subrenders)
						if (stage == pipeline_stage && subrender->enabled)
							subrender->render();

					if (i < render_pass->getSubpassCount() - 1)
						render_pass->nextSubpass();
				}
				render_pass->end();
				++pipeline_stage.render_pass;
			}
		});
}

void RenderPipeline::resize()
{
	for (auto& render_pass : render_passes)
		render_pass->resize(Window::getActive().getSwapChain());
}
