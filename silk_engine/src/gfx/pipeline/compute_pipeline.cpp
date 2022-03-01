#include "compute_pipeline.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

ComputePipeline::ComputePipeline(shared<Shader> shader, const std::vector<Constant>& constants)
{
	this->shader = shader;

	//std::vector<uint8_t> constant_data;
	//std::vector<vk::SpecializationMapEntry> entries(constants.size());
	//for (const auto& constant : constants)
	//{
	//	const auto& shader_constant = shader->getConstants().at(constant.name);
	//	size_t old_size = constant_data.size();
	//	vk::SpecializationMapEntry entry{};
	//	entry.constantID = shader_constant.id;
	//	entry.offset = old_size;
	//	entry.size = constant.size;
	//	entries.emplace_back(std::move(entry));
	//	constant_data.resize(old_size + constant.size);
	//	std::memcpy(constant_data.data() + old_size, constant.data, constant.size);
	//}

	//vk::SpecializationInfo specialization_info{};
	//specialization_info.mapEntryCount = entries.size();
	//specialization_info.pMapEntries = entries.data();
	//specialization_info.dataSize = constant_data.size();
	//specialization_info.pData = constant_data.data();

	vk::PipelineShaderStageCreateInfo shader_stage_info{};
	shader_stage_info.stage = vk::ShaderStageFlagBits::eCompute;
	shader_stage_info.module = shader->getStages().front().module;
	shader_stage_info.pName = "main";
	//shader_stage_info.pSpecializationInfo = &specialization_info;
	shader_stage_infos.emplace_back(std::move(shader_stage_info));

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