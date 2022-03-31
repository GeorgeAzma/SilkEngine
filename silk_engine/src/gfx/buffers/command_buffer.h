#pragma once

#include "gfx/allocators/command_pool.h"

struct CommandBufferSubmitInfo
{
	vk::Fence fence = VK_NULL_HANDLE;
	std::vector<vk::Semaphore> wait_semaphores = {};
	std::vector<vk::Semaphore> signal_semaphores = {};
	vk::PipelineStageFlags* wait_stages = nullptr;
};

class CommandBuffer : public vk::CommandBuffer, NonCopyable
{
	struct BoundDescriptorSet
	{
		vk::DescriptorSet set = VK_NULL_HANDLE;
		std::vector<uint32_t> dynamic_offsets;
	};
	struct OffsetVertexBuffer
	{
		vk::Buffer vertex_buffer = VK_NULL_HANDLE;
		vk::DeviceSize offset = 0;
	};
public:
	CommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary, vk::QueueFlagBits queue_type = vk::QueueFlagBits::eGraphics);
	~CommandBuffer();

	void begin(vk::CommandBufferUsageFlags usage = vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	void end();

	void bindPipeline(vk::PipelineBindPoint bind_point, vk::Pipeline pipeline, vk::PipelineLayout layout);
	void setViewport(vk::Viewport viewport);
	void setScissor(vk::Rect2D scissor);
	void setLineWidth(float width);
	void setDepthBias(float constant, float clamp, float slope);
	void setBlendConstants(const float blend_constants[4]);
	void setDepthBounds(float min, float max);
	void setStencilCompareMask(vk::StencilFaceFlags face_mask, uint32_t compare_mask);
	void bindDescriptorSets(uint32_t first, const std::vector<vk::DescriptorSet>& sets, const std::vector<uint32_t>& dynamic_offsets = {});
	void bindIndexBuffer(vk::Buffer buffer, vk::DeviceSize offset, vk::IndexType index_type);
	void bindVertexBuffers(uint32_t first, const std::vector<vk::Buffer>& buffers, const std::vector<vk::DeviceSize>& offsets = {});
	void beginQuery(vk::QueryPool query_pool, uint32_t query, vk::QueryControlFlags flags);
	void endQuery(vk::QueryPool query_pool, uint32_t query);
	void beginRenderPass(const vk::RenderPassBeginInfo& render_pass_begin_info, vk::SubpassContents contents);
	void nextSubpass(vk::SubpassContents contents);
	void endRenderPass();
	void executeCommands(const std::vector<vk::CommandBuffer>& command_buffers);
	void setDeviceMask(uint32_t device_mask);
	void setCullMode(vk::CullModeFlags cull_mode);
	void setFrontFace(vk::FrontFace front_face);
	void setPrimitiveTopology(vk::PrimitiveTopology primitive_topology);
	void setViewportWithCount(vk::Viewport viewport);
	void setScissorWithCount(vk::Rect2D scissor);
	void setDepthTestEnable(vk::Bool32 depth_test_enable);
	void setDepthWriteEnable(vk::Bool32 depth_write_enable);
	void setDepthCompareOp(vk::CompareOp depth_compare_op);
	void setDepthBoundsTestEnable(vk::Bool32 depth_bound_test_enable);
	void setStencilTestEnable(vk::Bool32 stencil_test_enable);
	void setStencilOp(vk::StencilFaceFlags face_mask, vk::StencilOp fail_op, vk::StencilOp pass_op, vk::StencilOp depth_fail_op, vk::CompareOp compare_op);
	void setRasterizerDiscardEnable(vk::Bool32 rasterizer_discard_enable);
	void setDepthBiasEnable(vk::Bool32 depth_bias_enable);
	void setPrimitiveRestartEnable(vk::Bool32 primitive_restart_enable);

	void submit(const CommandBufferSubmitInfo& info = {});
	void submitIdle();

	bool wasRecorded() const { return recorded; }
	bool isRecording() const { return running; }
	bool isPrimary() const { return is_primary; }

private:
	vk::CommandBufferLevel level;
	vk::QueueFlagBits queue_type;
	shared<CommandPool> pool;
	bool recorded = false;
	bool running = false;
	bool is_primary = false;

	struct Active
	{
		uint32_t subpass = 0;
		std::vector<BoundDescriptorSet> descriptor_sets{};
		vk::DeviceSize index_buffer_offset = 0;
		std::vector<OffsetVertexBuffer> vertex_buffers{};

		std::optional<vk::PipelineBindPoint> pipeline_bind_point = vk::PipelineBindPoint(VK_PIPELINE_BIND_POINT_MAX_ENUM);
		std::optional<vk::Pipeline> pipeline = {};
		std::optional<vk::PipelineLayout> pipeline_layout = {};
		std::optional<vk::RenderPass> render_pass = {};
		std::optional<vk::QueryPool> query_pool = {};
		std::optional<vk::Framebuffer> framebuffer = {};
		std::optional<vk::Buffer> index_buffer = {};
		std::optional<vk::Viewport> viewport = {};
		std::optional<vk::Rect2D> scissor = {};
		std::optional<float> line_width = {};
		std::optional<float> depth_bias_constant = {};
		std::optional<float> depth_bias_clamp = {};
		std::optional<float> depth_bias_slope = {};
		std::optional<std::array<float, 4>> blend_constants = {};
		std::optional<float> min_depth_bound = {};
		std::optional<float> max_depth_bound = {};
		std::optional<vk::StencilFaceFlags> stencil_compare_mask_face_mask = {};
		std::optional<uint32_t> stencil_compare_mask_compare_mask = {};
		std::optional<uint32_t> device_mask = {};
		std::optional<vk::CullModeFlags> cull_mode = {};
		std::optional<vk::FrontFace> front_face = {};
		std::optional<vk::PrimitiveTopology> primitive_topology = {};
		std::optional<vk::Bool32> depth_test_enable = {};
		std::optional<vk::Bool32> depth_write_enable = {};
		std::optional<vk::CompareOp> depth_compare_op = {};
		std::optional<vk::Bool32> depth_bound_test_enable = {};
		std::optional<vk::Bool32> stencil_test_enable = {};
		std::optional<vk::StencilFaceFlags> stencil_op_face_mask = {};
		std::optional<vk::StencilOp> stencil_op_fail_op = {};
		std::optional<vk::StencilOp> stencil_op_pass_op = {};
		std::optional<vk::StencilOp> stencil_op_depth_fail_op = {};
		std::optional<vk::CompareOp> stencil_op_compare_op = {};
		std::optional<vk::Bool32> rasterizer_discard_enable = {};
		std::optional<vk::Bool32> depth_bias_enable = {};
		std::optional<vk::Bool32> primitive_restart_enable = {};
		std::optional<vk::Rect2D> render_area = {};
	} active;

private:
	vk::Queue getQueue() const;

public:
	const Active& getActive() const { return active; }
};