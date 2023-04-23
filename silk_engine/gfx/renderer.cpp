#include "renderer.h"
#include "gfx/render_context.h"
#include "debug_renderer.h"
#include "gfx/window/window.h"
#include "gfx/window/swap_chain.h"
#include "scene/meshes/line_mesh.h"
#include "scene/meshes/bezier_mesh.h"
#include "scene/meshes/triangle_mesh.h"
#include "scene/meshes/text_mesh.h"
#include "gfx/devices/logical_device.h"
#include "gfx/pipeline/default_render_pipeline.h"
#include "pipeline/pipeline_stage.h"
#include "scene/camera/camera.h"
#include "buffers/command_buffer.h"
#include "gfx/fence.h"
#include "gfx/pipeline/render_pass.h"
#include "gfx/buffers/framebuffer.h"

unique<RenderPipeline> Renderer::render_pipeline = nullptr;
Fence* Renderer::previous_frame_finished = nullptr;
VkSemaphore Renderer::swap_chain_image_available = nullptr;
VkSemaphore Renderer::render_finished = nullptr;

void Renderer::init()
{
	previous_frame_finished = new Fence(true);
	swap_chain_image_available = RenderContext::getLogicalDevice().createSemaphore();
	render_finished = RenderContext::getLogicalDevice().createSemaphore();

	setRenderPipeline<DefaultRenderPipeline>();
	render_pipeline->init();

	for (auto& render_pass : render_pipeline->getRenderPasses())
		render_pass->onResize(Window::getActive().getSwapChain());
	
	DebugRenderer::init();
}

void Renderer::destroy()
{
	DebugRenderer::destroy();
	delete previous_frame_finished;
	RenderContext::getLogicalDevice().destroySemaphore(swap_chain_image_available);
	RenderContext::getLogicalDevice().destroySemaphore(render_finished);
	render_pipeline = nullptr;
}

void Renderer::wait()
{
	previous_frame_finished->wait();
	previous_frame_finished->reset();
	DebugRenderer::reset();
}

const RenderPass& Renderer::getRenderPass(uint32_t index)
{
	return *render_pipeline->getRenderPasses()[index];
}

void Renderer::render(Camera* camera)
{
	DebugRenderer::update(camera);

	if (!Window::getActive().getSwapChain().acquireNextImage(Renderer::swap_chain_image_available))
	{
		SK_ERROR("Unexpected, window should already be updated");
		Window::getActive().recreate();
		for (auto& render_pass : render_pipeline->getRenderPasses())
			render_pass->onResize(Window::getActive().getSwapChain());
	}

	render_pipeline->update(); 

	RenderContext::submit([&](CommandBuffer& cb)
		{
			PipelineStage stage{};
			for (auto& render_pass : render_pipeline->getRenderPasses())
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
					stage.subpass = i;
					render_pipeline->renderStage(stage);
					if (i < render_pass->getSubpassCount() - 1)
						render_pass->nextSubpass();
				}
				render_pass->end();
				++stage.render_pass;
			}
		});

	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	CommandBuffer::SubmitInfo submit_info{};
	submit_info.wait_stages = &wait_stage;
	submit_info.wait_semaphores = { swap_chain_image_available };
	submit_info.signal_semaphores = { render_finished };
	submit_info.fence = previous_frame_finished;
	RenderContext::execute(submit_info);
	if (!Window::getActive().getSwapChain().present(render_finished))
	{
		Window::getActive().recreate();
		for (auto& render_pass : render_pipeline->getRenderPasses())
			render_pass->onResize(Window::getActive().getSwapChain());
	}

	RenderContext::update();
}