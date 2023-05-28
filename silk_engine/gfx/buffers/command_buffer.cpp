#include "command_buffer.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/queue.h"
#include "silk_engine/gfx/devices/logical_device.h"
#include "silk_engine/gfx/allocators/command_pool.h"
#include "silk_engine/gfx/fence.h"
#include "silk_engine/gfx/instance.h"

// TODO: Doesn't account for: "Any primary command buffer that is in the recording or executable state and has resetting command buffer recorded into it, becomes invalid."

CommandBuffer::CommandBuffer(CommandPool& command_pool, VkCommandBufferLevel level)
	: level(level), command_pool(command_pool),
	is_primary(level == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
{
	command_buffer = command_pool.allocate(level);
	*state = State::INITIAL;
}

CommandBuffer::~CommandBuffer()
{
	*state = State::INVALID;
	command_pool.deallocate(command_buffer);
}

#pragma region Begin/End
void CommandBuffer::begin(VkCommandBufferUsageFlags usage)
{
	if (*state == State::RECORDING)
		return;

	//TODO: Inheritance info and secondary command buffers

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = usage;
	vkBeginCommandBuffer(command_buffer, &begin_info);
	*state = State::RECORDING;
}

void CommandBuffer::end()
{
	if (*state != State::RECORDING)
		return;

	vkEndCommandBuffer(command_buffer);
	*state = State::EXECUTABLE;
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

#ifdef SK_ENABLE_DEBUG_OUTPUT
void CommandBuffer::beginLabel(const char* name, vec4 color)
{
	static auto vkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(RenderContext::getInstance(), "vkCmdBeginDebugUtilsLabelEXT");
	if (vkCmdBeginDebugUtilsLabelEXT == nullptr) // VK_ERROR_EXTENSION_NOT_PRESENT
		return;

	VkDebugUtilsLabelEXT label{};
	label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	label.pNext = nullptr;
	label.pLabelName = name;
	label.color[0] = color.r;
	label.color[1] = color.g;
	label.color[2] = color.b;
	label.color[3] = color.a;

	vkCmdBeginDebugUtilsLabelEXT(command_buffer, &label);
}

void CommandBuffer::insertLabel(const char* name, vec4 color)
{
	static auto vkCmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(RenderContext::getInstance(), "vkCmdInsertDebugUtilsLabelEXT");
	if (vkCmdInsertDebugUtilsLabelEXT == nullptr) // VK_ERROR_EXTENSION_NOT_PRESENT
		return;

	VkDebugUtilsLabelEXT label{};
	label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	label.pNext = nullptr;
	label.pLabelName = name;
	label.color[0] = color.r;
	label.color[1] = color.g;
	label.color[2] = color.b;
	label.color[3] = color.a;

	vkCmdInsertDebugUtilsLabelEXT(command_buffer, &label);
}

void CommandBuffer::endLabel()
{
	static auto vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(RenderContext::getInstance(), "vkCmdEndDebugUtilsLabelEXT");
	if (vkCmdEndDebugUtilsLabelEXT == nullptr) // VK_ERROR_EXTENSION_NOT_PRESENT
		return;

	vkCmdEndDebugUtilsLabelEXT(command_buffer);
}
#endif

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
	// TODO: Support dynamic offsets
	if (active.descriptor_sets.size() >= (sets.size() + first))
	{
		size_t i = 0;
		for (; i < sets.size(); ++i)
			if (sets[i] != active.descriptor_sets[i + first])
				break;
		if (i >= sets.size())
			return;
	}

	vkCmdBindDescriptorSets(command_buffer, active.pipeline_bind_point, active.pipeline_layout, first, sets.size(), sets.data(), dynamic_offsets.size(), dynamic_offsets.data());
	
	active.descriptor_sets.resize(max(first + sets.size(), active.descriptor_sets.size()));
	memcpy(active.descriptor_sets.data() + first, sets.data(), sets.size() * sizeof(VkDescriptorSet));
	active.descriptor_dynamic_offsets.resize(max(active.descriptor_dynamic_offsets.size(), dynamic_offsets.size()));
	memcpy(active.descriptor_dynamic_offsets.data(), dynamic_offsets.data(), dynamic_offsets.size() * sizeof(uint32_t));
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
	if (active.vertex_buffers.size() >= (buffers.size() + first))
	{
		size_t i = 0;
		for (; i < buffers.size(); ++i)
			if (buffers[i] != active.vertex_buffers[i + first] || active.vertex_offsets[i + first] != offsets[i])
				break;
		if (i >= buffers.size())
			return;
	}

	vkCmdBindVertexBuffers(command_buffer, first, buffers.size(), buffers.data(), offsets.data());

	active.vertex_buffers.resize(std::max(active.vertex_buffers.size(), first + buffers.size()));
	memcpy(active.vertex_buffers.data() + first, buffers.data(), buffers.size() * sizeof(VkBuffer));
	active.vertex_offsets.resize(std::max(active.vertex_offsets.size(), first + offsets.size()));
	memcpy(active.vertex_offsets.data() + first, offsets.data(), offsets.size() * sizeof(VkDeviceSize));
}

void CommandBuffer::bindVertexBuffers(uint32_t first, const std::vector<VkBuffer>& buffers)
{
	std::vector<VkDeviceSize> offsets;
	offsets.resize(buffers.size(), 0);
	bindVertexBuffers(first, buffers, offsets);
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

void CommandBuffer::setEvent(VkEvent event, VkPipelineStageFlags stage_mask) const
{
	vkCmdSetEvent(command_buffer, event, stage_mask);
}

void CommandBuffer::resetEvent(VkEvent event, VkPipelineStageFlags stage_mask) const
{
	vkCmdResetEvent(command_buffer, event, stage_mask);
}

void CommandBuffer::waitEvents(const std::vector<VkEvent>& events, VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkDependencyFlags dependency, const std::vector<VkMemoryBarrier>& memory_barriers, const std::vector<VkBufferMemoryBarrier>& buffer_barriers, const std::vector<VkImageMemoryBarrier>& image_barriers) const
{
	vkCmdWaitEvents(command_buffer, events.size(), events.data(), source_stage_mask, destination_stage_mask, memory_barriers.size(), memory_barriers.data(), buffer_barriers.size(), buffer_barriers.data(), image_barriers.size(), image_barriers.data());
}

void CommandBuffer::resetQueryPool(VkQueryPool query_pool, uint32_t first, uint32_t count) const
{
	vkCmdResetQueryPool(command_buffer, query_pool, first, count);
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

void CommandBuffer::submit(VkQueueFlagBits queue_type, const std::vector<VkPipelineStageFlags>& wait_stages, const std::vector<VkSemaphore>& wait_semaphores, const std::vector<VkSemaphore>& signal_semaphores)
{
	end();
	if (*state != State::EXECUTABLE)
		return;

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	submit_info.pWaitDstStageMask = wait_stages.data();
	submit_info.waitSemaphoreCount = wait_semaphores.size();
	submit_info.pWaitSemaphores = wait_semaphores.data();

	submit_info.signalSemaphoreCount = signal_semaphores.size();
	submit_info.pSignalSemaphores = signal_semaphores.data();
	
	if (!fence)
		fence = makeShared<Fence>();
	RenderContext::getLogicalDevice().getQueue(queue_type).submit(submit_info, *fence);
	*state = State::PENDING;
}

void CommandBuffer::execute(VkQueueFlagBits queue_type, const std::vector<VkPipelineStageFlags>& wait_stages, const std::vector<VkSemaphore>& wait_semaphores, const std::vector<VkSemaphore>& signal_semaphores)
{
	end();
	if (*state != State::EXECUTABLE)
		return;

	submit(queue_type, wait_stages, wait_semaphores, signal_semaphores);
	wait();
}	

void CommandBuffer::wait()
{
	if (!fence)
		fence = makeShared<Fence>(true);
	fence->wait();
	fence->reset();
	*state = State::INVALID;
	active = {};
}

void CommandBuffer::reset(bool free)
{
	if (*state == State::INITIAL)
		return;
	if (command_pool.getFlags() & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
		vkResetCommandBuffer(command_buffer, free * VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	*state = State::INITIAL;
}