#include "graphics_pipeline.h"
#include "pipeline_cache.h"
#include "gfx/render_context.h"
#include "gfx/window/swap_chain.h"
#include "gfx/devices/logical_device.h"
#include "gfx/buffers/command_buffer.h"
#include "gfx/pipeline/render_pass.h"

GraphicsPipeline::GraphicsPipeline()
{
	input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_info.primitiveRestartEnable = VK_FALSE;

	viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_info.viewportCount = 1;
	viewport_info.scissorCount = 1;

	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;

	depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil_info.depthTestEnable = VK_TRUE;
	depth_stencil_info.depthWriteEnable = VK_TRUE;
	depth_stencil_info.stencilTestEnable = VK_FALSE;

	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.attachmentCount = 1;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.pAttachments = &color_blend_attachment;

	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.blendEnable = VK_TRUE;

	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
}

void GraphicsPipeline::bind()
{
	RenderContext::record([&](CommandBuffer& cb) { cb.bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline, layout); });
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

GraphicsPipeline& GraphicsPipeline::addDynamicState(VkDynamicState dynamic_state)
{
	dynamic_states.emplace_back(dynamic_state);
	return *this;
}

GraphicsPipeline& GraphicsPipeline::enableTag(EnableTag tag, bool enable)
{
	switch (tag)
	{
	case EnableTag::BLEND:
		color_blend_attachment.blendEnable = enable;
		break;
	case EnableTag::DEPTH_TEST:
		depth_stencil_info.depthTestEnable = enable;
		break;
	case EnableTag::DEPTH_WRITE:
		depth_stencil_info.depthWriteEnable = enable;
		break;
	case EnableTag::PRIMITIVE_RESTART:
		input_assembly_info.primitiveRestartEnable = enable;
		break;
	case EnableTag::SAMPLE_SHADING:
		multisampling.sampleShadingEnable = enable;
		break;
	case EnableTag::STENCIL_TEST:
		depth_stencil_info.stencilTestEnable = enable;
		break;
	case EnableTag::RASTERIZER_DISCARD:
		rasterizer.rasterizerDiscardEnable = enable;
		break;
	case EnableTag::DEPTH_CLAMP:
		rasterizer.depthClampEnable = enable;
		break;
	case EnableTag::DEPTH_BIAS:
		rasterizer.depthBiasEnable = enable;
		break;
	case EnableTag::COLOR_BLEND_LOGIC_OP:
		color_blending.logicOpEnable = enable;
		break;
	}
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setCullMode(CullMode cull_mode)
{
	rasterizer.cullMode = VkCullModeFlags(cull_mode);
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setLineWidth(float width)
{
	rasterizer.lineWidth = width;
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setPolygonMode(PolygonMode polygon_mode)
{
	rasterizer.polygonMode = VkPolygonMode(polygon_mode);
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setFrontFace(FrontFace front_face)
{
	rasterizer.frontFace = VkFrontFace(front_face);
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setDepthBias(float constant, float slope, float clamp)
{
	rasterizer.depthBiasConstantFactor = constant;
	rasterizer.depthBiasSlopeFactor = slope;
	rasterizer.depthBiasClamp = clamp;
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setDepthCompareOp(CompareOp compare_op)
{
	depth_stencil_info.depthCompareOp = VkCompareOp(compare_op);
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setBlend(BlendFactor src_color, BlendFactor dst_color, BlendFactor src_alpha, BlendFactor dst_alpha)
{
	color_blend_attachment.srcColorBlendFactor = VkBlendFactor(src_color);
	color_blend_attachment.dstColorBlendFactor = VkBlendFactor(dst_color);
	color_blend_attachment.srcAlphaBlendFactor = VkBlendFactor(src_alpha);
	color_blend_attachment.dstAlphaBlendFactor = VkBlendFactor(dst_alpha);
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setColorBlendOp(BlendOp blend_op)
{
	color_blend_attachment.colorBlendOp = VkBlendOp(blend_op);
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setAlphaBlendOp(BlendOp blend_op)
{
	color_blend_attachment.alphaBlendOp = VkBlendOp(blend_op);
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setColorWriteMask(ColorComponent mask)
{
	color_blend_attachment.colorWriteMask = VkColorComponentFlags(mask);
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setShader(const shared<Shader>& shader, const std::vector<Pipeline::Constant>& constants)
{
	SK_VERIFY(!this->shader, "Must not call setShader more than once");
	Pipeline::setShader(shader, constants);
	return *this;
}

void GraphicsPipeline::build()
{
	dynamic_state.dynamicStateCount = dynamic_states.size();
	dynamic_state.pDynamicStates = dynamic_states.data();

	ci.pViewportState = &viewport_info;
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

void GraphicsPipeline::create()
{
	ci.layout = layout;

	ci.stageCount = shader_stage_infos.size();
	ci.pStages = shader_stage_infos.data();

	// TODO: Support per pipeline buffer layout, when you need it (probably almost never)
	vertex_input_info.vertexBindingDescriptionCount = shader->getReflectionData().vertex_input_binding_descriptions.size();
	vertex_input_info.pVertexBindingDescriptions = shader->getReflectionData().vertex_input_binding_descriptions.data();
	vertex_input_info.vertexAttributeDescriptionCount = shader->getReflectionData().vertex_input_attribute_descriptions.size();
	vertex_input_info.pVertexAttributeDescriptions = shader->getReflectionData().vertex_input_attribute_descriptions.data();
	ci.pViewportState = &viewport_info;
	
	pipeline = RenderContext::getLogicalDevice().createGraphicsPipeline(RenderContext::getPipelineCache(), ci);
}
