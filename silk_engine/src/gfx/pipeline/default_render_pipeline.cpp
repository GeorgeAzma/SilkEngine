#include "default_render_pipeline.h"
#include "gfx/graphics.h"
#include "gfx/window/swap_chain.h"
#include "gfx/subrender/mesh_subrender.h"
#include "gfx/subrender/particle_subrender.h"

DefaultRenderPipeline::DefaultRenderPipeline()
{
	shared<RenderPass> render_pass = makeShared<RenderPass>();
	render_pass->addSubpass();
	render_pass->addAttachment({ Graphics::swap_chain->getFormat(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, Graphics::swap_chain->getSamples() });
	render_pass->addAttachment({ Graphics::swap_chain->getDepthFormat(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, Graphics::swap_chain->getSamples() });
	render_pass->addAttachment({ Graphics::swap_chain->getFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR });
	render_pass->build();
	std::vector<Attachment> attachments 
	{ 
		Attachment(Graphics::swap_chain->getFormat(), Graphics::swap_chain->getSamples()),
		Attachment(Graphics::swap_chain->getDepthFormat(), Graphics::swap_chain->getSamples()),
		Attachment(Graphics::swap_chain->getFormat(), VK_SAMPLE_COUNT_1_BIT, true)
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
