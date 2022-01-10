#pragma once

#include "shader.h"
#include "buffers/buffer_layout.h"

struct GraphicsPipelineProps
{
	Shader* shader = nullptr;
	BufferLayout buffer_layout;
	VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT;
};

class GraphicsPipeline : NonCopyable
{
public:
	GraphicsPipeline(const GraphicsPipelineProps& props);
	~GraphicsPipeline();

	void bind(VkPipelineBindPoint bind_point = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS);

	const GraphicsPipelineProps& getProps() const { return props; }
	const VkPipelineLayout& getLayout() const { return pipeline_layout; }

	operator const VkPipeline& () const { return graphics_pipeline; }

private:
	static const std::vector<VkDynamicState> dynamic_states;
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
	GraphicsPipelineProps props;
	VkPipelineBindPoint bind_point = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
};