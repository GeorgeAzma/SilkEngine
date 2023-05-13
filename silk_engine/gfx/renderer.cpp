#include "renderer.h"
#include "fence.h"
#include "semaphore.h"
#include "window/window.h"
#include "window/surface.h"
#include "window/swap_chain.h"
#include "debug_renderer.h"
#include "scene/scene.h"
#include "buffers/command_buffer.h"
#include "render_context.h"
#include "pipeline/render_pass.h"
#include "pipeline/subrenders/particle_subrender.h"
#include "pipeline/subrenders/mesh_subrender.h"
#include "buffers/framebuffer.h"

void Renderer::init()
{
	previous_frame_finished = new Fence(true);
	swap_chain_image_available = new Semaphore();
	render_finished = new Semaphore();

	shared<RenderPass> render_pass = shared<RenderPass>(new RenderPass({
		   {
			   {
				   { Image::Format(RenderContext::getPhysicalDevice().getDepthFormat()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, RenderContext::getPhysicalDevice().getMaxSampleCount() },
				   { Image::Format(Window::getActive().getSurface().getFormat().format), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, RenderContext::getPhysicalDevice().getMaxSampleCount() }
			   }, {}
		   }
		}));
	render_pass->addSubrender<ParticleSubrender>(0);
	render_pass->addSubrender<MeshSubrender>(0);
	render_passes.emplace_back(render_pass);

	for (auto& render_pass : render_passes)
		render_pass->resize(Window::getActive().getSwapChain());

	DebugRenderer::init();
}

void Renderer::destroy()
{
	DebugRenderer::destroy();
	delete previous_frame_finished;
	delete swap_chain_image_available;
	delete render_finished;
	render_passes.clear();
}

void Renderer::wait()
{
	previous_frame_finished->wait();
	previous_frame_finished->reset();
	DebugRenderer::reset();
}

void Renderer::render()
{
	if (Scene::getActive())
		DebugRenderer::update(Scene::getActive()->getMainCamera());

	if (!Window::getActive().getSwapChain().acquireNextImage(*Renderer::swap_chain_image_available))
	{
		Window::getActive().recreate();
		for (auto& render_pass : render_passes)
			render_pass->resize(Window::getActive().getSwapChain());
	}

	CommandBuffer& cb = RenderContext::getCommandBuffer();

	for (uint32_t render_pass_index = 0; render_pass_index < render_passes.size(); ++render_pass_index)
	{
		auto& render_pass = render_passes[render_pass_index];
		uint32_t width = render_pass->getFramebuffer()->getWidth();
		uint32_t height = render_pass->getFramebuffer()->getHeight();

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = height;
		viewport.width = float(width);
		viewport.height = -float(height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		cb.setViewport({ viewport });

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = { width, height };
		cb.setScissor({ scissor });

		render_pass->render();
	}
	RenderContext::submit(previous_frame_finished, { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }, { *swap_chain_image_available }, { *render_finished });

	if (!Window::getActive().getSwapChain().present(*render_finished))
	{
		Window::getActive().recreate();
		for (auto& render_pass : render_passes)
			render_pass->resize(Window::getActive().getSwapChain());
	}

	RenderContext::update();
}