#pragma once

#include "shader.h"

class Pipeline : NonCopyable
{
public:
	struct Constant
	{
		std::string name;
		void* data;
		size_t size;
	};

public:
	~Pipeline();

	void recreate()
	{
		destroy();
		create();
	}

	shared<Shader> getShader() { return shader; }
	const vk::PipelineLayout& getLayout() const { return pipeline_layout; }
	operator const vk::Pipeline& () const { return pipeline; }

protected:
	struct StageSpecializationInfo
	{
		vk::SpecializationInfo specialization_info;
		std::vector<uint8_t> constant_data;
		std::vector<vk::SpecializationMapEntry> entries;
	};

protected:
	void destroy();
	virtual void create() = 0;

protected:
	vk::PipelineCache cache;
	vk::Pipeline pipeline;
	vk::PipelineLayout pipeline_layout;

	vk::PipelineLayoutCreateInfo pipeline_layout_info{};
	std::vector<vk::PushConstantRange> push_constant_ranges;
	std::vector<vk::DescriptorSetLayout> descriptor_set_layouts;
	shared<Shader> shader = nullptr;
	std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_infos;
	std::vector<StageSpecializationInfo> stage_specialization_infos{};
};