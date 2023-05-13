#include "event.h"
#include "render_context.h"
#include "devices/logical_device.h"

Event::Event(bool device_only)
{
	RenderContext::getLogicalDevice().createEvent();
}

Event::~Event()
{
	RenderContext::getLogicalDevice().destroyEvent(event);
}

void Event::set(VkPipelineStageFlags stage_mask) const
{
	RenderContext::getCommandBuffer().setEvent(event, stage_mask);
}

void Event::reset(VkPipelineStageFlags stage_mask) const
{
	RenderContext::getCommandBuffer().resetEvent(event, stage_mask);
}

void Event::wait(VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkDependencyFlags dependency, const std::vector<VkMemoryBarrier>& memory_barriers, const std::vector<VkBufferMemoryBarrier>& buffer_barriers, const std::vector<VkImageMemoryBarrier>& image_barriers) const
{
	RenderContext::getCommandBuffer().waitEvents({ event }, source_stage_mask, destination_stage_mask, dependency, memory_barriers, buffer_barriers, image_barriers);
}