#pragma once
#include "instance.h"
#include "surface.h"
#include "physical_device.h"
#include "logical_device.h"
#include "swap_chain.h"
#include "render_pass.h"
#include "graphics_pipeline.h"
#include "command_pool.h"
#include "buffers/command_buffer.h"
#include "descriptor_set.h"
#include "descriptor_set_layout.h"
#include "descriptor_pool.h"
#include "buffers/uniform_buffer.h"
#include "buffers/vertex_buffer.h"
#include "buffers/index_buffer.h"
#include "image.h"

class WindowResizeEvent;

class Graphics
{
	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	static inline VertexBuffer* vertex_buffer = nullptr;
	static inline IndexBuffer* index_buffer = nullptr;
	static inline Image* image = nullptr;

	static void recordCommandBuffers();

	static void recreateSwapChain();

public:
	static void init(GLFWwindow* window);
	static void update();
	static void cleanup();

	static void vulkanAssert(VkResult result);

public:
	static inline Instance* instance = nullptr;
	static inline Surface* surface = nullptr;
	static inline PhysicalDevice* physical_device = nullptr;
	static inline LogicalDevice* logical_device = nullptr;
	static inline CommandPool* command_pool = nullptr;
	static inline SwapChain* swap_chain = nullptr;
	static inline RenderPass* render_pass = nullptr;
	static inline GraphicsPipeline* graphics_pipeline = nullptr;
	static inline CommandBuffer* command_buffer = nullptr;
	static inline GLFWwindow* window = nullptr;
	static inline DescriptorSetLayout* descriptor_set_layout = nullptr;
	static inline DescriptorSet* descriptor_set = nullptr;
	static inline DescriptorPool* descriptor_pool = nullptr;
	static inline std::vector<std::shared_ptr<UniformBuffer>> uniform_buffers = {};

private:
	static constexpr std::string stringifyResult(VkResult result);
	static void onWindowResize(const WindowResizeEvent& e);

private:
	static inline std::vector<VkSemaphore> image_available_semaphores = std::vector<VkSemaphore>(MAX_FRAMES_IN_FLIGHT);
	static inline std::vector<VkSemaphore> render_finished_semaphores = std::vector<VkSemaphore>(MAX_FRAMES_IN_FLIGHT);
	static inline std::vector<VkFence> in_flight_fences = std::vector<VkFence>(MAX_FRAMES_IN_FLIGHT);
	static inline std::vector<VkFence> images_in_flight = {};

	static inline uint32_t current_frame = 0;
	static inline bool framebuffer_resized = false;
};