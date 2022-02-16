#pragma once

#include "gfx/enums.h"
#include "shader.h"
#include "gfx/buffers/buffer_layout.h"
#include "gfx/descriptors/descriptor_set_layout.h"
#include "core/event.h"

class GraphicsPipeline : NonCopyable
{
public:
	GraphicsPipeline();
	~GraphicsPipeline();

	GraphicsPipeline& setShader(shared<Shader> shader);
	GraphicsPipeline& setVertexLayout(const BufferLayout& layout);
	GraphicsPipeline& setSampleCount(VkSampleCountFlagBits sample_count);
	GraphicsPipeline& setRenderPass(VkRenderPass render_pass);
	GraphicsPipeline& setSubpass(uint32_t subpass);
	GraphicsPipeline& addDescriptorSetLayout(VkDescriptorSetLayout layout);
	GraphicsPipeline& addDynamicState(VkDynamicState dynamic_state);
	GraphicsPipeline& addPushConstant(uint32_t size, VkShaderStageFlags shader_stages, uint32_t offset = 0);
	GraphicsPipeline& enable(EnableTag tag);

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
	VkPipeline pipeline;
	VkPipelineCache cache;
	VkPipelineLayout pipeline_layout;

private:
	std::vector<VkDynamicState> dynamic_states;
	VkPipelineVertexInputStateCreateInfo vertex_input_info{};
	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	VkPipelineMultisampleStateCreateInfo multisampling{};
	VkPipelineDepthStencilStateCreateInfo depth_stencil_info{};
	VkPipelineColorBlendAttachmentState color_blend_attachment{};
	VkPipelineColorBlendStateCreateInfo color_blending{};
	VkPipelineViewportStateCreateInfo viewport_info{};
	VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
	VkGraphicsPipelineCreateInfo create_info{};
	std::vector<VkPushConstantRange> push_constant_ranges;
	std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
	VkPipelineDynamicStateCreateInfo dynamic_state{};
	VkRenderPass render_pass = VK_NULL_HANDLE;
	uint32_t subpass = 0;
	BufferLayout layout = {}; 
	shared<Shader> shader = nullptr;
	std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos;
};