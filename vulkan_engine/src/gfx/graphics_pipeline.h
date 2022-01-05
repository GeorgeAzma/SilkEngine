#pragma once

#include "shader.h"

class GraphicsPipeline
{
public:
	GraphicsPipeline(Shader shader, VkRenderPass render_pass);
	~GraphicsPipeline();

private:
	std::vector<VkDynamicState> dynamic_states;
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
};