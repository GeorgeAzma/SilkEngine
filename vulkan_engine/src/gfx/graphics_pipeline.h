#pragma once

#include "shader.h"

struct GraphicsPipelineProps
{
	Shader* shader;
	std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions;
	std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions;
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