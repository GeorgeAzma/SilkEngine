#pragma once

#include "command_queue.h"

class WindowResizeEvent;
class DebugMessenger;
class Instance;
class PhysicalDevice;
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

	static void record(std::function<void(CommandBuffer&)>&& command);
	static void submit(const Fence* fence = nullptr, const std::vector<VkPipelineStageFlags>& wait_stages = {}, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {});
	static void execute(); 

	static void recordCompute(std::function<void(CommandBuffer&)>&& command);
	static void submitCompute(const Fence* fence = nullptr, const std::vector<VkPipelineStageFlags>& wait_stages = {}, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {});
	static void executeCompute();

	static void recordTransfer(std::function<void(CommandBuffer&)>&& command);
	static void submitTransfer(const Fence* fence = nullptr, const std::vector<VkPipelineStageFlags>& wait_stages = {}, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {});
	static void executeTransfer();

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