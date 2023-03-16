#include "default_render_pipeline.h"
#include "gfx/window/window.h"
#include "gfx/window/swap_chain.h"
#include "gfx/subrender/mesh_subrender.h"
#include "gfx/subrender/particle_subrender.h"

DefaultRenderPipeline::DefaultRenderPipeline()
{
	const auto& swap_chain = Window::getActive().getSwapChain();
	shared<RenderPass> render_pass = shared<RenderPass>(new RenderPass({
		{
			{
				{ swap_chain.getDepthFormat(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, swap_chain.getSamples() },
				{ swap_chain.getFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, swap_chain.getSamples() }
			},
			{}
		}
	}));
	addRenderStage(RenderStage(render_pass));
}

void DefaultRenderPipeline::init()
{
	addSubrender<ParticleSubrender>({ 0, 0 });
	addSubrender<MeshSubrender>({ 0, 0 });
}

void DefaultRenderPipeline::update()
{
}
