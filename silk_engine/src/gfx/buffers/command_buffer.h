#pragma once

class Queue;
class Fence;
class CommandPool;

class CommandBuffer : NonCopyable
{
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
	CommandBuffer(CommandPool& command_pool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, VkQueueFlagBits queue_type = VK_QUEUE_GRAPHICS_BIT);
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
	void pipelineBarrier(VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkDependencyFlags dependency, const std::vector<VkBufferMemoryBarrier>& buffer_barriers, const std::vector<VkMemoryBarrier>& memory_barriers = {}) const;
	void pipelineBarrier(VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkDependencyFlags dependency, const std::vector<VkImageMemoryBarrier>& image_barriers, const std::vector<VkMemoryBarrier>& memory_barriers = {}) const;
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

	void submit(const SubmitInfo& info = {});
	void submitImmidiatly();
	void reset(bool free = false);

	bool isPrimary() const { return is_primary; }
	State getState() const { return state; }

	operator const VkCommandBuffer& () const { return command_buffer; }

private:
	VkCommandBuffer command_buffer;
	VkCommandBufferLevel level;
	VkQueueFlagBits queue_type;
	CommandPool& pool;
	State state = State::INITIAL;
	bool is_primary = false;

	struct Active
	{
		uint32_t subpass = 0;
		std::vector<BoundDescriptorSet> descriptor_sets{};
		VkDeviceSize index_buffer_offset = 0;
		std::vector<OffsetVertexBuffer> vertex_buffers{};
		std::optional<VkPipelineBindPoint> pipeline_bind_point = VkPipelineBindPoint(VK_PIPELINE_BIND_POINT_MAX_ENUM);
		std::optional<VkPipeline> pipeline = {};
		std::optional<VkPipelineLayout> pipeline_layout = {};
		std::optional<VkRenderPass> render_pass = {};
		std::optional<VkQueryPool> query_pool = {};
		std::optional<VkFramebuffer> framebuffer = {};
		std::optional<VkBuffer> index_buffer = {};
		std::optional<VkViewport> viewport = {};
		std::optional<VkRect2D> scissor = {};
		std::optional<float> line_width = {};
		std::optional<float> depth_bias_constant = {};
		std::optional<float> depth_bias_clamp = {};
		std::optional<float> depth_bias_slope = {};
		std::optional<std::array<float, 4>> blend_constants = {};
		std::optional<float> min_depth_bound = {};
		std::optional<float> max_depth_bound = {};
		std::optional<VkStencilFaceFlags> stencil_compare_mask_face_mask = {};
		std::optional<uint32_t> stencil_compare_mask_compare_mask = {};
		std::optional<uint32_t> device_mask = {};
		std::optional<VkCullModeFlags> cull_mode = {};
		std::optional<VkFrontFace> front_face = {};
		std::optional<VkPrimitiveTopology> primitive_topology = {};
		std::optional<VkBool32> depth_test_enable = {};
		std::optional<VkBool32> depth_write_enable = {};
		std::optional<VkCompareOp> depth_compare_op = {};
		std::optional<VkBool32> depth_bound_test_enable = {};
		std::optional<VkBool32> stencil_test_enable = {};
		std::optional<VkStencilFaceFlags> stencil_op_face_mask = {};
		std::optional<VkStencilOp> stencil_op_fail_op = {};
		std::optional<VkStencilOp> stencil_op_pass_op = {};
		std::optional<VkStencilOp> stencil_op_depth_fail_op = {};
		std::optional<VkCompareOp> stencil_op_compare_op = {};
		std::optional<VkBool32> rasterizer_discard_enable = {};
		std::optional<VkBool32> depth_bias_enable = {};
		std::optional<VkBool32> primitive_restart_enable = {};
		std::optional<VkRect2D> render_area = {};
	} active;

public:
	const Active& getActive() const { return active; }
};