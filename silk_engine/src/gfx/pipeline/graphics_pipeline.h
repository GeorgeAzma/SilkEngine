#pragma once

#include "pipeline.h"
#include "pipeline_stage.h"

class GraphicsPipeline : public Pipeline
{
public:
	GraphicsPipeline();

	GraphicsPipeline& setShader(const shared<Shader>& shader, const std::vector<Constant>& constants = {});
	GraphicsPipeline& setSamples(VkSampleCountFlagBits sample_count);
	GraphicsPipeline& setRenderPass(VkRenderPass render_pass);
	GraphicsPipeline& setSubpass(uint32_t subpass);
	GraphicsPipeline& setStage(const PipelineStage& stage);
	GraphicsPipeline& addDynamicState(VkDynamicState dynamic_state);

	void build();
	void bind();
	
private:
	void IsetShader(const shared<Shader>& shader, const std::vector<Constant>& constants = {}) override;
	void create() override;

private:
	std::vector<VkDynamicState> dynamic_states{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineVertexInputStateCreateInfo vertex_input_info{};
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	VkPipelineMultisampleStateCreateInfo multisampling{};
	VkPipelineDepthStencilStateCreateInfo depth_stencil_info{};
	VkPipelineColorBlendAttachmentState color_blend_attachment{};
	VkPipelineColorBlendStateCreateInfo color_blending{};
	VkPipelineViewportStateCreateInfo viewport_info{};
	VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
	VkPipelineDynamicStateCreateInfo dynamic_state{};
	VkRenderPass render_pass = nullptr;
	uint32_t subpass = 0;
	VkGraphicsPipelineCreateInfo ci{};
};