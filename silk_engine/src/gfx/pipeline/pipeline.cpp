#include "pipeline.h"
#include "core/event.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

Pipeline::Pipeline()
{
	Dispatcher::subscribe(this, &Pipeline::onWindowResize);
}

Pipeline::~Pipeline()
{
	Dispatcher::unsubscribe(this, &Pipeline::onWindowResize);
	destroy();
}

void Pipeline::destroy()
{
	vkDeviceWaitIdle(*Graphics::logical_device);
	//vkDestroyPipelineCache(*Graphics::logical_device, cache, nullptr);
	vkDestroyPipeline(*Graphics::logical_device, pipeline, nullptr);
	vkDestroyPipelineLayout(*Graphics::logical_device, pipeline_layout, nullptr);
}