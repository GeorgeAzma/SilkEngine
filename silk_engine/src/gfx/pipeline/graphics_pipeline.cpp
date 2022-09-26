#include "graphics_pipeline.h"
#include "pipeline_cache.h"
#include "gfx/graphics.h"
#include "gfx/window/swap_chain.h"
#include "gfx/devices/logical_device.h"
#include "gfx/buffers/command_buffer.h"
#include "gfx/renderer.h"

GraphicsPipeline::GraphicsPipeline()
{
	input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

	viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_info.viewportCount = 1;
	viewport_info.scissorCount = 1;

	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

	depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.attachmentCount = 1;

	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

	ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
}

GraphicsPipeline& GraphicsPipeline::setShader(const shared<Shader>& shader, const std::vector<Constant>& constants)
{
	IsetShader(shader, constants);
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setSamples(VkSampleCountFlagBits sample_count)
{
	multisampling.rasterizationSamples = sample_count;
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setRenderPass(VkRenderPass render_pass)
{
	this->render_pass = render_pass;
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setSubpass(uint32_t subpass)
{
	this->subpass = subpass;
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setStage(const PipelineStage& stage)
{
	render_pass = *Renderer::getRenderPass(stage.first);
	subpass = stage.second;
	return *this;
}

GraphicsPipeline& GraphicsPipeline::addDynamicState(VkDynamicState dynamic_state)
{
	dynamic_states.emplace_back(dynamic_state);
	return *this;
}

void GraphicsPipeline::build()
{
	color_blending.pAttachments = &color_blend_attachment;

	dynamic_state.dynamicStateCount = dynamic_states.size();
	dynamic_state.pDynamicStates = dynamic_states.data();

	ci.pVertexInputState = &vertex_input_info;
	ci.pInputAssemblyState = &input_assembly_info;
	ci.pRasterizationState = &rasterizer;
	ci.pMultisampleState = &multisampling;
	ci.pColorBlendState = &color_blending;
	ci.pDynamicState = &dynamic_state;
	ci.renderPass = render_pass;
	ci.subpass = subpass;
	ci.pDepthStencilState = &depth_stencil_info;

	create();
}

void GraphicsPipeline::bind()
{
	Graphics::submit([&](CommandBuffer& cb) { cb.bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline, layout); });
}

void GraphicsPipeline::IsetShader(const shared<Shader>& shader, const std::vector<Constant>& constants)
{
	input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_info.primitiveRestartEnable = VK_FALSE;

	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.depthBiasEnable = VK_FALSE;

	depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil_info.depthTestEnable = VK_TRUE;
	depth_stencil_info.depthWriteEnable = VK_TRUE;
	depth_stencil_info.stencilTestEnable = VK_FALSE;

	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.blendEnable = VK_TRUE;
	color_blending.logicOpEnable = VK_FALSE;
	multisampling.sampleShadingEnable = VK_FALSE;

	Pipeline::IsetShader(shader, constants);

	ci.stageCount = shader_stage_infos.size();
	ci.pStages = shader_stage_infos.data();

	vertex_input_info.vertexBindingDescriptionCount = this->shader->getBufferLayout().getBindingDescriptions().size();
	vertex_input_info.pVertexBindingDescriptions = this->shader->getBufferLayout().getBindingDescriptions().data();
	vertex_input_info.vertexAttributeDescriptionCount = this->shader->getBufferLayout().getAttributeDescriptions().size();
	vertex_input_info.pVertexAttributeDescriptions = this->shader->getBufferLayout().getAttributeDescriptions().data();

	const auto& params = shader->getParameters();
	for (size_t i = 0; i < params.enabled.size(); ++i)
	{
		const auto& value = params.enabled[i];

		if (!value.has_value())
			continue;

		switch ((EnableTag)i)
		{
		case EnableTag::BLEND:
			color_blend_attachment.blendEnable = *value;
			break;
		case EnableTag::DEPTH_TEST:
			depth_stencil_info.depthTestEnable = *value;
			break;
		case EnableTag::DEPTH_WRITE:
			depth_stencil_info.depthWriteEnable = *value;
			break;
		case EnableTag::PRIMITIVE_RESTART:
			input_assembly_info.primitiveRestartEnable = *value;
			break;
		case EnableTag::SAMPLE_SHADING:
			multisampling.sampleShadingEnable = *value;
			break;
		case EnableTag::STENCIL_TEST:
			depth_stencil_info.stencilTestEnable = *value;
			break;
		case EnableTag::RASTERIZER_DISCARD:
			rasterizer.rasterizerDiscardEnable = *value;
			break;
		case EnableTag::DEPTH_CLAMP:
			rasterizer.depthClampEnable = *value;
			break;
		case EnableTag::DEPTH_BIAS:
			rasterizer.depthBiasEnable = *value;
			break;
		case EnableTag::COLOR_BLEND_LOGIC_OP:
			color_blending.logicOpEnable = *value;
			break;
		}
	}

	if (params.cull_mode) rasterizer.cullMode = *params.cull_mode;
	if (params.line_width) rasterizer.lineWidth = *params.line_width;
	if (params.polygon_mode) rasterizer.polygonMode = *params.polygon_mode;
	if (params.front_face) rasterizer.frontFace = *params.front_face;
	if (params.depth_bias) rasterizer.depthBiasConstantFactor = *params.depth_bias;
	if (params.depth_slope) rasterizer.depthBiasSlopeFactor = *params.depth_slope;
	if (params.depth_clamp) rasterizer.depthBiasClamp = *params.depth_clamp;

	if (params.depth_compare_op) depth_stencil_info.depthCompareOp = *params.depth_compare_op;

	if (params.src_blend) color_blend_attachment.srcColorBlendFactor = *params.src_blend;
	if (params.dst_blend) color_blend_attachment.dstColorBlendFactor = *params.dst_blend;
	if (params.blend_op) color_blend_attachment.colorBlendOp = *params.blend_op;
	if (params.src_alpha_blend) color_blend_attachment.srcAlphaBlendFactor = *params.src_alpha_blend;
	if (params.dst_alpha_blend) color_blend_attachment.dstAlphaBlendFactor = *params.dst_alpha_blend;
	if (params.alpha_blend_op) color_blend_attachment.alphaBlendOp = *params.alpha_blend_op;
	if (params.color_write_mask) color_blend_attachment.colorWriteMask = *params.color_write_mask;
}

void GraphicsPipeline::create()
{
	layout = Graphics::logical_device->createPipelineLayout(pipeline_layout_info);
	ci.layout = layout;
	ci.pViewportState = &viewport_info;
	pipeline = Graphics::logical_device->createGraphicsPipeline(*Graphics::pipeline_cache, ci);
}