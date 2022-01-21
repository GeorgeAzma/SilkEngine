#pragma once
#include "instance.h"
#include "window/surface.h"
#include "devices/physical_device.h"
#include "devices/logical_device.h"
#include "window/swap_chain.h"
#include "allocators/command_pool.h"
#include "descriptors/descriptor_pool.h"
#include "allocators/allocator.h"
#include "pipeline/graphics_pipeline.h"
#include "pipeline/compute_pipeline.h"


class WindowResizeEvent;

class Graphics
{
public:
	struct GlobalUniformData
	{
		glm::mat4 projection_view;
	};
public:
	static void init();

	static void beginFrame();
	static void endFrame();

	static void cleanup();

	static void vulkanAssert(VkResult result);

public:
	static constexpr uint32_t API_VERSION = VK_API_VERSION_1_2;

	static inline Instance* instance = nullptr;
	static inline Surface* surface = nullptr;
	static inline PhysicalDevice* physical_device = nullptr;
	static inline LogicalDevice* logical_device = nullptr;
	static inline Allocator* allocator = nullptr;
	static inline CommandPool* command_pool = nullptr;
	static inline SwapChain* swap_chain = nullptr;
	static inline DescriptorPool* descriptor_pool = nullptr;

	static inline struct Active
	{
	public:
		VkCommandBuffer command_buffer = VK_NULL_HANDLE;
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
		VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
		VkBuffer vertex_buffer = VK_NULL_HANDLE;
		VkBuffer index_buffer = VK_NULL_HANDLE;
		VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
		VkRenderPass render_pass = VK_NULL_HANDLE;
		uint32_t subpass = 0;
	} active;
private:
	static constexpr std::string stringifyResult(VkResult result);
};