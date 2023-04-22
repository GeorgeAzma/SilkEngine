#include "command_buffer.h"
#include "gfx/render_context.h"
#include "gfx/queues/queue.h"
#include "gfx/devices/logical_device.h"
#include "gfx/allocators/command_pool.h"
#include "gfx/fence.h"

//TODO: Doesn't account for: "Any primary command buffer that is in the recording or executable state and has resetting command buffer recorded into it, becomes invalid."

CommandBuffer::CommandBuffer(CommandPool& command_pool, VkCommandBufferLevel level)
	: level(level), pool(command_pool),
	is_primary(level == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
{
	command_buffer = pool.allocate(level);
	state = State::INITIAL;
}

CommandBuffer::~CommandBuffer()
{
	pool.deallocate(command_buffer);
}

#pragma region Begin/End
void CommandBuffer::begin(VkCommandBufferUsageFlags usage)
{
	if (state == State::RECORDING)
		return;

	//TODO: Inheritance info and secondary command buffers

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = usage;
	vkBeginCommandBuffer(command_buffer, &begin_info);
	state = State::RECORDING;
	active = {};
}

void CommandBuffer::end()
{
	if (state != State::RECORDING)
		return;

	vkEndCommandBuffer(command_buffer);
	state = State::EXECUTABLE;
}

void CommandBuffer::beginQuery(VkQueryPool query_pool, uint32_t query, VkQueryControlFlags flags)
{
	if (active.query_pool == query_pool)
		return;
	vkCmdBeginQuery(command_buffer, query_pool, query, flags);
	active.query_pool = query_pool;
}

void CommandBuffer::endQuery(VkQueryPool query_pool, uint32_t query)
{
	if (active.query_pool != query_pool)
		return;
	vkCmdEndQuery(command_buffer, query_pool, query);
	active.query_pool = nullptr;
}

void CommandBuffer::beginRenderPass(const VkRenderPassBeginInfo& render_pass_begin_info, VkSubpassContents contents)
{
	if (active.render_pass == render_pass_begin_info.renderPass)
		return;
	vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, contents);
	active.render_pass = render_pass_begin_info.renderPass;
	active.framebuffer = render_pass_begin_info.framebuffer;
	active.render_area = render_pass_begin_info.renderArea;
	active.subpass = 0;
}

void CommandBuffer::nextSubpass(VkSubpassContents contents)
{
	SK_ASSERT(active.render_pass != VkRenderPass(nullptr), "Can't call nextSubpass() when there is no active render pass in this command buffer");
	vkCmdNextSubpass(command_buffer, contents);
	++active.subpass;
}

void CommandBuffer::endRenderPass()
{
	if (active.render_pass == VkRenderPass(nullptr))
		return;
	vkCmdEndRenderPass(command_buffer);
	active.render_pass = nullptr;
	active.framebuffer = nullptr;
	active.render_area = { { None<int32_t>(), None<int32_t>() }, { None<uint32_t>(), None<uint32_t>() } };
}
#pragma endregion

#pragma region Binds
void CommandBuffer::bindPipeline(VkPipelineBindPoint bind_point, VkPipeline pipeline, VkPipelineLayout layout)
{
	if (active.pipeline == pipeline && active.pipeline_bind_point == bind_point)
		return;
	vkCmdBindPipeline(command_buffer, bind_point, pipeline);
	active.pipeline = pipeline;
	active.pipeline_layout = layout;
	active.pipeline_bind_point = bind_point;
}

