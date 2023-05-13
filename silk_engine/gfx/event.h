#pragma once

class Event
{
public:
	Event(bool device_only = true);
	~Event();

	void set(VkPipelineStageFlags stage_mask) const;
	void reset(VkPipelineStageFlags stage_mask) const;
	void wait(VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkDependencyFlags dependency, const std::vector<VkMemoryBarrier>& memory_barriers, const std::vector<VkBufferMemoryBarrier>& buffer_barriers, const std::vector<VkImageMemoryBarrier>& image_barriers) const;

	operator const VkEvent& () const { return event; }

private:
	VkEvent event = nullptr;
};