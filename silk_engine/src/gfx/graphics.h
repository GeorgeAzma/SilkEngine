#pragma once

#include "enums.h"
#include "utils/alarm.h"

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
	static constexpr size_t MAX_INSTANCE_BATCHES = 65536;
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
			instance_batches = 0; 
			instances = 0; 
			vertices = 0;
			indices = 0;
		}
	} stats{};

public:
	static void init();
	static void cleanup();
	static void update();

	static void beginFrame();
	static void endFrame();

	static shared<CommandPool> getCommandPool();
	static CommandBuffer& getActiveCommandBuffer();
	static CommandBuffer& getActivePrimaryCommandBuffer();
	static void setActiveCommandBuffer(CommandBuffer* command_buffer);
	static void setActivePrimaryCommandBuffer(CommandBuffer* command_buffer);

	static void screenshot(const std::string& file);

	static void vulkanAssert(vk::Result result);

public:
	static inline Instance* instance = nullptr;
	static inline Surface* surface = nullptr;
	static inline PhysicalDevice* physical_device = nullptr;
	static inline LogicalDevice* logical_device = nullptr;
	static inline Allocator* allocator = nullptr;
	static inline std::unordered_map<std::thread::id, shared<CommandPool>> command_pools;
	static inline SwapChain* swap_chain = nullptr;
	static inline DescriptorPool* descriptor_pool = nullptr;
	static inline Alarm command_pool_purge_alarm = Alarm(5s);
	static inline CommandBuffer* command_buffer = nullptr;
	static inline VkFence previous_frame_finished = VK_NULL_HANDLE;
	static inline VkSemaphore swap_chain_image_available = VK_NULL_HANDLE;
	static inline VkSemaphore render_finished = VK_NULL_HANDLE;

private:
	static inline std::mutex active_command_buffer_mutex;
	static inline std::mutex active_primary_command_buffer_mutex;
	static inline std::unordered_map<std::thread::id, CommandBuffer*> command_buffers;
	static inline std::unordered_map<std::thread::id, CommandBuffer*> primary_command_buffers;
};