void CommandBuffer::bindDescriptorSets(uint32_t first, const std::vector<VkDescriptorSet>& sets, const std::vector<uint32_t>& dynamic_offsets)
{
	if (active.descriptor_sets.size() < (sets.size() + first))
		goto down;
	for (size_t i = 0; i < sets.size(); ++i)
		if (sets[i] != active.descriptor_sets[i + first].set)
			goto down;
	for (size_t i = 0; i < sets.size(); ++i)
	{
		if (dynamic_offsets.size() != active.descriptor_sets[i + first].dynamic_offsets.size())
			goto down;
		for (size_t j = 0; j < dynamic_offsets.size(); ++j)
			if (dynamic_offsets[j] != active.descriptor_sets[i + first].dynamic_offsets[j])
				goto down;
	}
	return;
down:
	vkCmdBindDescriptorSets(command_buffer, active.pipeline_bind_point, active.pipeline_layout, first, sets.size(), sets.data(), dynamic_offsets.size(), dynamic_offsets.data());
	active.descriptor_sets.resize(std::max(first + sets.size(), active.descriptor_sets.size()));
	for (size_t i = 0; i < sets.size(); ++i)
	{
		active.descriptor_sets[first + i].set = sets[i];
		active.descriptor_sets[first + i].dynamic_offsets = dynamic_offsets;
	}
}

void CommandBuffer::bindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType index_type)
{
	// NOTE: We are not checking if index_type changed, since there is never a circumstance when that's useful
	if (active.index_buffer == buffer && active.index_buffer_offset == offset)
		return;
	vkCmdBindIndexBuffer(command_buffer, buffer, offset, index_type);
	active.index_buffer = buffer;
	active.index_buffer_offset = offset;
}

void CommandBuffer::bindVertexBuffers(uint32_t first, const std::vector<VkBuffer>& buffers, const std::vector<VkDeviceSize>& offsets)
{
	if (active.vertex_buffers.size() < (buffers.size() + first))
		goto down;
	for (size_t i = 0; i < buffers.size(); ++i)
		if (buffers[i] != active.vertex_buffers[i + first].vertex_buffer ||
			(offsets.empty() ? VkDeviceSize(0) : offsets[i]) != active.vertex_buffers[i + first].offset)
			goto down;
	return;

down:
	if (offsets.empty())
	{
		std::vector<VkDeviceSize> default_offsets(buffers.size());
		for (auto& offset : default_offsets)
			offset = 0;
		vkCmdBindVertexBuffers(command_buffer, first, buffers.size(), buffers.data(), default_offsets.data());
	}
	else
		vkCmdBindVertexBuffers(command_buffer, first, buffers.size(), buffers.data(), offsets.data());

	active.vertex_buffers.resize(std::max(active.vertex_buffers.size(), first + buffers.size()));
	for (size_t i = 0; i < buffers.size(); ++i)
	{
		active.vertex_buffers[first + i].vertex_buffer = buffers[i];
		active.vertex_buffers[first + i].offset = offsets.empty() ? VkDeviceSize(0) : offsets[i];
	}
}
#pragma endregion

#pragma region Setters
void CommandBuffer::setViewport(VkViewport viewport)
{
	if (active.viewport.width == viewport.width &&
		active.viewport.height == viewport.height &&
		active.viewport.x == viewport.x &&
		active.viewport.y == viewport.y &&
		active.viewport.minDepth == viewport.minDepth &&
		active.viewport.maxDepth == viewport.maxDepth)
		return;
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
	active.viewport = viewport;
}

void CommandBuffer::setScissor(VkRect2D scissor)
{
	if (active.scissor.extent.width == scissor.extent.width &&
		active.scissor.extent.height == scissor.extent.height &&
		active.scissor.offset.x == scissor.offset.x &&
		active.scissor.offset.y == scissor.offset.y)
		return;
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);
	active.scissor = scissor;
}

void CommandBuffer::setLineWidth(float width)
{
	if (active.line_width == width)
		return;
	vkCmdSetLineWidth(command_buffer, width);
	active.line_width = width;
}

void CommandBuffer::setDepthBias(float constant, float clamp, float slope)
{
	if (active.depth_bias_constant == constant &&
		active.depth_bias_clamp == clamp &&
		active.depth_bias_slope == slope)
		return;
	vkCmdSetDepthBias(command_buffer, constant, clamp, slope);
	active.depth_bias_constant = constant;
	active.depth_bias_clamp = clamp;
	active.depth_bias_slope = slope;
}

void CommandBuffer::setBlendConstants(const float blend_constants[4])
{
	if (active.blend_constants[0] == blend_constants[0] &&
		active.blend_constants[1] == blend_constants[1] &&
		active.blend_constants[2] == blend_constants[2] &&
		active.blend_constants[3] == blend_constants[3])
		return;
	vkCmdSetBlendConstants(command_buffer, blend_constants);
	active.blend_constants = { blend_constants[0], blend_constants[1], blend_constants[2], blend_constants[3] };
}

