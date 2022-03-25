#include "command_buffer.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/allocators/command_pool.h"

CommandBuffer::CommandBuffer(vk::CommandBufferLevel level, vk::QueueFlagBits queue_type)
	: level(level), queue_type(queue_type), pool(Graphics::getCommandPool()), 
	vk::CommandBuffer(VkCommandBuffer(Graphics::logical_device->allocateCommandBuffers({ *Graphics::getCommandPool(), level, 1 }).front())),
	is_primary(level == vk::CommandBufferLevel::ePrimary)
{
}

CommandBuffer::~CommandBuffer()
{
	Graphics::logical_device->freeCommandBuffers(*pool, { *this });
}

void CommandBuffer::begin(vk::CommandBufferUsageFlags usage)
{
	if (running)
		return;

	vk::CommandBufferInheritanceInfo inheritance_info{};
	if (usage & vk::CommandBufferUsageFlagBits::eRenderPassContinue)
	{
		SK_ASSERT(!is_primary, "Only primary command buffers can have eRenderPassContinue flag");
		const auto& primary_command_buffer = Graphics::getActivePrimaryCommandBuffer();
		inheritance_info.renderPass = primary_command_buffer.getActive().render_pass;
		inheritance_info.subpass = primary_command_buffer.getActive().subpass;
		inheritance_info.framebuffer = primary_command_buffer.getActive().framebuffer;
		//inheritance_info.occlusionQueryEnable = ; //TODO:
		//inheritance_info.pipelineStatistics = ;  //TODO:
	}

	vk::CommandBuffer::begin(vk::CommandBufferBeginInfo(usage, &inheritance_info));
	Graphics::setActiveCommandBuffer(this);
	if(is_primary)
		Graphics::setActivePrimaryCommandBuffer(this);
	running = true;
}

void CommandBuffer::end()
{
	if (!running)
		return;

	vk::CommandBuffer::end();
	Graphics::setActiveCommandBuffer(nullptr);
	if (is_primary)
		Graphics::setActivePrimaryCommandBuffer(nullptr);
	recorded = true;
	running = false;
	active = {};
}

void CommandBuffer::bindPipeline(vk::PipelineBindPoint bind_point, vk::Pipeline pipeline, vk::PipelineLayout layout)
{
	if (active.pipeline == pipeline && active.pipeline_bind_point == bind_point)
		return;
	vk::CommandBuffer::bindPipeline(bind_point, pipeline);
	active.pipeline = pipeline;
	active.pipeline_layout = layout;
	active.pipeline_bind_point = bind_point;
}

void CommandBuffer::setViewport(vk::Viewport viewport)
{
	if (active.viewport == viewport)
		return;
	vk::CommandBuffer::setViewport(0, { viewport });
	active.viewport = viewport;
}

void CommandBuffer::setScissor(vk::Rect2D scissor)
{
	if (active.scissor == scissor)
		return;
	vk::CommandBuffer::setScissor(0, { scissor });
	active.scissor = scissor;
}

void CommandBuffer::setLineWidth(float width)
{
	if (active.line_width == width)
		return;
	vk::CommandBuffer::setLineWidth(width);
	active.line_width = width;
}

void CommandBuffer::setDepthBias(float constant, float clamp, float slope)
{
	if (active.depth_bias_constant == constant &&
		active.depth_bias_clamp == clamp &&
		active.depth_bias_slope == slope)
		return;
	vk::CommandBuffer::setDepthBias(constant, clamp, slope);
	active.depth_bias_constant = constant;
	active.depth_bias_clamp = clamp;
	active.depth_bias_slope = slope;
}

void CommandBuffer::setBlendConstants(const float blend_constants[4])
{
	if (active.blend_constants &&
		active.blend_constants->at(0) == blend_constants[0] &&
		active.blend_constants->at(1) == blend_constants[1] &&
		active.blend_constants->at(2) == blend_constants[2] &&
		active.blend_constants->at(3) == blend_constants[3])
		return;
	vk::CommandBuffer::setBlendConstants(blend_constants);
	active.blend_constants = { blend_constants[0], blend_constants[1], blend_constants[2], blend_constants[3] };
}

