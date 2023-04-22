#pragma once

class Queue;
class Fence;
class CommandPool;

class CommandBuffer : NonCopyable
{
	template<std::unsigned_integral T>
	static constexpr T None() 
	{ 
		return std::numeric_limits<T>::max(); 
	}

	template<typename T>
	static constexpr T None()
	{
		return std::numeric_limits<T>::lowest();
	}

public:
	struct SubmitInfo
	{
		Fence* fence = nullptr;
		std::vector<VkSemaphore> wait_semaphores = {};
		std::vector<VkSemaphore> signal_semaphores = {};
		VkPipelineStageFlags* wait_stages = nullptr;
	};

	enum class State
	{
		INITIAL,
		RECORDING,
		EXECUTABLE,
		PENDING,
		INVALID
	};

	struct BoundDescriptorSet
	{
		VkDescriptorSet set = nullptr;
		std::vector<uint32_t> dynamic_offsets;
	};

	struct OffsetVertexBuffer
	{
		VkBuffer vertex_buffer = nullptr;
		VkDeviceSize offset = 0;
	};

public:
	CommandBuffer(CommandPool& command_pool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	~CommandBuffer();

	void begin(VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	void end();
	void beginQuery(VkQueryPool query_pool, uint32_t query, VkQueryControlFlags flags);
	void endQuery(VkQueryPool query_pool, uint32_t query);
	void beginRenderPass(const VkRenderPassBeginInfo& render_pass_begin_info, VkSubpassContents contents);
	void nextSubpass(VkSubpassContents contents);
	void endRenderPass();

	void bindPipeline(VkPipelineBindPoint bind_point, VkPipeline pipeline, VkPipelineLayout layout);
	void bindDescriptorSets(uint32_t first, const std::vector<VkDescriptorSet>& sets, const std::vector<uint32_t>& dynamic_offsets = {});
	void bindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType index_type);
	void bindVertexBuffers(uint32_t first, const std::vector<VkBuffer>& buffers, const std::vector<VkDeviceSize>& offsets = {});

	void setViewport(VkViewport viewport);
	void setScissor(VkRect2D scissor);
	void setLineWidth(float width);
	void setDepthBias(float constant, float clamp, float slope);
	void setBlendConstants(const float blend_constants[4]);
	void setDepthBounds(float min, float max);
	void setStencilCompareMask(VkStencilFaceFlags face_mask, uint32_t compare_mask);
	void setDeviceMask(uint32_t device_mask);
	void setCullMode(VkCullModeFlags cull_mode);
	void setFrontFace(VkFrontFace front_face);
	void setPrimitiveTopology(VkPrimitiveTopology primitive_topology);
	void setViewportWithCount(VkViewport viewport);
	void setScissorWithCount(VkRect2D scissor);
	void setDepthTestEnable(VkBool32 depth_test_enable);
	void setDepthWriteEnable(VkBool32 depth_write_enable);
	void setDepthCompareOp(VkCompareOp depth_compare_op);
	void setDepthBoundsTestEnable(VkBool32 depth_bound_test_enable);
	void setStencilTestEnable(VkBool32 stencil_test_enable);
	void setStencilOp(VkStencilFaceFlags face_mask, VkStencilOp fail_op, VkStencilOp pass_op, VkStencilOp depth_fail_op, VkCompareOp compare_op);
	void setRasterizerDiscardEnable(VkBool32 rasterizer_discard_enable);
	void setDepthBiasEnable(VkBool32 depth_bias_enable);
	void setPrimitiveRestartEnable(VkBool32 primitive_restart_enable);
	
	void executeCommands(const std::vector<VkCommandBuffer>& command_buffers) const;
	void copyBuffer(VkBuffer source, VkBuffer destination, const std::vector<VkBufferCopy>& copy_regions) const;
	void pipelineBarrier(VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkDependencyFlags dependency, const std::vector<VkMemoryBarrier>& memory_barriers, const std::vector<VkBufferMemoryBarrier>& buffer_barriers, const std::vector<VkImageMemoryBarrier>& image_barriers) const;
	void pipelineBarrier(VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, const std::vector<VkBufferMemoryBarrier>& buffer_barriers, const std::vector<VkMemoryBarrier>& memory_barriers = {}) const;
	void pipelineBarrier(VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, const std::vector<VkImageMemoryBarrier>& image_barriers, const std::vector<VkMemoryBarrier>& memory_barriers = {}) const;
	void blitImage(VkImage source, VkImageLayout source_layout, VkImage destination, VkImageLayout destination_layout, const std::vector<VkImageBlit>& blit_regions, VkFilter filter = VK_FILTER_LINEAR) const;
	void copyBufferToImage(VkBuffer buffer, VkImage image, VkImageLayout image_layout, const std::vector<VkBufferImageCopy>& copy_regions) const;
	void copyImageToBuffer(VkImage image, VkImageLayout image_layout, VkBuffer buffer, const std::vector<VkBufferImageCopy>& copy_regions) const;
	void copyImage(VkImage source, VkImageLayout source_layout, VkImage destination, VkImageLayout destination_layout, const std::vector<VkImageCopy>& copy_regions) const;
	void dispatch(uint32_t global_invocation_count_x, uint32_t global_invocation_count_y, uint32_t global_invocation_count_z) const;
	void pushConstants(VkPipelineStageFlags stages, uint32_t offset, uint32_t size, const void* data) const;
	void draw(uint32_t vertices, uint32_t instances = 1, uint32_t first_vertex = 0, uint32_t first_instance = 0) const;
	void drawIndexed(uint32_t indices, uint32_t instances = 1, uint32_t first_index = 0, uint32_t vertex_offset = 0, uint32_t first_instance = 0) const;
	void drawIndirect(VkBuffer indirect_buffer, uint32_t offset, uint32_t draw_count, uint32_t stride) const;
	void drawIndexedIndirect(VkBuffer indirect_buffer, uint32_t offset, uint32_t draw_count, uint32_t stride) const;

	void submit(const SubmitInfo& info = {}, VkQueueFlagBits queue_type = VK_QUEUE_GRAPHICS_BIT);
	void submitImmidiatly(VkQueueFlagBits queue_type = VK_QUEUE_GRAPHICS_BIT);
	void reset(bool free = false);

	bool isPrimary() const { return is_primary; }
	State getState() const { return state; }

	operator const VkCommandBuffer& () const { return command_buffer; }

private:
	VkCommandBuffer command_buffer;
	VkCommandBufferLevel level;
	CommandPool& pool;
	State state = State::INITIAL;
	bool is_primary = false;

	struct Active
	{
		uint32_t subpass = None<uint32_t>();
		std::vector<BoundDescriptorSet> descriptor_sets{};
		VkDeviceSize index_buffer_offset = None<VkDeviceSize>();
		std::vector<OffsetVertexBuffer> vertex_buffers{};
		VkPipelineBindPoint pipeline_bind_point = None<VkPipelineBindPoint>();
		VkPipeline pipeline = nullptr;
		VkPipelineLayout pipeline_layout = nullptr;
		VkRenderPass render_pass = nullptr;
		VkQueryPool query_pool = nullptr;
		VkFramebuffer framebuffer = nullptr;
		VkBuffer index_buffer = nullptr;
		VkViewport viewport = { None<float>(), None<float>(), None<float>(), None<float>(), None<float>(), None<float>() };
		VkRect2D scissor = { { None<int32_t>(), None<int32_t>() }, { None<uint32_t>(), None<uint32_t>() } };
		float line_width = None<float>();
		float depth_bias_constant = None<float>();
		float depth_bias_clamp = None<float>();
		float depth_bias_slope = None<float>();
		std::array<float, 4> blend_constants = { None<float>(), None<float>(), None<float>(), None<float>() };
		float min_depth_bound = None<float>();
		float max_depth_bound = None<float>();
		VkStencilFaceFlags stencil_compare_mask_face_mask = None<VkStencilFaceFlags>();
		uint32_t stencil_compare_mask_compare_mask = None<uint32_t>();
		uint32_t device_mask = None<uint32_t>();
		VkCullModeFlags cull_mode = None<VkCullModeFlags>();
		VkFrontFace front_face = None<VkFrontFace>();
		VkPrimitiveTopology primitive_topology = None<VkPrimitiveTopology>();
		VkBool32 depth_test_enable = None<VkBool32>();
		VkBool32 depth_write_enable = None<VkBool32>();
		VkCompareOp depth_compare_op = None<VkCompareOp>();
		VkBool32 depth_bound_test_enable = None<VkBool32>();
		VkBool32 stencil_test_enable = None<VkBool32>();
		VkStencilFaceFlags stencil_op_face_mask = {};
		VkStencilOp stencil_op_fail_op = None<VkStencilOp>();
		VkStencilOp stencil_op_pass_op = None<VkStencilOp>();
		VkStencilOp stencil_op_depth_fail_op = None<VkStencilOp>();
		VkCompareOp stencil_op_compare_op = None<VkCompareOp>();
		VkBool32 rasterizer_discard_enable = None<VkBool32>();
		VkBool32 depth_bias_enable = None<VkBool32>();
		VkBool32 primitive_restart_enable = None<VkBool32>();
		VkRect2D render_area = { { None<int32_t>(), None<int32_t>() }, { None<uint32_t>(), None<uint32_t>() } };
	} active;

public:
	const Active& getActive() const { return active; }
};