void CommandBuffer::setDepthBounds(float min, float max)
{
	if (active.min_depth_bound == min && active.max_depth_bound == max)
		return;
	vkCmdSetDepthBounds(command_buffer, min, max);
	active.min_depth_bound = min;
	active.max_depth_bound = max;
}

void CommandBuffer::setStencilCompareMask(VkStencilFaceFlags face_mask, uint32_t compare_mask)
{
	if (active.stencil_compare_mask_face_mask == face_mask &&
		active.stencil_compare_mask_compare_mask == compare_mask)
		return;
	vkCmdSetStencilCompareMask(command_buffer, face_mask, compare_mask);
	active.stencil_compare_mask_face_mask = face_mask;
	active.stencil_compare_mask_compare_mask = compare_mask;

}

void CommandBuffer::setDeviceMask(uint32_t device_mask)
{
	if (active.device_mask == device_mask)
		return;
	vkCmdSetDeviceMask(command_buffer, device_mask);
	active.device_mask = device_mask;
}

void CommandBuffer::setCullMode(VkCullModeFlags cull_mode)
{
	if (active.cull_mode == cull_mode)
		return;
	vkCmdSetCullMode(command_buffer, cull_mode);
	active.cull_mode = cull_mode;
}

void CommandBuffer::setFrontFace(VkFrontFace front_face)
{
	if (active.front_face == front_face)
		return;
	vkCmdSetFrontFace(command_buffer, front_face);
	active.front_face = front_face;
}

void CommandBuffer::setPrimitiveTopology(VkPrimitiveTopology primitive_topology)
{
	if (active.primitive_topology == primitive_topology)
		return;
	vkCmdSetPrimitiveTopology(command_buffer, primitive_topology);
	active.primitive_topology = primitive_topology;
}

void CommandBuffer::setViewportWithCount(VkViewport viewport)
{
	if (active.viewport.width == viewport.width &&
		active.viewport.height == viewport.height &&
		active.viewport.x == viewport.x &&
		active.viewport.y == viewport.y &&
		active.viewport.minDepth == viewport.minDepth &&
		active.viewport.maxDepth == viewport.maxDepth)
		return;
	vkCmdSetViewportWithCount(command_buffer, 1, &viewport);
	active.viewport = viewport;
}

void CommandBuffer::setScissorWithCount(VkRect2D scissor)
{
	if (active.scissor.extent.width == scissor.extent.width &&
		active.scissor.extent.height == scissor.extent.height &&
		active.scissor.offset.x == scissor.offset.x &&
		active.scissor.offset.y == scissor.offset.y)
		return;
	vkCmdSetScissorWithCount(command_buffer, 1, &scissor);
	active.scissor = scissor;
}

void CommandBuffer::setDepthTestEnable(VkBool32 depth_test_enable)
{
	if (active.depth_test_enable == depth_test_enable)
		return;
	vkCmdSetDepthTestEnable(command_buffer, depth_test_enable);
	active.depth_test_enable = depth_test_enable;
}

void CommandBuffer::setDepthWriteEnable(VkBool32 depth_write_enable)
{
	if (active.depth_write_enable == depth_write_enable)
		return;
	vkCmdSetDepthWriteEnable(command_buffer, depth_write_enable);
	active.depth_write_enable = depth_write_enable;
}

void CommandBuffer::setDepthCompareOp(VkCompareOp depth_compare_op)
{
	if (active.depth_compare_op == depth_compare_op)
		return;
	vkCmdSetDepthCompareOp(command_buffer, depth_compare_op);
	active.depth_compare_op = depth_compare_op;
}

void CommandBuffer::setDepthBoundsTestEnable(VkBool32 depth_bound_test_enable)
{
	if (active.depth_bound_test_enable == depth_bound_test_enable)
		return;
	vkCmdSetDepthBoundsTestEnable(command_buffer, depth_bound_test_enable);
	active.depth_bound_test_enable = depth_bound_test_enable;
}