void CommandBuffer::setDepthBounds(float min, float max)
{
	if (active.min_depth_bound == min && active.max_depth_bound == max)
		return;
	vk::CommandBuffer::setDepthBounds(min, max);
	active.min_depth_bound = min;
	active.max_depth_bound = max;
}

void CommandBuffer::setStencilCompareMask(vk::StencilFaceFlags face_mask, uint32_t compare_mask)
{
	if (active.stencil_compare_mask_face_mask == face_mask &&
		active.stencil_compare_mask_compare_mask == compare_mask)
		return;
	vk::CommandBuffer::setStencilCompareMask(face_mask, compare_mask);
	active.stencil_compare_mask_face_mask = face_mask;
	active.stencil_compare_mask_compare_mask = compare_mask;

}

void CommandBuffer::bindDescriptorSets(uint32_t first, const std::vector<vk::DescriptorSet>& sets, const std::vector<uint32_t>& dynamic_offsets)
{	
	//UNTESTED:
	bool needs_binding = false;
	if (active.descriptor_sets.size() < (sets.size() + first))
	{
		needs_binding = true;
	}
	else
	{
		for (size_t i = 0; i < sets.size(); ++i)
		{
			if (sets[i] != active.descriptor_sets[i + first].set)
			{
				needs_binding = true;
				break;
			}
		}
		for (size_t i = 0; i < sets.size(); ++i)
		{
			if (dynamic_offsets.size() != active.descriptor_sets[i + first].dynamic_offsets.size())
			{
				needs_binding = true;
				break;
			}
			for (size_t j = 0; j < dynamic_offsets.size(); ++j)
			{
				if (dynamic_offsets[j] != active.descriptor_sets[i + first].dynamic_offsets[j])
				{
					needs_binding = true;
					break;
				}
			}
		}
	}
	if (!needs_binding)
		return;
	vk::CommandBuffer::bindDescriptorSets(active.pipeline_bind_point, active.pipeline_layout, first, sets, dynamic_offsets);
	active.descriptor_sets.resize(std::max(first + sets.size(), active.descriptor_sets.size()));
	for (size_t i = 0; i < sets.size(); ++i)
	{
		active.descriptor_sets[first + i].set = sets[i];
		active.descriptor_sets[first + i].dynamic_offsets = dynamic_offsets;
	}
}

void CommandBuffer::bindIndexBuffer(vk::Buffer buffer, vk::DeviceSize offset, vk::IndexType index_type)
{
	//NOTE: We are not checking if index_type changed, since there is never a circumstance when that's useful
	if (active.index_buffer == buffer && active.index_buffer_offset == offset)
		return;
	vk::CommandBuffer::bindIndexBuffer(buffer, offset, index_type);
	active.index_buffer = buffer;
	active.index_buffer_offset = offset;
}

void CommandBuffer::bindVertexBuffers(uint32_t first, const std::vector<vk::Buffer>& buffers, const std::vector<vk::DeviceSize>& offsets)
{
	//UNTESTED:
	bool needs_binding = false;
	if (active.vertex_buffers.size() < (buffers.size() + first))
	{
		needs_binding = true;
	}
	else
	{
		for (size_t i = 0; i < buffers.size(); ++i)
		{
			if (buffers[i] != active.vertex_buffers[i + first].vertex_buffer || 
				(offsets.empty() ? vk::DeviceSize(0) : offsets[i]) != active.vertex_buffers[i + first].offset)
			{
				needs_binding = true;
				break;
			}
		}
	}
	if (!needs_binding)
		return;
	if (offsets.empty())
	{
		std::vector<vk::DeviceSize> default_offsets(buffers.size());
		for (auto& offset : default_offsets)
			offset = 0;
		vk::CommandBuffer::bindVertexBuffers(first, buffers, default_offsets);
	}
	else
		vk::CommandBuffer::bindVertexBuffers(first, buffers, offsets);

	active.vertex_buffers.resize(std::max(active.vertex_buffers.size(), first + buffers.size()));
	for (size_t i = 0; i < buffers.size(); ++i)
	{
		active.vertex_buffers[first + i].vertex_buffer = buffers[i];
		active.vertex_buffers[first + i].offset = offsets.empty() ? vk::DeviceSize(0) : offsets[i];
	}
}

