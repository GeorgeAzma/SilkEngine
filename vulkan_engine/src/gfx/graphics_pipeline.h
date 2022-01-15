#pragma once

#include "enums.h"
#include "shader.h"
#include "buffers/buffer_layout.h"
#include "descriptor_set_layout.h"
#include "core/event.h"

struct GraphicsPipelineProps
{
	Shader* shader = nullptr;
	BufferLayout buffer_layout;
	VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT;
};

class GraphicsPipeline : NonCopyable
{
public:
	GraphicsPipeline();
	~GraphicsPipeline();

	GraphicsPipeline& setShader(std::shared_ptr<Shader> shader);
	GraphicsPipeline& setVertexLayout(const BufferLayout& layout);
	GraphicsPipeline& setSampleCount(VkSampleCountFlagBits sample_count);
	GraphicsPipeline& setRenderPass(VkRenderPass render_pass);
	GraphicsPipeline& addDescriptorSetLayout(const DescriptorSetLayout& layout);
	GraphicsPipeline& addDynamicState(VkDynamicState dynamic_state);
	GraphicsPipeline& addPushConstant(size_t size, VkShaderStageFlagBits shader_stages, size_t offset = 0);
	GraphicsPipeline& enable(EnableTag tag);

	void recreate();

	void build();

	void bind(VkPipelineBindPoint bind_point = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS);

	const VkPipelineLayout& getLayout() const { return pipeline_layout; }
	VkPipelineBindPoint getBindPoint() const { return bind_point; }

	operator const VkPipeline& () const { return graphics_pipeline; }
	
private:
	void destroy();
	void create();
	void onWindowResize(const WindowResizeEvent& e);

private:
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
	VkPipelineBindPoint bind_point = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
	VkPipelineCache cache;

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
	VkRenderPass render_pass = VK_NULL_HANDLE;
	VkPipelineDynamicStateCreateInfo dynamic_state{};
	BufferLayout layout = {}; 
	std::shared_ptr<Shader> shader;
};