#include "pipeline.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

Pipeline::~Pipeline()
{
	destroy();
}

void Pipeline::destroy()
{
	Graphics::logical_device->waitIdle();
	Graphics::logical_device->destroyPipeline(pipeline);
	Graphics::logical_device->destroyPipelineLayout(pipeline_layout);
}