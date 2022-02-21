#pragma once

#include "shader.h"

class WindowResizeEvent;

class Pipeline : NonCopyable
{
public:
	Pipeline();
	~Pipeline();

	void recreate()
	{
		destroy();
		create();
	}

	shared<Shader> getShader() { return shader; }
	const VkPipelineLayout& getLayout() const { return pipeline_layout; }
	operator const VkPipeline& () const { return pipeline; }

protected:
	void destroy();
	virtual void create() = 0;
	virtual void onWindowResize(const WindowResizeEvent& e) { recreate(); }

protected:
	VkPipelineCache cache;
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;

	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	std::vector<VkPushConstantRange> push_constant_ranges;
	std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
	shared<Shader> shader = nullptr;
	std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos;
};