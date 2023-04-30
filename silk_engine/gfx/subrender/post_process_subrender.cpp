#include "post_process_subrender.h"
#include "gfx/pipeline/graphics_pipeline.h"
#include "gfx/pipeline/render_pass.h"
#include "gfx/material.h"

PostProcessSubrender::PostProcessSubrender(RenderPass& render_pass, uint32_t subpass)
{
	auto graphics_pipeline = makeShared<GraphicsPipeline>();
	graphics_pipeline->setShader(makeShared<Shader>(std::vector<std::string_view>{ "screen", "post_process" }))
		.setRenderPass(render_pass)
		.setSubpass(subpass)
		.setDepthCompareOp(GraphicsPipeline::CompareOp::LESS_OR_EQUAL)
		.enableTag(GraphicsPipeline::EnableTag::BLEND, false)
		.build();
	GraphicsPipeline::add("Post Process", graphics_pipeline);
	material = makeShared<Material>(graphics_pipeline);
}

void PostProcessSubrender::render()
{
	//material->set("image", render_pass->subrenders);
}
