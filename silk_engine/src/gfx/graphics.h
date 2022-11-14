#pragma once

#include "buffers/command_buffer.h"

enum class APIVersion
{
	VULKAN_1_0,
	VULKAN_1_1,
	VULKAN_1_2,
	VULKAN_1_3,
};

class CommandQueue;
class WindowResizeEvent;
class Instance;
class PhysicalDevice;
class LogicalDevice;
class Allocator;
class PipelineCache;
class DescriptorPool;
class DescriptorSet;
class Application;

class Graphics
{
public:
	static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
	static constexpr APIVersion API_VERSION = APIVersion::VULKAN_1_3;

public:
	static inline struct Statistics
	{
		size_t instance_batches = 0;
		size_t instances = 0;
		size_t vertices = 0;
		size_t indices = 0;
	} stats{};

public:
	static void init(const Application& app);
	static void destroy();
	static void update();

	static void submit(std::function<void(CommandBuffer&)>&& command);
	static void execute(); 
	static void execute(const CommandBuffer::SubmitInfo& submit_info);

	static void screenshot(std::string_view file);
	static void vulkanAssert(VkResult result);
	static std::string stringifyResult(VkResult result);
	static uint32_t apiVersion(APIVersion api_version);

public:
	static inline Instance* instance = nullptr;
	static inline PhysicalDevice* physical_device = nullptr;
	static inline LogicalDevice* logical_device = nullptr;
	static inline Allocator* allocator = nullptr;
	static inline CommandQueue* command_queue = nullptr;
	static inline PipelineCache* pipeline_cache = nullptr;
};