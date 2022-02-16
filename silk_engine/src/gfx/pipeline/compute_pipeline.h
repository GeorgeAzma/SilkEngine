#pragma once

#include "gfx/enums.h"
#include "shader.h"
#include "gfx/descriptors/descriptor_set_layout.h"
#include "core/event.h"

class ComputePipeline : NonCopyable
{
public:
	ComputePipeline();
	~ComputePipeline();

	ComputePipeline& setShader(shared<Shader> shader);
	ComputePipeline& addDescriptorSetLayout(const DescriptorSetLayout& layout);
	ComputePipeline& addPushConstant(size_t size, size_t offset = 0);
	ComputePipeline& enable(EnableTag tag);

	void recreate();

	void build();

	void bind();

	const VkPipelineLayout& getLayout() const { return pipeline_layout; }

	operator const VkPipeline& () const { return pipeline; }
	
private:
	void destroy();
	void create();
	void onWindowResize(const WindowResizeEvent& e);

private:
	VkPipelineCache cache;
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;

private:
	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	VkComputePipelineCreateInfo create_info{};
	std::vector<VkPushConstantRange> push_constant_ranges;
	std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
	shared<Shader> shader = nullptr;
	VkPipelineShaderStageCreateInfo shader_stage_info;
};