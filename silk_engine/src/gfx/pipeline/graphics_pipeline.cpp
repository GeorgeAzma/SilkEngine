#include "graphics_pipeline.h"
#include "graphics_pipeline.h"
#include "gfx/graphics.h"
#include "gfx/window/swap_chain.h"
#include "gfx/devices/logical_device.h"

GraphicsPipeline::GraphicsPipeline()
{
	input_assembly_info.topology = vk::PrimitiveTopology::eTriangleList;

	viewport_info.viewportCount = 1;
	viewport_info.scissorCount = 1;

	rasterizer.polygonMode = vk::PolygonMode::eFill;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = vk::CullModeFlagBits::eBack;
	rasterizer.frontFace = vk::FrontFace::eCounterClockwise;

	depth_stencil_info.depthCompareOp = vk::CompareOp::eLessOrEqual;

	color_blend_attachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	color_blend_attachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
	color_blend_attachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
	color_blend_attachment.colorBlendOp = vk::BlendOp::eAdd;
	color_blend_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
	color_blend_attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
	color_blend_attachment.alphaBlendOp = vk::BlendOp::eAdd;

	color_blending.attachmentCount = 1;
}

GraphicsPipeline& GraphicsPipeline::setShader(shared<Shader> shader, const std::vector<Constant>& constants)
{
	this->shader = shader;
	shader_stage_infos.clear();
	shader_stage_infos.reserve(shader->getStages().size());
	stage_specialization_infos.clear();
	stage_specialization_infos.reserve(shader->getStages().size());

	for (const auto& stage : shader->getStages())
	{
		stage_specialization_infos.emplace_back();
		StageSpecializationInfo& stage_specialization_info = stage_specialization_infos.back();
		stage_specialization_info.entries.reserve(constants.size());

		for (const auto& constant : constants)
		{
			const auto& shader_constant = shader->getConstants().at(constant.name);
			if (!(shader_constant.stage & stage.stage))
				continue;
		
			size_t old_size = stage_specialization_info.constant_data.size();
			vk::SpecializationMapEntry entry{};
			entry.constantID = shader_constant.id;
			entry.offset = old_size;
			entry.size = constant.size;
			stage_specialization_info.entries.emplace_back(std::move(entry));
			stage_specialization_info.constant_data.resize(old_size + constant.size);
			std::memcpy(stage_specialization_info.constant_data.data() + old_size, constant.data, constant.size);
		}
		
		vk::SpecializationInfo specialization_info{};
		specialization_info.mapEntryCount = stage_specialization_info.entries.size();
		specialization_info.pMapEntries = stage_specialization_info.entries.data();
		specialization_info.dataSize = stage_specialization_info.constant_data.size();
		specialization_info.pData = stage_specialization_info.constant_data.data();
		stage_specialization_info.specialization_info = std::move(specialization_info);

		vk::PipelineShaderStageCreateInfo shader_stage_info{};
		shader_stage_info.stage = stage.stage;
		shader_stage_info.module = stage.module;
		shader_stage_info.pName = "main";
		shader_stage_info.pSpecializationInfo = &stage_specialization_info.specialization_info;
		shader_stage_infos.emplace_back(std::move(shader_stage_info));
	}

	ci.stageCount = shader_stage_infos.size();
	ci.pStages = shader_stage_infos.data();
	
	push_constant_ranges = shader->getPushConstants();
	descriptor_set_layouts.clear();
	for (auto&& [set, descriptor_set] : shader->getDescriptorSets())
		descriptor_set_layouts.emplace_back(descriptor_set->getLayout());

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

GraphicsPipeline& GraphicsPipeline::setSampleCount(vk::SampleCountFlagBits sample_count)
{
	multisampling.rasterizationSamples = sample_count;
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setRenderPass(vk::RenderPass render_pass)
{
	this->render_pass = render_pass;
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setSubpass(uint32_t subpass)
{
	this->subpass = subpass;
	return *this;
}

GraphicsPipeline& GraphicsPipeline::setDepthCompareOp(vk::CompareOp depth_compare_op)
{
	depth_stencil_info.depthCompareOp = depth_compare_op;
	return *this;
}

GraphicsPipeline& GraphicsPipeline::addDynamicState(vk::DynamicState dynamic_state)
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
	pipeline_layout_info.setLayoutCount = descriptor_set_layouts.size();
	pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
	pipeline_layout_info.pushConstantRangeCount = push_constant_ranges.size();
	pipeline_layout_info.pPushConstantRanges = push_constant_ranges.data();
	
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
	if (Graphics::active.pipeline == pipeline && Graphics::active.bind_point == vk::PipelineBindPoint::eGraphics)
		return; 
	Graphics::getActiveCommandBuffer().bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
	Graphics::active.pipeline = pipeline;
	Graphics::active.pipeline_layout = pipeline_layout;
	Graphics::active.bind_point = vk::PipelineBindPoint::eGraphics;
}

void GraphicsPipeline::create()
{
	pipeline_layout = Graphics::logical_device->createPipelineLayout(pipeline_layout_info);
	ci.layout = pipeline_layout;
	ci.pViewportState = &viewport_info;
	pipeline = Graphics::logical_device->createGraphicsPipeline(VK_NULL_HANDLE, ci);
}