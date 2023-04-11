#include "pipeline.h"
#include "gfx/render_context.h"
#include "gfx/devices/logical_device.h"
#include "gfx/buffers/command_buffer.h"

Pipeline::~Pipeline()
{
	destroy();
}

void Pipeline::recreate(const std::vector<Constant>& constants)
{
	destroy();
	shader->compile();
	setShader(shader, constants);
	create();
}

void Pipeline::pushConstant(std::string_view name, const void* data) const
{
	shader->pushConstants(name, data);
}

void Pipeline::setShader(const shared<Shader>& shader, const std::vector<Constant>& constants)
{
	this->shader = shader;
	shader_stage_infos.clear();
	shader_stage_infos.resize(shader->getStages().size());

	if (constants.size())
	{
		stage_specialization_infos.clear();
		stage_specialization_infos.resize(shader->getStages().size());

		for (size_t i = 0; i < shader->getStages().size(); ++i)
		{
			const auto& stage = shader->getStages()[i];
			StageSpecializationInfo& stage_specialization_info = stage_specialization_infos[i];
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

			stage_specialization_info.build();
		}
	}

	for (size_t i = 0; i < shader->getStages().size(); ++i)
	{
		const auto& stage = shader->getStages()[i];
		VkPipelineShaderStageCreateInfo shader_stage_info{};
		shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stage_info.stage = VkShaderStageFlagBits(stage->type);
		shader_stage_info.module = stage->module;
		shader_stage_info.pName = "main";
		shader_stage_info.pSpecializationInfo = constants.size() ? &stage_specialization_infos[i].specialization_info : nullptr;
		shader_stage_infos[i] = std::move(shader_stage_info);
	}

	std::vector<VkDescriptorSetLayout> descriptor_set_layouts(shader->getDescriptorSetLayouts().size());
	size_t index = 0;
	for (auto&& [set, descriptor_set_layout] : shader->getDescriptorSetLayouts())
		descriptor_set_layouts[index++] = *descriptor_set_layout;

	// Create pipeline layout
	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = descriptor_set_layouts.size();
	pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
	pipeline_layout_info.pushConstantRangeCount = shader->getPushConstants().size();
	pipeline_layout_info.pPushConstantRanges = shader->getPushConstants().data();

	layout = RenderContext::getLogicalDevice().createPipelineLayout(pipeline_layout_info);
}

void Pipeline::destroy()
{
	RenderContext::getLogicalDevice().wait();
	if (pipeline)
	{
		RenderContext::getLogicalDevice().destroyPipeline(pipeline);
		pipeline = nullptr;
	}
	if (layout)
	{
		RenderContext::getLogicalDevice().destroyPipelineLayout(layout);
		layout = nullptr;
	}
}