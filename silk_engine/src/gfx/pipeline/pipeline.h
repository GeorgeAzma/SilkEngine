#pragma once

#include "shader.h"

enum class EnableTag
{
	DEPTH_TEST,
	DEPTH_WRITE,
	STENCIL_TEST,
	COLOR_BLENDING,
	SAMPLE_SHADING,
	PRIMITIVE_RESTART,
	RASTERIZER_DISCARD,
	DEPTH_CLAMP,
	DEPTH_BIAS,
	COLOR_BLEND_LOGIC_OP
};

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

	void pushConstant(std::string_view name, const void* data) const;

	const shared<Shader>& getShader() const { return shader; }
	const VkPipelineLayout& getLayout() const { return layout; }
	operator const VkPipeline& () const { return pipeline; }

protected:
	struct StageSpecializationInfo
	{
		VkSpecializationInfo specialization_info;
		std::vector<uint8_t> constant_data;
		std::vector<VkSpecializationMapEntry> entries;
	};

protected:
	void destroy();
	virtual void create() = 0;

protected:
	VkPipelineCache cache;
	VkPipeline pipeline;
	VkPipelineLayout layout;

	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	std::vector<VkPushConstantRange> push_constant_ranges;
	std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
	shared<Shader> shader = nullptr;
	std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos;
	std::vector<StageSpecializationInfo> stage_specialization_infos{};
};