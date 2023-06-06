#pragma once

#include "command_queue.h"
#include "devices/physical_device.h"

class WindowResizeEvent;
class DebugMessenger;
class Instance;
class LogicalDevice;
class Allocator;
class PipelineCache;
class DescriptorPool;
class DescriptorSet;
class Application;
class RenderGraph;

class RenderContext
{
public:
	static constexpr size_t MAX_FRAMES = 2;

public:
	static void init(std::string_view app_name);
	static void destroy();
	static void update();
	static void nextFrame() { frame = (frame + 1) % MAX_FRAMES; }

	static CommandBuffer& getNewCommandBuffer(bool begin = true)
	{
		return getCommandQueues()[frame]->getNewCommandBuffer(begin);
	}
	static CommandBuffer& getCommandBuffer(bool begin = true)
	{
		return getCommandQueues()[frame]->getCommandBuffer(begin);
	}
	static const shared<CommandBuffer>& submit(const std::vector<PipelineStage>& wait_stages = {}, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {})
	{
		return getCommandQueues()[frame]->submit(wait_stages, wait_semaphores, signal_semaphores);
	}
	static void execute(const std::vector<PipelineStage>& wait_stages = {}, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {})
	{
		getCommandQueues()[frame]->execute(wait_stages, wait_semaphores, signal_semaphores);
	}

	static CommandBuffer& getNewComputeCommandBuffer(bool begin = true)
	{
		return getComputeCommandQueues()[frame]->getNewCommandBuffer(begin);
	}
	static CommandBuffer& getComputeCommandBuffer(bool begin = true)
	{
		return getComputeCommandQueues()[frame]->getCommandBuffer(begin);
	}
	static const shared<CommandBuffer>& submitCompute(const std::vector<PipelineStage>& wait_stages = {}, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {})
	{
		return getComputeCommandQueues()[frame]->submit(wait_stages, wait_semaphores, signal_semaphores);
	}
	static void executeCompute(const std::vector<PipelineStage>& wait_stages = {}, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {})
	{
		getComputeCommandQueues()[frame]->execute(wait_stages, wait_semaphores, signal_semaphores);
	}

	static CommandBuffer& getNewTransferCommandBuffer(bool begin = true)
	{
		return getTransferCommandQueues()[frame]->getNewCommandBuffer(begin);
	}
	static CommandBuffer& getTransferCommandBuffer(bool begin = true)
	{
		return getTransferCommandQueues()[frame]->getCommandBuffer(begin);
	}
	static const shared<CommandBuffer>& submitTransfer(const std::vector<PipelineStage>& wait_stages = {}, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {})
	{
		return getTransferCommandQueues()[frame]->submit(wait_stages, wait_semaphores, signal_semaphores);
	}
	static void executeTransfer(const std::vector<PipelineStage>& wait_stages = {}, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {})
	{
		getTransferCommandQueues()[frame]->execute(wait_stages, wait_semaphores, signal_semaphores);
	}

	static void screenshot(const fs::path& file);
	static void vulkanAssert(VkResult result);
	static std::string stringifyResult(VkResult result);

	static void setRenderGraph(const shared<RenderGraph>& render_graph) { RenderContext::render_graph = render_graph; }

	static const Instance& getInstance() { return *instance; }
	static const LogicalDevice& getLogicalDevice() { return *logical_device; }
	static const PhysicalDevice& getPhysicalDevice();
	static const Allocator& getAllocator() { return *allocator; }
	static const PipelineCache& getPipelineCache() { return *pipeline_cache; }
	static const RenderGraph& getRenderGraph() { return *render_graph; }

private:
	static const std::vector<shared<CommandQueue>>& getCommandQueues();
	static const std::vector<shared<CommandQueue>>& getComputeCommandQueues();
	static const std::vector<shared<CommandQueue>>& getTransferCommandQueues();

private:
	static inline VkAllocationCallbacks allocation_callbacks = {}; // TODO: add this everywhere?
	static inline Instance* instance = nullptr;
	static inline PhysicalDevice* physical_device = nullptr;
	static inline LogicalDevice* logical_device = nullptr;
	static inline Allocator* allocator = nullptr;
	static inline std::unordered_map<std::thread::id, std::vector<shared<CommandQueue>>> command_queues{};
	static inline std::unordered_map<std::thread::id, std::vector<shared<CommandQueue>>> compute_command_queues{};
	static inline std::unordered_map<std::thread::id, std::vector<shared<CommandQueue>>> transfer_command_queues{};
	static inline PipelineCache* pipeline_cache = nullptr;
	static inline size_t frame = 0;
	static inline shared<RenderGraph> render_graph = nullptr;
};