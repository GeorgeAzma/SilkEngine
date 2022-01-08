#pragma once

inline struct GraphicsState
{
	VkCommandBuffer* command_buffer = nullptr;
	VkPipelineBindPoint* bind_point = nullptr;
} graphics_state;