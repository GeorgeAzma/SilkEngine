#include "compute_pipeline.h"
#include "pipeline_cache.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/devices/logical_device.h"
#include "silk_engine/gfx/buffers/command_buffer.h"

ComputePipeline::ComputePipeline(const shared<Shader>& shader, const std::vector<Constant>& constants)
{
	Pipeline::setShader(shader, constants);
	ci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	create();
}

void ComputePipeline::bind()
{
	RenderContext::getCommandBuffer().bindPipeline(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline, layout);
}

void ComputePipeline::dispatch(uint32_t global_invocation_count_x, uint32_t global_invocation_count_y, uint32_t global_invocation_count_z) const
{
	uvec3 global_invocation_count(global_invocation_count_x, global_invocation_count_y, global_invocation_count_z);
	uvec3 local_size = shader->getReflectionData().local_size;
	uvec3 group_count = (global_invocation_count + local_size - uvec3(1)) / local_size;
	RenderContext::getCommandBuffer().dispatch(group_count.x, group_count.y, group_count.z);
}

void ComputePipeline::create()
{
	ci.layout = layout;
	ci.stage = shader_stage_infos.front();
	pipeline = RenderContext::getLogicalDevice().createComputePipeline(RenderContext::getPipelineCache(), ci);
}
