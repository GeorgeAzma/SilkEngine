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
	shader->pushConstants(name, data);
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
			if (!(shader_constant.stage & VkShaderStageFlags(stage->type)))
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
		shader_stage_info.stage = VkShaderStageFlagBits(stage->type);
		shader_stage_info.module = stage->module;
		shader_stage_info.pName = "main";
		shader_stage_info.pSpecializationInfo = &stage_specialization_info.specialization_info;
		shader_stage_infos.emplace_back(std::move(shader_stage_info));
	}

	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = shader->getDescriptorSetLayouts().size();
	pipeline_layout_info.pSetLayouts = shader->getDescriptorSetLayouts().data();
	pipeline_layout_info.pushConstantRangeCount = shader->getPushConstants().size();
	pipeline_layout_info.pPushConstantRanges = shader->getPushConstants().data();
}

void Pipeline::destroy()
{
	Graphics::logical_device->wait();
	Graphics::logical_device->destroyPipeline(pipeline);
	Graphics::logical_device->destroyPipelineLayout(layout);
}