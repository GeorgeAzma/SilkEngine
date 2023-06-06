#include "event.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/devices/logical_device.h"

Event::Event(bool device_only)
	: event(RenderContext::getLogicalDevice().createEvent(device_only)) {}

Event::~Event()
{
	RenderContext::getLogicalDevice().destroyEvent(event);
}

void Event::set(PipelineStage stage) const
{
	RenderContext::getCommandBuffer().setEvent(event, stage);
}

void Event::reset(PipelineStage stage) const
{
	RenderContext::getCommandBuffer().resetEvent(event, stage);
}

void Event::wait(PipelineStage source_stage_mask, PipelineStage destination_stage, VkDependencyFlags dependency, const std::vector<VkMemoryBarrier>& memory_barriers, const std::vector<VkBufferMemoryBarrier>& buffer_barriers, const std::vector<VkImageMemoryBarrier>& image_barriers) const
{
	RenderContext::getCommandBuffer().waitEvents({ event }, source_stage_mask, destination_stage, dependency, memory_barriers, buffer_barriers, image_barriers);
}