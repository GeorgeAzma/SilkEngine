#include "pipeline.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/buffers/command_buffer.h"

Pipeline::~Pipeline()
{
	destroy();
}

void Pipeline::pushConstant(std::string_view name, const void* data) const
{
	const auto& push_constant = getShader()->getPushConstants().at(name);
	Graphics::submit([&](CommandBuffer& cb) { cb.pushConstants(push_constant.stageFlags, push_constant.offset, push_constant.size, data); });
}

void Pipeline::IsetShader(const shared<Shader>& shader, const std::vector<Constant>& constants)
{
	this->shader = shader;
	this->constants = constants;
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
			if (!(shader_constant.stage & Shader::Stage::toVulkanType(stage.type)))
				continue;

			size_t old_size = stage_specialization_info.constant_data.size();
			VkSpecializationMapEntry entry{};
			entry.constantID = shader_constant.id;
			entry.offset = old_size;
			entry.size = constant.size;
			stage_specialization_info.entries.emplace_back(std::move(entry));
			stage_specialization_info.constant_data.resize(old_size + constant.size);
			memcpy(stage_specialization_info.constant_data.data() + old_size, constant.data, constant.size);
		}

		VkSpecializationInfo specialization_info{};
		specialization_info.mapEntryCount = stage_specialization_info.entries.size();
		specialization_info.pMapEntries = stage_specialization_info.entries.data();
		specialization_info.dataSize = stage_specialization_info.constant_data.size();
		specialization_info.pData = stage_specialization_info.constant_data.data();
		stage_specialization_info.specialization_info = std::move(specialization_info);

		VkPipelineShaderStageCreateInfo shader_stage_info{};
		shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stage_info.stage = Shader::Stage::toVulkanType(stage.type);
		shader_stage_info.module = stage.module;
		shader_stage_info.pName = "main";
		shader_stage_info.pSpecializationInfo = &stage_specialization_info.specialization_info;
		shader_stage_infos.emplace_back(std::move(shader_stage_info));
	}

	push_constant_ranges.clear();
	push_constant_ranges.reserve(shader->getPushConstants().size());
	for (auto&& [name, push_constant] : shader->getPushConstants())
		push_constant_ranges.emplace_back(push_constant);

	descriptor_set_layouts.clear();
	descriptor_set_layouts.reserve(shader->getDescriptorSets().size());
	for (auto&& [set, descriptor_set] : shader->getDescriptorSets())
		descriptor_set_layouts.emplace_back(descriptor_set->getLayout());

	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = descriptor_set_layouts.size();
	pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
	pipeline_layout_info.pushConstantRangeCount = push_constant_ranges.size();
	pipeline_layout_info.pPushConstantRanges = push_constant_ranges.data();
}

void Pipeline::destroy()
{
	Graphics::logical_device->wait();
	Graphics::logical_device->destroyPipeline(pipeline);
	Graphics::logical_device->destroyPipelineLayout(layout);
}