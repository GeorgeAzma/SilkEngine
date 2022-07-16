#pragma once

#include "pipeline.h"
#include "pipeline_stage.h"
#include "gfx/buffers/buffer_layout.h"

class GraphicsPipeline : public Pipeline
{
public:
	GraphicsPipeline();

	GraphicsPipeline& setShader(const shared<Shader>& shader, const std::vector<Constant>& constants = {});
	GraphicsPipeline& setVertexLayout(const BufferLayout& layout);
	GraphicsPipeline& setSamples(VkSampleCountFlagBits sample_count);
	GraphicsPipeline& setRenderPass(VkRenderPass render_pass);
	GraphicsPipeline& setSubpass(uint32_t subpass);
	GraphicsPipeline& setStage(const PipelineStage& stage);
	GraphicsPipeline& addDynamicState(VkDynamicState dynamic_state);

	void build();
	void bind();
	
private:
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
	VkRenderPass render_pass = VK_NULL_HANDLE;
	uint32_t subpass = 0;
	BufferLayout buffer_layout = {};
	VkGraphicsPipelineCreateInfo ci{};
};