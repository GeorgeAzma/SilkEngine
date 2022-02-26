#include "compute_pipeline.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

ComputePipeline::ComputePipeline(shared<Shader> shader)
{
	this->shader = shader;
	shader_stage_infos.resize(1);
	shader_stage_infos[0] = shader->getPipelineShaderStageInfos().back();
	ci.stage = shader_stage_infos[0];

	push_constant_ranges = shader->getPushConstants(); 
	descriptor_set_layouts.clear();
	for (auto&& [set, descriptor_set] : shader->getDescriptorSets())
		descriptor_set_layouts.emplace_back(descriptor_set->getLayout());

	pipeline_layout_info.setLayoutCount = descriptor_set_layouts.size();
	pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
	pipeline_layout_info.pushConstantRangeCount = push_constant_ranges.size();
	pipeline_layout_info.pPushConstantRanges = push_constant_ranges.data();

	create();
}

void ComputePipeline::bind()
{
	if (Graphics::active.pipeline == pipeline && Graphics::active.bind_point == vk::PipelineBindPoint::eCompute)
		return;
	Graphics::active.command_buffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
	Graphics::active.pipeline = pipeline;
	Graphics::active.pipeline_layout = pipeline_layout;
	Graphics::active.bind_point = vk::PipelineBindPoint::eCompute;
}

void ComputePipeline::dispatch(uint32_t global_invocation_count_x, uint32_t global_invocation_count_y, uint32_t global_invocation_count_z) const
{
	const glm::uvec3 global_invocation_count(global_invocation_count_x, global_invocation_count_y, global_invocation_count_z);
	const glm::uvec3 local_size = shader->getLocalSize();
	const glm::uvec3 group_count = global_invocation_count / local_size + glm::uvec3(global_invocation_count.x % local_size.x > 0, global_invocation_count.y % local_size.y > 0, global_invocation_count.z % local_size.z > 0);
	Graphics::active.command_buffer.dispatch(group_count.x, group_count.y, group_count.z);
}

void ComputePipeline::create()
{
	pipeline_layout = Graphics::logical_device->createPipelineLayout(pipeline_layout_info);
	ci.layout = pipeline_layout;
	pipeline = Graphics::logical_device->createComputePipeline(VK_NULL_HANDLE, ci);
}