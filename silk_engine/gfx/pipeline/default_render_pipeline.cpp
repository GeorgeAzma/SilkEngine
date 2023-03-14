#include "default_render_pipeline.h"
#include "gfx/window/window.h"
#include "gfx/window/swap_chain.h"
#include "gfx/subrender/mesh_subrender.h"
#include "gfx/subrender/particle_subrender.h"

DefaultRenderPipeline::DefaultRenderPipeline()
{
	const auto& swap_chain = Window::getActive().getSwapChain();
	shared<RenderPass> render_pass = makeShared<RenderPass>();
	render_pass->addSubpass();
	render_pass->addAttachment({ swap_chain.getFormat(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, swap_chain.getSamples()});
	render_pass->addAttachment({ swap_chain.getDepthFormat(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, swap_chain.getSamples() });
	render_pass->addAttachment({ swap_chain.getFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR });
	render_pass->build();
	std::vector<Attachment> attachments 
	{ 
		Attachment(swap_chain.getFormat(), swap_chain.getSamples()),
		Attachment(swap_chain.getDepthFormat(), swap_chain.getSamples()),
		Attachment(swap_chain.getFormat(), VK_SAMPLE_COUNT_1_BIT, true)
	};
	addRenderStage(RenderStage(render_pass, attachments));
}

void DefaultRenderPipeline::init()
{
	addSubrender<ParticleSubrender>({ 0, 0 });
	addSubrender<MeshSubrender>({ 0, 0 });
}

void DefaultRenderPipeline::update()
{
}
