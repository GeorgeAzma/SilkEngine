#include "compute_pipeline.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/buffers/command_buffer.h"

ComputePipeline::ComputePipeline(const shared<Shader>& shader, const std::vector<Constant>& constants)
{
	this->shader = shader;

	stage_specialization_infos.resize(1);
	StageSpecializationInfo& stage_specialization_info = stage_specialization_infos[0];
	stage_specialization_info.entries.reserve(constants.size());

	for (const auto& constant : constants)
	{
		const auto& shader_constant = shader->getConstants().at(constant.name);
		size_t old_size = stage_specialization_info.constant_data.size();
		VkSpecializationMapEntry entry{};
		entry.constantID = shader_constant.id;
		entry.offset = old_size;
		entry.size = constant.size;
		stage_specialization_info.entries.emplace_back(std::move(entry));
		stage_specialization_info.constant_data.resize(old_size + constant.size);
		std::memcpy(stage_specialization_info.constant_data.data() + old_size, constant.data, constant.size);
	}

	VkSpecializationInfo specialization_info{};
	specialization_info.mapEntryCount = stage_specialization_info.entries.size();
	specialization_info.pMapEntries = stage_specialization_info.entries.data();
	specialization_info.dataSize = stage_specialization_info.constant_data.size();
	specialization_info.pData = stage_specialization_info.constant_data.data();
	stage_specialization_info.specialization_info = std::move(specialization_info);

	VkPipelineShaderStageCreateInfo shader_stage_info{};
	shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shader_stage_info.module = shader->getStages().front().module;
	shader_stage_info.pName = "main";
	shader_stage_info.pSpecializationInfo = &stage_specialization_info.specialization_info;
	shader_stage_infos.emplace_back(std::move(shader_stage_info));

	ci.stage = shader_stage_infos[0];

	push_constant_ranges.clear();
	for (auto&& [name, push_constant] : shader->getPushConstants())
		push_constant_ranges.emplace_back(push_constant);

	descriptor_set_layouts.clear();
	for (auto&& [set, descriptor_set] : shader->getDescriptorSets())
		descriptor_set_layouts.emplace_back(descriptor_set->getLayout());

	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = descriptor_set_layouts.size();
	pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
	pipeline_layout_info.pushConstantRangeCount = push_constant_ranges.size();
	pipeline_layout_info.pPushConstantRanges = push_constant_ranges.data();

	ci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

	create();
}

void ComputePipeline::bind()
{
	Graphics::getActiveCommandBuffer().bindPipeline(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline, layout);
}

void ComputePipeline::dispatch(uint32_t global_invocation_count_x, uint32_t global_invocation_count_y, uint32_t global_invocation_count_z) const
{
	const glm::uvec3 global_invocation_count(global_invocation_count_x, global_invocation_count_y, global_invocation_count_z);
	const glm::uvec3 local_size = shader->getLocalSize();
	const glm::uvec3 group_count = global_invocation_count / local_size + glm::uvec3(global_invocation_count.x % local_size.x > 0, global_invocation_count.y % local_size.y > 0, global_invocation_count.z % local_size.z > 0);
	Graphics::getActiveCommandBuffer().dispatch(group_count.x, group_count.y, group_count.z);
}

void ComputePipeline::create()
{
	layout = Graphics::logical_device->createPipelineLayout(pipeline_layout_info);
	ci.layout = layout;
	pipeline = Graphics::logical_device->createComputePipeline(VK_NULL_HANDLE, ci);
}