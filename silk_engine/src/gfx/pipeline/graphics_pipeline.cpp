#include "graphics_pipeline.h"
#include "gfx/graphics.h"
#include "gfx/window/swap_chain.h"
#include "gfx/devices/logical_device.h"

GraphicsPipeline& GraphicsPipeline::setShader(shared<Shader> shader)
{
	this->shader = shader;
	shader_stage_infos.clear();
	for (const auto& pipeline_shader_stage_info : shader->getPipelineShaderStageInfos())
		shader_stage_infos.emplace_back(pipeline_shader_stage_info);
	create_info.stageCount = shader_stage_infos.size();
	create_info.pStages = shader_stage_infos.data();

	push_constant_ranges = shader->getPushConstantRanges();
	if(shader->getDescriptorSet().get())
		descriptor_set_layouts = { shader->getDescriptorSet()->getLayout() }; //TODO: Support more than 1 descriptor sets

	return *this;
}

GraphicsPipeline& GraphicsPipeline::setVertexLayout(const BufferLayout& layout)
{
	this->layout = layout;
	vertex_input_info.vertexBindingDescriptionCount = this->layout.getBindingDescriptions().size();
	vertex_input_info.pVertexBindingDescriptions = this->layout.getBindingDescriptions().data();
	vertex_input_info.vertexAttributeDescriptionCount = this->layout.getAttributeDescriptions().size();
	vertex_input_info.pVertexAttributeDescriptions = this->layout.getAttributeDescriptions().data();
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setSampleCount(VkSampleCountFlagBits sample_count)
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

GraphicsPipeline& GraphicsPipeline::enable(EnableTag tag)
{
	switch (tag)
	{
	case EnableTag::COLOR_BLENDING:
		color_blend_attachment.blendEnable = VK_TRUE;
		break;
	case EnableTag::DEPTH_TEST:
		depth_stencil_info.depthTestEnable = VK_TRUE;
		break;
	case EnableTag::DEPTH_WRITE:
		depth_stencil_info.depthWriteEnable = VK_TRUE;
		break;
	case EnableTag::PRIMITIVE_RESTART:
		input_assembly_info.primitiveRestartEnable = VK_TRUE;
		break;
	case EnableTag::SAMPLE_SHADING:
		multisampling.sampleShadingEnable = VK_TRUE;
		break;
	case EnableTag::STENCIL_TEST:
		depth_stencil_info.stencilTestEnable = VK_TRUE;
		break;
	case EnableTag::RASTERIZER_DISCARD:
		rasterizer.rasterizerDiscardEnable = VK_TRUE;
		break;
	case EnableTag::DEPTH_CLAMP:
		rasterizer.depthClampEnable = VK_TRUE;
		break;
	case EnableTag::DEPTH_BIAS:
		rasterizer.depthBiasEnable = VK_TRUE;
		break;
	case EnableTag::COLOR_BLEND_LOGIC_OP:
		color_blending.logicOpEnable = VK_TRUE;
		break;
	}

	return *this;
}

void GraphicsPipeline::build()
{
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = descriptor_set_layouts.size();
	pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
	pipeline_layout_info.pushConstantRangeCount = push_constant_ranges.size();
	pipeline_layout_info.pPushConstantRanges = push_constant_ranges.data(); 

	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; //This will ALWAYS be triangles

	viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_info.viewportCount = 1;
	viewport_info.scissorCount = 1;

	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; //This might change only for debugging porpuses
	rasterizer.lineWidth = 1.0f; //We won't ever use lines
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; //Might change for deferred rendering and some custom renderers

	depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; //Default value was less, but we find this more useful

	//Default blend settings for now
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;

	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = dynamic_states.size();
	dynamic_state.pDynamicStates = dynamic_states.data();

	create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	create_info.pVertexInputState = &vertex_input_info;
	create_info.pInputAssemblyState = &input_assembly_info;
	create_info.pRasterizationState = &rasterizer;
	create_info.pMultisampleState = &multisampling;
	create_info.pColorBlendState = &color_blending;
	create_info.pDynamicState = &dynamic_state;
	create_info.renderPass = render_pass;
	create_info.subpass = subpass;
	create_info.pDepthStencilState = &depth_stencil_info;

	create();
}

void GraphicsPipeline::bind()
{
	if (Graphics::active.pipeline == pipeline && Graphics::active.bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS)
		return; 
	vkCmdBindPipeline(Graphics::active.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	Graphics::active.pipeline = pipeline;
	Graphics::active.pipeline_layout = pipeline_layout;
	Graphics::active.bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
}

void GraphicsPipeline::create()
{
	Graphics::vulkanAssert(vkCreatePipelineLayout(*Graphics::logical_device, &pipeline_layout_info, nullptr, &pipeline_layout));
	create_info.layout = pipeline_layout;

	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = Graphics::swap_chain->getExtent().height;
	viewport.width = Graphics::swap_chain->getExtent().width;
	viewport.height = -(float)Graphics::swap_chain->getExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport_info.pViewports = &viewport;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = Graphics::swap_chain->getExtent();
	viewport_info.pScissors = &scissor;
	create_info.pViewportState = &viewport_info;

	Graphics::vulkanAssert(vkCreateGraphicsPipelines(*Graphics::logical_device, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline));
}