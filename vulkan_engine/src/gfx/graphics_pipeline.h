#pragma once

#include "shader.h"
#include "buffers/buffer_layout.h"

struct GraphicsPipelineProps
{
	Shader* shader;
	BufferLayout buffer_layout;
};

class GraphicsPipeline
{
public:
	GraphicsPipeline(const GraphicsPipelineProps& props);
	~GraphicsPipeline();

	void bind(VkPipelineBindPoint bind_point = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS);

	operator const VkPipeline& () const { return graphics_pipeline; }

private:
	static const std::vector<VkDynamicState> dynamic_states;
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
};