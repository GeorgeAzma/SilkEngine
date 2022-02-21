#include "compute_pipeline.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

ComputePipeline& ComputePipeline::setShader(shared<Shader> shader)
{
	this->shader = shader;
	shader_stage_infos.resize(1);
	shader_stage_infos[0] = shader->getPipelineShaderStageInfos().back();
	create_info.stage = shader_stage_infos[0];

	push_constant_ranges = shader->getPushConstantRanges();
	if (shader->getDescriptorSet().get())
		descriptor_set_layouts = { shader->getDescriptorSet()->getLayout() };

	return *this;
}

void ComputePipeline::build()
{
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = descriptor_set_layouts.size();
	pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
	pipeline_layout_info.pushConstantRangeCount = push_constant_ranges.size();
	pipeline_layout_info.pPushConstantRanges = push_constant_ranges.data();

	create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

	create();
}

void ComputePipeline::bind()
{
	if (Graphics::active.pipeline == pipeline && Graphics::active.bind_point == VK_PIPELINE_BIND_POINT_COMPUTE)
		return;
	vkCmdBindPipeline(Graphics::active.command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	Graphics::active.pipeline = pipeline;
	Graphics::active.pipeline_layout = pipeline_layout;
	Graphics::active.bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;
}

void ComputePipeline::create()
{
	Graphics::vulkanAssert(vkCreatePipelineLayout(*Graphics::logical_device, &pipeline_layout_info, nullptr, &pipeline_layout));
	create_info.layout = pipeline_layout;

	Graphics::vulkanAssert(vkCreateComputePipelines(*Graphics::logical_device, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline));
}