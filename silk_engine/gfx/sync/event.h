#pragma once

class Event
{
public:
	Event(bool device_only = true);
	~Event();

	void set(PipelineStage stage) const;
	void reset(PipelineStage stage) const;
	void wait(PipelineStage source_stage, PipelineStage destination_stage, VkDependencyFlags dependency = VkDependencyFlags(0), const std::vector<VkMemoryBarrier>& memory_barriers = {}, const std::vector<VkBufferMemoryBarrier>& buffer_barriers = {}, const std::vector<VkImageMemoryBarrier>& image_barriers = {}) const;

	operator const VkEvent& () const { return event; }

private:
	VkEvent event = nullptr;
};