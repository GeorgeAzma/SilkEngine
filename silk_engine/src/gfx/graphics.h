#pragma once

#include "scene/light.h"
#include "enums.h"

class WindowResizeEvent;
class Instance;
class Surface;
class PhysicalDevice;
class LogicalDevice;
class Allocator;
class CommandPool;
class SwapChain;
class DescriptorPool;
class UniformBuffer;
class DescriptorSet;

class Graphics
{
public:
	static constexpr size_t MAX_INSTANCE_BATCHES = 65536;
	static constexpr size_t MAX_INSTANCES = 16384;
	static constexpr size_t MAX_IMAGE_SLOTS = 256; //Can be more
	static constexpr size_t MAX_LIGHTS = 64;
	static constexpr APIVersion API_VERSION = APIVersion::VULKAN_1_2;

public:
	struct GlobalUniformData
	{
		glm::mat4 projection_view;
		glm::vec3 camera_position;
		float time;
		glm::vec3 camera_direction;
		float delta_time;
		glm::uvec2 resolution;
		uint32_t frame;
		uint32_t light_count;
		std::array<Light, MAX_LIGHTS> lights;
	};
	static inline struct Statistics
	{
		size_t instance_batches = 0;
	} stats{};
	static inline struct Active
	{
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
		VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;
		VkCommandBuffer command_buffer = VK_NULL_HANDLE;
		VkCommandPool command_pool = VK_NULL_HANDLE;
		VkRenderPass render_pass = VK_NULL_HANDLE;
	} active{};
public:
	static void init();
	static void cleanup();

	static void vulkanAssert(VkResult result);

public:
	static inline Instance* instance = nullptr;
	static inline Surface* surface = nullptr;
	static inline PhysicalDevice* physical_device = nullptr;
	static inline LogicalDevice* logical_device = nullptr;
	static inline Allocator* allocator = nullptr;
	static inline CommandPool* command_pool = nullptr;
	static inline SwapChain* swap_chain = nullptr;
	static inline DescriptorPool* descriptor_pool = nullptr;
	static inline UniformBuffer* global_uniform = nullptr;

private:
	static constexpr std::string stringifyResult(VkResult result);
};