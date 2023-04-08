#pragma once

#include "buffers/command_buffer.h"

class CommandQueue;
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

	static void submit(std::function<void(CommandBuffer&)>&& command);
	static void execute(); 
	static void execute(const CommandBuffer::SubmitInfo& submit_info);

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
	static inline CommandQueue* command_queue = nullptr;
	static inline PipelineCache* pipeline_cache = nullptr;
};