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

class RenderContext
{
public:
	static void init(std::string_view app_name);
	static void destroy();
	static void update();

	template <Command Func>
	static void record(Func&& command)
	{
		command_queues[frame]->record(std::forward<Func>(command));
	}
	static void submit(const Fence* fence = nullptr, const std::vector<VkPipelineStageFlags>& wait_stages = {}, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {}) 
	{
		command_queues[frame]->submit(fence, wait_stages, wait_semaphores, signal_semaphores);
	}
	static void execute()
	{
		command_queues[frame]->execute();
	}

	template <Command Func>
	static void recordCompute(Func&& command)
	{
		((physical_device->getComputeQueue() > -1) ? compute_command_queues : command_queues)[frame]->record(std::forward<Func>(command));
	}
	static void submitCompute(const Fence* fence = nullptr, const std::vector<VkPipelineStageFlags>& wait_stages = {}, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {}) 
	{
		((physical_device->getComputeQueue() > -1) ? compute_command_queues : command_queues)[frame]->submit(fence, wait_stages, wait_semaphores, signal_semaphores);
	}
	static void executeCompute()
	{
		((physical_device->getComputeQueue() > -1) ? compute_command_queues : command_queues)[frame]->execute();
	}

	template <Command Func>
	static void recordTransfer(Func&& command)
	{
		((physical_device->getTransferQueue() > -1) ? transfer_command_queues : command_queues)[frame]->record(std::forward<Func>(command));
	}
	static void submitTransfer(const Fence* fence = nullptr, const std::vector<VkPipelineStageFlags>& wait_stages = {}, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {}) 
	{
		((physical_device->getTransferQueue() > -1) ? transfer_command_queues : command_queues)[frame]->submit(fence, wait_stages, wait_semaphores, signal_semaphores);
	}
	static void executeTransfer()
	{
		((physical_device->getTransferQueue() > -1) ? transfer_command_queues : command_queues)[frame]->execute();
	}

	static void screenshot(const path& file);
	static void vulkanAssert(VkResult result);
	static std::string stringifyResult(VkResult result);

	static const Instance& getInstance() { return *instance; }
	static const LogicalDevice& getLogicalDevice() { return *logical_device; }
	static const PhysicalDevice& getPhysicalDevice();
	static const Allocator& getAllocator() { return *allocator; }
	static const PipelineCache& getPipelineCache() { return *pipeline_cache; }

private:
	static inline Instance* instance = nullptr;
	static inline PhysicalDevice* physical_device = nullptr;
	static inline LogicalDevice* logical_device = nullptr;
	static inline Allocator* allocator = nullptr;
	static inline std::vector<shared<CommandQueue>> command_queues{};
	static inline std::vector<shared<CommandQueue>> compute_command_queues{};
	static inline std::vector<shared<CommandQueue>> transfer_command_queues{};
	static inline PipelineCache* pipeline_cache = nullptr;
	static inline size_t frame = 0;
};