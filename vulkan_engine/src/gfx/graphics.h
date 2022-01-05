#pragma once
#include "instance.h"
#include "surface.h"
#include "physical_device.h"
#include "logical_device.h"
#include "swap_chain.h"
#include "render_pass.h"
#include "graphics_pipeline.h"
#include "command_pool.h"

class Graphics
{
	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
public:
	static void init(GLFWwindow* window);
	static void update();
	static void cleanup();

	static constexpr void vulkanAssert(VkResult result);

	static inline Instance* instance = nullptr;
	static inline Surface* surface = nullptr;
	static inline PhysicalDevice* physical_device = nullptr;
	static inline LogicalDevice* logical_device = nullptr;
	static inline CommandPool* command_pool = nullptr;
	static inline SwapChain* swap_chain = nullptr;
	static inline RenderPass* render_pass = nullptr;
	static inline GraphicsPipeline* graphics_pipeline = nullptr;
	static inline std::vector<VkCommandBuffer> command_buffers = {};

private:
	static constexpr std::string stringifyResult(VkResult result);

private:
	static inline VkSemaphore image_available_semaphore;
	static inline VkSemaphore render_finished_semaphore;
};