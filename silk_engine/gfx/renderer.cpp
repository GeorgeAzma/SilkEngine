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
	render_pipeline->resize();
	
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
	return *render_pipeline->render_passes[index];
}

void Renderer::render()
{
	if (Scene::getActive())
		DebugRenderer::update(Scene::getActive()->getMainCamera());

	if (!Window::getActive().getSwapChain().acquireNextImage(*Renderer::swap_chain_image_available))
	{
		Window::getActive().recreate();
		render_pipeline->resize();
	}

	render_pipeline->render();
	RenderContext::submit(previous_frame_finished, { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }, { *swap_chain_image_available }, { *render_finished });

	if (!Window::getActive().getSwapChain().present(*render_finished))
	{
		Window::getActive().recreate();
		render_pipeline->resize();
	}

	RenderContext::update();
}