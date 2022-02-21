#pragma once

#include "gfx/enums.h"
#include "pipeline.h"
#include "gfx/buffers/buffer_layout.h"

class GraphicsPipeline : public Pipeline
{
public:
	GraphicsPipeline& setShader(const std::filesystem::path& shader_file);
	GraphicsPipeline& setVertexLayout(const BufferLayout& layout);
	GraphicsPipeline& setSampleCount(VkSampleCountFlagBits sample_count);
	GraphicsPipeline& setRenderPass(VkRenderPass render_pass);
	GraphicsPipeline& setSubpass(uint32_t subpass);
	GraphicsPipeline& addDynamicState(VkDynamicState dynamic_state);
	GraphicsPipeline& enable(EnableTag tag);

	void build();
	void bind();
	
private:
	void create() override;

private:
	std::vector<VkDynamicState> dynamic_states;
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
	BufferLayout layout = {};
	VkGraphicsPipelineCreateInfo create_info{};
};