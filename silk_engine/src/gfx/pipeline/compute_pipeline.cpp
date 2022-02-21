#include "compute_pipeline.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

ComputePipeline::ComputePipeline(const std::string& shader_file)
{
	this->shader = makeShared<Shader>(shader_file);
	shader_stage_infos.resize(1);
	shader_stage_infos[0] = shader->getPipelineShaderStageInfos().back();
	create_info.stage = shader_stage_infos[0];

	push_constant_ranges = shader->getPushConstants(); 
	descriptor_set_layouts.clear();
	for (auto&& [set, descriptor_set] : shader->getDescirptorSets())
		descriptor_set_layouts.emplace_back(descriptor_set->getLayout());

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

void ComputePipeline::dispatch(uint32_t global_invocation_count_x, uint32_t global_invocation_count_y, uint32_t global_invocation_count_z) const
{
	glm::uvec3 global_invocation_count(global_invocation_count_x, global_invocation_count_y, global_invocation_count_z);
	glm::uvec3 local_size = shader->getLocalSize();
	glm::uvec3 group_count = global_invocation_count / local_size + glm::uvec3(global_invocation_count.x % local_size.x > 0, global_invocation_count.y % local_size.y > 0, global_invocation_count.z % local_size.z > 0);
	vkCmdDispatch(Graphics::active.command_buffer, group_count.x, group_count.y, group_count.z);
}

void ComputePipeline::create()
{
	Graphics::vulkanAssert(vkCreatePipelineLayout(*Graphics::logical_device, &pipeline_layout_info, nullptr, &pipeline_layout));
	create_info.layout = pipeline_layout;

	Graphics::vulkanAssert(vkCreateComputePipelines(*Graphics::logical_device, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline));
}