void CommandBuffer::setStencilTestEnable(VkBool32 stencil_test_enable)
{
	if (active.stencil_test_enable == stencil_test_enable)
		return;
	vkCmdSetStencilTestEnable(command_buffer, stencil_test_enable);
	active.stencil_test_enable = stencil_test_enable;
}

void CommandBuffer::setStencilOp(VkStencilFaceFlags face_mask, VkStencilOp fail_op, VkStencilOp pass_op, VkStencilOp depth_fail_op, VkCompareOp compare_op)
{
	if (active.stencil_op_face_mask == face_mask &&
		active.stencil_op_fail_op == fail_op &&
		active.stencil_op_pass_op == pass_op &&
		active.stencil_op_depth_fail_op == depth_fail_op &&
		active.stencil_op_compare_op == compare_op)
		return;
	vkCmdSetStencilOp(command_buffer, face_mask, fail_op, pass_op, depth_fail_op, compare_op);
	active.stencil_op_face_mask = face_mask;
	active.stencil_op_fail_op = fail_op;
	active.stencil_op_pass_op = pass_op;
	active.stencil_op_depth_fail_op = depth_fail_op;
	active.stencil_op_compare_op = compare_op;
}

void CommandBuffer::setRasterizerDiscardEnable(VkBool32 rasterizer_discard_enable)
{
	if (active.rasterizer_discard_enable == rasterizer_discard_enable)
		return;
	vkCmdSetRasterizerDiscardEnable(command_buffer, rasterizer_discard_enable);
	active.rasterizer_discard_enable = rasterizer_discard_enable;
}

void CommandBuffer::setDepthBiasEnable(VkBool32 depth_bias_enable)
{
	if (active.depth_bias_enable == depth_bias_enable)
		return;
	vkCmdSetDepthBiasEnable(command_buffer, depth_bias_enable);
	active.depth_bias_enable = depth_bias_enable;
}

void CommandBuffer::setPrimitiveRestartEnable(VkBool32 primitive_restart_enable)
{
	if (active.primitive_restart_enable == primitive_restart_enable)
		return;
	vkCmdSetPrimitiveRestartEnable(command_buffer, primitive_restart_enable);
	active.primitive_restart_enable = primitive_restart_enable;
}
#pragma endregion 

#pragma region Commands
void CommandBuffer::executeCommands(const std::vector<VkCommandBuffer>& command_buffers) const
{
	vkCmdExecuteCommands(command_buffer, command_buffers.size(), command_buffers.data());
}

void CommandBuffer::copyBuffer(VkBuffer source, VkBuffer destination, const std::vector<VkBufferCopy>& copy_regions) const
{
	vkCmdCopyBuffer(command_buffer, source, destination, copy_regions.size(), copy_regions.data());
}

void CommandBuffer::pipelineBarrier(VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkDependencyFlags dependency, const std::vector<VkMemoryBarrier>& memory_barriers, const std::vector<VkBufferMemoryBarrier>& buffer_barriers, const std::vector<VkImageMemoryBarrier>& image_barriers) const
{
	vkCmdPipelineBarrier(command_buffer, source_stage_mask, destination_stage_mask, dependency, memory_barriers.size(), memory_barriers.data(), buffer_barriers.size(), buffer_barriers.data(), image_barriers.size(), image_barriers.data());
}

void CommandBuffer::pipelineBarrier(VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, const std::vector<VkBufferMemoryBarrier>& buffer_barriers, const std::vector<VkMemoryBarrier>& memory_barriers) const
{
	pipelineBarrier(source_stage_mask, destination_stage_mask, VkDependencyFlags(0), memory_barriers, buffer_barriers, {});
}
void CommandBuffer::pipelineBarrier(VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, const std::vector<VkImageMemoryBarrier>& image_barriers, const std::vector<VkMemoryBarrier>& memory_barriers) const
{
	pipelineBarrier(source_stage_mask, destination_stage_mask, VkDependencyFlags(0), memory_barriers, {}, image_barriers);
}

