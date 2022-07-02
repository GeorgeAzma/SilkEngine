#pragma once

#include "utils/alarm.h"

enum class APIVersion
{
	VULKAN_1_0,
	VULKAN_1_1,
	VULKAN_1_2,
};

class WindowResizeEvent;
class Instance;
class Surface;
class PhysicalDevice;
class LogicalDevice;
class Allocator;
class CommandPool;
class CommandBuffer;
class SwapChain;
class DescriptorPool;
class DescriptorSet;

class Graphics
{
public:
	static constexpr size_t MAX_INSTANCE_BATCHES = 8192;
	static constexpr size_t MAX_INSTANCES = 65536;
	static constexpr size_t MAX_IMAGE_SLOTS = 64; //Can be more
	static constexpr APIVersion API_VERSION = APIVersion::VULKAN_1_2;

public:
	static inline struct Statistics
	{
		size_t instance_batches = 0;
		size_t instances = 0;
		size_t vertices = 0;
		size_t indices = 0;

		void reset() 
		{ 
			*this = {};
		}
	} stats{};

public:
	static void init();
	static void destroy();
	static void update();

	static shared<CommandPool> getActiveCommandPool();
	static CommandBuffer& getActiveCommandBuffer();
	static CommandBuffer* getActiveCommandBufferP();
	static CommandBuffer& getActivePrimaryCommandBuffer();
	static void setActiveCommandBuffer(CommandBuffer* command_buffer);
	static void setActivePrimaryCommandBuffer(CommandBuffer* command_buffer);
	static void screenshot(std::string_view file);
	static void vulkanAssert(VkResult result);
	static std::string stringifyResult(VkResult result);
	static uint32_t apiVersion(APIVersion api_version);

public:
	static inline Instance* instance = nullptr;
	static inline Surface* surface = nullptr;
	static inline PhysicalDevice* physical_device = nullptr;
	static inline LogicalDevice* logical_device = nullptr;
	static inline Allocator* allocator = nullptr;
	static inline std::unordered_map<std::thread::id, shared<CommandPool>> command_pools;
	static inline SwapChain* swap_chain = nullptr;
	static inline Alarm command_pool_purge_alarm = Alarm(5s);

private:
	static inline std::mutex active_command_buffer_mutex;
	static inline std::mutex active_primary_command_buffer_mutex;
	static inline std::mutex active_command_pool_mutex;
	static inline std::unordered_map<std::thread::id, CommandBuffer*> command_buffers;
	static inline std::unordered_map<std::thread::id, CommandBuffer*> primary_command_buffers;
};