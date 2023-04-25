#include "renderer.h"
#include "fence.h"
#include "semaphore.h"
#include "pipeline/default_render_pipeline.h"
#include "window/window.h"
#include "window/swap_chain.h"
#include "debug_renderer.h"
#include "scene/scene.h"
#include "buffers/command_buffer.h"
#include "render_context.h"

void Renderer::init()
{
	previous_frame_finished = new Fence(true);
	swap_chain_image_available = new Semaphore();
	render_finished = new Semaphore();

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
	delete swap_chain_image_available;
	delete render_finished;
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

void Renderer::render()
{
	DebugRenderer::update(Scene::getActive().get() ? Scene::getActive()->getMainCamera() : nullptr);

	if (!Window::getActive().getSwapChain().acquireNextImage(*Renderer::swap_chain_image_available))
	{
		SK_ERROR("Unexpected, window should already be updated");
		Window::getActive().recreate();
		for (auto& render_pass : render_pipeline->getRenderPasses())
			render_pass->onResize(Window::getActive().getSwapChain());
	}

	render_pipeline->render();

	CommandBuffer::SubmitInfo submit_info;
	submit_info.wait_stages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.wait_semaphores = { *swap_chain_image_available };
	submit_info.signal_semaphores = { *render_finished };
	submit_info.fence = previous_frame_finished;
	RenderContext::execute(submit_info);

	if (!Window::getActive().getSwapChain().present(*render_finished))
	{
		Window::getActive().recreate();
		for (auto& render_pass : render_pipeline->getRenderPasses())
			render_pass->onResize(Window::getActive().getSwapChain());
	}

	RenderContext::update();
}