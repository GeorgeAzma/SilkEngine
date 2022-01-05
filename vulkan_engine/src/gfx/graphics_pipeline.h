#pragma once

#include "shader.h"

class GraphicsPipeline
{
public:
	GraphicsPipeline(Shader shader);
	~GraphicsPipeline();

	operator const VkPipeline& () const { return graphics_pipeline; }

private:
	std::vector<VkDynamicState> dynamic_states;
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
};