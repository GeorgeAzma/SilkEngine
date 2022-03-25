#include "pipeline.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/buffers/command_buffer.h"

Pipeline::~Pipeline()
{
	destroy();
}

void Pipeline::pushConstant(const std::string& name, const void* data) const
{
	const auto& push_constant = getShader()->getPushConstants().at(name);
	Graphics::getActiveCommandBuffer().pushConstants(layout, push_constant.stageFlags, push_constant.offset, push_constant.size, data);
}

void Pipeline::destroy()
{
	Graphics::logical_device->waitIdle();
	Graphics::logical_device->destroyPipeline(pipeline);
	Graphics::logical_device->destroyPipelineLayout(layout);
}