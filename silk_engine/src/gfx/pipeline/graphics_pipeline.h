#pragma once

#include "gfx/enums.h"
#include "pipeline.h"
#include "gfx/buffers/buffer_layout.h"

class GraphicsPipeline : public Pipeline
{
public:
	GraphicsPipeline& setShader(shared<Shader> shader);
	GraphicsPipeline& setVertexLayout(const BufferLayout& layout);
	GraphicsPipeline& setSampleCount(vk::SampleCountFlagBits sample_count);
	GraphicsPipeline& setRenderPass(vk::RenderPass render_pass);
	GraphicsPipeline& setSubpass(uint32_t subpass);
	GraphicsPipeline& addDynamicState(vk::DynamicState dynamic_state);
	GraphicsPipeline& enable(EnableTag tag);

	void build();
	void bind();
	
private:
	void create() override;

private:
	std::vector<vk::DynamicState> dynamic_states;
	vk::PipelineVertexInputStateCreateInfo vertex_input_info{};
	vk::PipelineRasterizationStateCreateInfo rasterizer{};
	vk::PipelineMultisampleStateCreateInfo multisampling{};
	vk::PipelineDepthStencilStateCreateInfo depth_stencil_info{};
	vk::PipelineColorBlendAttachmentState color_blend_attachment{};
	vk::PipelineColorBlendStateCreateInfo color_blending{};
	vk::PipelineViewportStateCreateInfo viewport_info{};
	vk::PipelineInputAssemblyStateCreateInfo input_assembly_info{};
	vk::PipelineDynamicStateCreateInfo dynamic_state{};
	vk::RenderPass render_pass = VK_NULL_HANDLE;
	uint32_t subpass = 0;
	BufferLayout layout = {};
	vk::GraphicsPipelineCreateInfo ci{};
};