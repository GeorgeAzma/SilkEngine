#include "graphics_pipeline.h"
#include "gfx/graphics.h"

GraphicsPipeline::GraphicsPipeline()
{
	Dispatcher::subscribe(this, &GraphicsPipeline::onWindowResize);
}

GraphicsPipeline::~GraphicsPipeline()
{
	Dispatcher::unsubscribe(this, &GraphicsPipeline::onWindowResize);
	destroy();
}

GraphicsPipeline& GraphicsPipeline::setShader(shared<Shader> shader)
{
	this->shader = shader;
	create_info.stageCount = shader->getShaderStageInfos().size();
	create_info.pStages = shader->getShaderStageInfos().data();
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

GraphicsPipeline& GraphicsPipeline::addDescriptorSetLayout(const DescriptorSetLayout& layout)
{
	descriptor_set_layouts.emplace_back((const VkDescriptorSetLayout&)layout);
	return *this;
}

GraphicsPipeline& GraphicsPipeline::addDynamicState(VkDynamicState dynamic_state)
{
	dynamic_states.emplace_back(dynamic_state);
	return *this;
}


GraphicsPipeline& GraphicsPipeline::addPushConstant(size_t size, VkShaderStageFlagBits shader_stages, size_t offset)
{
	VkPushConstantRange push_constant_range{};
	push_constant_range.offset = 0;
	push_constant_range.size = size;
	push_constant_range.stageFlags = shader_stages;
	push_constant_ranges.emplace_back(std::move(push_constant_range));
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

void GraphicsPipeline::recreate()
{
	destroy();
	create();
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
	input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_info.viewportCount = 1;
	viewport_info.scissorCount = 1;

	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

	depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

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
	create_info.subpass = 0;
	create_info.pDepthStencilState = &depth_stencil_info;

	create();
}

void GraphicsPipeline::bind(VkPipelineBindPoint bind_point)
{
	this->bind_point = bind_point;
	vkCmdBindPipeline(Graphics::active.command_buffer, bind_point, graphics_pipeline);
	Graphics::active.graphics_pipeline = this;
}

void GraphicsPipeline::destroy()
{
	vkDestroyPipelineCache(*Graphics::logical_device, cache, nullptr);
	vkDestroyPipeline(*Graphics::logical_device, graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(*Graphics::logical_device, pipeline_layout, nullptr);
}

void GraphicsPipeline::create()
{
	Graphics::vulkanAssert(vkCreatePipelineLayout(*Graphics::logical_device, &pipeline_layout_info, nullptr, &pipeline_layout));

	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = Graphics::swap_chain->getExtent().width;
	viewport.height = Graphics::swap_chain->getExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport_info.pViewports = &viewport;
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = Graphics::swap_chain->getExtent();
	viewport_info.pScissors = &scissor;
	create_info.pViewportState = &viewport_info;
	create_info.layout = pipeline_layout;

	Graphics::vulkanAssert(vkCreateGraphicsPipelines(*Graphics::logical_device, VK_NULL_HANDLE, 1, &create_info, nullptr, &graphics_pipeline));

	//Create cache
	VkPipelineCacheCreateInfo pipeline_cache_info = {};
	pipeline_cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	Graphics::vulkanAssert(vkCreatePipelineCache(*Graphics::logical_device, &pipeline_cache_info, nullptr, &cache));
}

void GraphicsPipeline::onWindowResize(const WindowResizeEvent& e)
{
	recreate();
}