void CommandBuffer::beginQuery(vk::QueryPool query_pool, uint32_t query, vk::QueryControlFlags flags)
{
	if (active.query_pool == query_pool)
		return;
	vk::CommandBuffer::beginQuery(query_pool, query, flags);
	active.query_pool = query_pool;
}

void CommandBuffer::endQuery(vk::QueryPool query_pool, uint32_t query)
{
	if (active.query_pool != query_pool)
		return;
	vk::CommandBuffer::endQuery(query_pool, query);
	active.query_pool = VK_NULL_HANDLE;
}

void CommandBuffer::beginRenderPass(const vk::RenderPassBeginInfo& render_pass_begin_info, vk::SubpassContents contents)
{
	if (active.render_pass == render_pass_begin_info.renderPass)
		return;
	vk::CommandBuffer::beginRenderPass(render_pass_begin_info, contents);
	active.render_pass = render_pass_begin_info.renderPass;
	active.framebuffer = render_pass_begin_info.framebuffer;
	active.render_area = render_pass_begin_info.renderArea;
	active.subpass = 0;
}

void CommandBuffer::nextSubpass(vk::SubpassContents contents)
{
	SK_ASSERT(active.render_pass != vk::RenderPass(VK_NULL_HANDLE), "Can't call nextSubpass() when there is no active render pass in this command buffer");
	vk::CommandBuffer::nextSubpass(contents);
	++active.subpass;
}

void CommandBuffer::endRenderPass()
{
	if (active.render_pass == vk::RenderPass(VK_NULL_HANDLE))
		return;
	vk::CommandBuffer::endRenderPass();
	active.render_pass = VK_NULL_HANDLE;
}

void CommandBuffer::executeCommands(const std::vector<vk::CommandBuffer>& command_buffers)
{
	vk::CommandBuffer::executeCommands(command_buffers);
}

void CommandBuffer::setDeviceMask(uint32_t device_mask)
{
	if (active.device_mask == device_mask)
		return;
	vk::CommandBuffer::setDeviceMask(device_mask);
	active.device_mask = device_mask;
}

void CommandBuffer::setCullMode(vk::CullModeFlags cull_mode)
{
	if (active.cull_mode == cull_mode)
		return;
	vk::CommandBuffer::setCullMode(cull_mode);
	active.cull_mode = cull_mode;
}

void CommandBuffer::setFrontFace(vk::FrontFace front_face)
{
	if (active.front_face == front_face)
		return;
	vk::CommandBuffer::setFrontFace(front_face);
	active.front_face = front_face;
}

void CommandBuffer::setPrimitiveTopology(vk::PrimitiveTopology primitive_topology)
{
	if (active.primitive_topology == primitive_topology)
		return;
	vk::CommandBuffer::setPrimitiveTopology(primitive_topology);
	active.primitive_topology = primitive_topology;
}

void CommandBuffer::setViewportWithCount(vk::Viewport viewport)
{
	if (active.viewport == viewport)
		return;
	vk::CommandBuffer::setViewportWithCount({ viewport });
	active.viewport = viewport;
}

void CommandBuffer::setScissorWithCount(vk::Rect2D scissor)
{
	if (active.scissor == scissor)
		return;
	vk::CommandBuffer::setScissorWithCount({ scissor });
	active.scissor = scissor;
}

void CommandBuffer::setDepthTestEnable(vk::Bool32 depth_test_enable)
{
	if (active.depth_test_enable == depth_test_enable)
		return;
	vk::CommandBuffer::setDepthTestEnable(depth_test_enable);
	active.depth_test_enable = depth_test_enable;
}

void CommandBuffer::setDepthWriteEnable(vk::Bool32 depth_write_enable)
{
	if (active.depth_write_enable == depth_write_enable)
		return;
	vk::CommandBuffer::setDepthWriteEnable(depth_write_enable);
	active.depth_write_enable = depth_write_enable;
}

void CommandBuffer::setDepthCompareOp(vk::CompareOp depth_compare_op)
{
	if (active.depth_compare_op == depth_compare_op)
		return;
	vk::CommandBuffer::setDepthCompareOp(depth_compare_op);
	active.depth_compare_op = depth_compare_op;
}