void CommandBuffer::blitImage(VkImage source, VkImageLayout source_layout, VkImage destination, VkImageLayout destination_layout, const std::vector<VkImageBlit>& blit_regions, VkFilter filter) const
{
	vkCmdBlitImage(command_buffer, source, source_layout, destination, destination_layout, blit_regions.size(), blit_regions.data(), filter);
}

void CommandBuffer::copyBufferToImage(VkBuffer buffer, VkImage image, VkImageLayout image_layout, const std::vector<VkBufferImageCopy>& copy_regions) const
{
	vkCmdCopyBufferToImage(command_buffer, buffer, image, image_layout, copy_regions.size(), copy_regions.data());
}

void CommandBuffer::copyImageToBuffer(VkImage image, VkImageLayout image_layout, VkBuffer buffer, const std::vector<VkBufferImageCopy>& copy_regions) const
{
	vkCmdCopyImageToBuffer(command_buffer, image, image_layout, buffer, copy_regions.size(), copy_regions.data());
}

void CommandBuffer::copyImage(VkImage source, VkImageLayout source_layout, VkImage destination, VkImageLayout destination_layout, const std::vector<VkImageCopy>& copy_regions) const
{
	vkCmdCopyImage(command_buffer, source, source_layout, destination, destination_layout, copy_regions.size(), copy_regions.data());
}

void CommandBuffer::dispatch(uint32_t global_invocation_count_x, uint32_t global_invocation_count_y, uint32_t global_invocation_count_z) const
{
	vkCmdDispatch(command_buffer, global_invocation_count_x, global_invocation_count_y, global_invocation_count_z);
}

void CommandBuffer::pushConstants(VkPipelineStageFlags stages, uint32_t offset, uint32_t size, const void* data) const
{
	vkCmdPushConstants(command_buffer, active.pipeline_layout, stages, offset, size, data);
}

void CommandBuffer::draw(uint32_t vertices, uint32_t instances, uint32_t first_vertex, uint32_t first_instance) const
{
	vkCmdDraw(command_buffer, vertices, instances, first_vertex, first_instance);
}

void CommandBuffer::drawIndexed(uint32_t indices, uint32_t instances, uint32_t first_index, uint32_t vertex_offset, uint32_t first_instance) const
{
	vkCmdDrawIndexed(command_buffer, indices, instances, first_index, vertex_offset, first_instance);
}

void CommandBuffer::drawIndirect(VkBuffer indirect_buffer, uint32_t offset, uint32_t draw_count, uint32_t stride) const
{
	vkCmdDrawIndirect(command_buffer, indirect_buffer, offset, draw_count, stride);
}

void CommandBuffer::drawIndexedIndirect(VkBuffer indirect_buffer, uint32_t offset, uint32_t draw_count, uint32_t stride) const
{
	vkCmdDrawIndexedIndirect(command_buffer, indirect_buffer, offset, draw_count, stride);
}
#pragma endregion

void CommandBuffer::submit(const SubmitInfo& info, VkQueueFlagBits queue_type)
{
	end();
	if (state != State::EXECUTABLE)
		return;

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	submit_info.pWaitDstStageMask = info.wait_stages;
	submit_info.waitSemaphoreCount = info.wait_semaphores.size();
	submit_info.pWaitSemaphores = info.wait_semaphores.data();

	submit_info.signalSemaphoreCount = info.signal_semaphores.size();
	submit_info.pSignalSemaphores = info.signal_semaphores.data();

	if (info.fence)
		info.fence->reset();
	RenderContext::getLogicalDevice().getQueue(queue_type).submit(submit_info, info.fence ? *info.fence : nullptr);
	state = State::PENDING;
}

void CommandBuffer::submitImmidiatly(VkQueueFlagBits queue_type)
{
	end();
	if (state != State::EXECUTABLE)
		return;

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	RenderContext::getLogicalDevice().getQueue(queue_type).submitImmidiatly(submit_info);
	state = State::INVALID;
}

void CommandBuffer::reset(bool free)
{
	if (state == State::INITIAL)
		return;
	vkResetCommandBuffer(command_buffer, free * VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	state = State::INITIAL;
	active = {};
}