#include "compute_pipeline.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

ComputePipeline::ComputePipeline()
{
	Dispatcher::subscribe(this, &ComputePipeline::onWindowResize);
}

ComputePipeline::~ComputePipeline()
{
	Dispatcher::unsubscribe(this, &ComputePipeline::onWindowResize);
	destroy();
}

ComputePipeline& ComputePipeline::setShader(shared<Shader> shader)
{
	this->shader = shader;
	shader_stage_info = shader->getPipelineShaderStageInfos().back();
	create_info.stage = shader_stage_info;
	return *this;
}

ComputePipeline& ComputePipeline::addDescriptorSetLayout(const DescriptorSetLayout& layout)
{
	descriptor_set_layouts.emplace_back((const VkDescriptorSetLayout&)layout);
	return *this;
}

ComputePipeline& ComputePipeline::addPushConstant(size_t size, size_t offset)
{
	VkPushConstantRange push_constant_range{};
	push_constant_range.offset = 0;
	push_constant_range.size = size;
	push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	push_constant_ranges.emplace_back(std::move(push_constant_range));
	return *this;
}

ComputePipeline& ComputePipeline::enable(EnableTag tag)
{
	return *this;
}

void ComputePipeline::recreate()
{
	destroy();
	create();
}

void ComputePipeline::build()
{
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = descriptor_set_layouts.size();
	pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
	pipeline_layout_info.pushConstantRangeCount = push_constant_ranges.size();
	pipeline_layout_info.pPushConstantRanges = push_constant_ranges.data();

	create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

	create();
}

void ComputePipeline::bind()
{
	if (Graphics::active.pipeline == pipeline && Graphics::active.bind_point == VK_PIPELINE_BIND_POINT_COMPUTE)
		return;
	vkCmdBindPipeline(Graphics::active.command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	Graphics::active.pipeline = pipeline;
	Graphics::active.pipeline_layout = pipeline_layout;
	Graphics::active.bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;
}

void ComputePipeline::destroy()
{
	vkDestroyPipelineCache(*Graphics::logical_device, cache, nullptr);
	vkDestroyPipeline(*Graphics::logical_device, pipeline, nullptr);
	vkDestroyPipelineLayout(*Graphics::logical_device, pipeline_layout, nullptr);
}

void ComputePipeline::create()
{
	Graphics::vulkanAssert(vkCreatePipelineLayout(*Graphics::logical_device, &pipeline_layout_info, nullptr, &pipeline_layout));

	create_info.layout = pipeline_layout;

	Graphics::vulkanAssert(vkCreateComputePipelines(*Graphics::logical_device, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline));

	//Create cache
	VkPipelineCacheCreateInfo pipeline_cache_info = {};
	pipeline_cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	Graphics::vulkanAssert(vkCreatePipelineCache(*Graphics::logical_device, &pipeline_cache_info, nullptr, &cache));
}

void ComputePipeline::onWindowResize(const WindowResizeEvent& e)
{
	recreate();
}