void CommandBuffer::setDepthBoundsTestEnable(vk::Bool32 depth_bound_test_enable)
{
	if (active.depth_bound_test_enable == depth_bound_test_enable)
		return;
	vk::CommandBuffer::setDepthBoundsTestEnable(depth_bound_test_enable);
	active.depth_bound_test_enable = depth_bound_test_enable;
}

void CommandBuffer::setStencilTestEnable(vk::Bool32 stencil_test_enable)
{
	if (active.stencil_test_enable == stencil_test_enable)
		return;
	vk::CommandBuffer::setStencilTestEnable(stencil_test_enable);
	active.stencil_test_enable = stencil_test_enable;
}

void CommandBuffer::setStencilOp(vk::StencilFaceFlags face_mask, vk::StencilOp fail_op, vk::StencilOp pass_op, vk::StencilOp depth_fail_op, vk::CompareOp compare_op)
{
	if (active.stencil_op_face_mask == face_mask &&
		active.stencil_op_fail_op == fail_op &&
		active.stencil_op_pass_op == pass_op &&
		active.stencil_op_depth_fail_op == depth_fail_op &&
		active.stencil_op_compare_op == compare_op)
		return;
	vk::CommandBuffer::setStencilOp(face_mask, fail_op, pass_op, depth_fail_op, compare_op);
	active.stencil_op_face_mask = face_mask;
	active.stencil_op_fail_op = fail_op;
	active.stencil_op_pass_op = pass_op;
	active.stencil_op_depth_fail_op = depth_fail_op;
	active.stencil_op_compare_op = compare_op;
}

void CommandBuffer::setRasterizerDiscardEnable(vk::Bool32 rasterizer_discard_enable)
{
	if (active.rasterizer_discard_enable == rasterizer_discard_enable)
		return;
	vk::CommandBuffer::setRasterizerDiscardEnable(rasterizer_discard_enable);
	active.rasterizer_discard_enable = rasterizer_discard_enable;
}

void CommandBuffer::setDepthBiasEnable(vk::Bool32 depth_bias_enable)
{
	if (active.depth_bias_enable == depth_bias_enable)
		return;
	vk::CommandBuffer::setDepthBiasEnable(depth_bias_enable);
	active.depth_bias_enable = depth_bias_enable;
}

void CommandBuffer::setPrimitiveRestartEnable(vk::Bool32 primitive_restart_enable)
{
	if (active.primitive_restart_enable == primitive_restart_enable)
		return;
	vk::CommandBuffer::setPrimitiveRestartEnable(primitive_restart_enable);
	active.primitive_restart_enable = primitive_restart_enable;
}

void CommandBuffer::submit(const CommandBufferSubmitInfo& info)
{
	end();
	if (!recorded)
		return;

	vk::SubmitInfo submit_info{};
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = this;
	
	submit_info.pWaitDstStageMask = info.wait_stages;
	submit_info.waitSemaphoreCount = info.wait_semaphores.size();
	submit_info.pWaitSemaphores = info.wait_semaphores.data();

	submit_info.signalSemaphoreCount = info.signal_semaphores.size();
	submit_info.pSignalSemaphores = info.signal_semaphores.data();

	if ((const VkFence&)info.fence != VK_NULL_HANDLE)
		Graphics::logical_device->resetFences({ info.fence });
	
	getQueue().submit({ submit_info }, info.fence);
}

void CommandBuffer::submitIdle()
{
	end();
	if (!recorded)
		return;

	vk::SubmitInfo submit_info{};
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = this;

	vk::Fence fence = Graphics::logical_device->createFence({ vk::FenceCreateFlagBits::eSignaled });
	getQueue().submit({ submit_info }, fence);	
	Graphics::logical_device->waitForFences({ fence });	
	Graphics::logical_device->destroyFence(fence);
}

vk::Queue CommandBuffer::getQueue() const
{
	switch (queue_type) 
	{
		case vk::QueueFlagBits::eGraphics: return Graphics::logical_device->getGraphicsQueue(); 
		case vk::QueueFlagBits::eTransfer: return Graphics::logical_device->getTransferQueue();
		case vk::QueueFlagBits::eCompute: return Graphics::logical_device->getComputeQueue();
	}

	return Graphics::logical_device->getGraphicsQueue(); //Or nullptr
}
