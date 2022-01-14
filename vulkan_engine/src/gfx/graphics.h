#pragma once
#include "instance.h"
#include "surface.h"
#include "physical_device.h"
#include "logical_device.h"
#include "swap_chain.h"
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
#include "scene/vertex.h"

class WindowResizeEvent;

class Graphics
{
public:
	static void init(GLFWwindow* window);
	static void update();

	static void beginFrame();
	static void endFrame();

	static void cleanup();

	static void vulkanAssert(VkResult result);

public:
	static inline Instance* instance = nullptr;
	static inline Surface* surface = nullptr;
	static inline PhysicalDevice* physical_device = nullptr;
	static inline LogicalDevice* logical_device = nullptr;
	static inline CommandPool* command_pool = nullptr;
	static inline SwapChain* swap_chain = nullptr;
	static inline GraphicsPipeline* graphics_pipeline = nullptr;
	static inline GLFWwindow* window = nullptr;
	static inline DescriptorSetLayout* descriptor_set_layout = nullptr;
	static inline DescriptorSet* descriptor_set = nullptr;
	static inline DescriptorPool* descriptor_pool = nullptr;

	//TEMP
	static inline std::vector<std::shared_ptr<UniformBuffer>> uniform_buffers = {};
	static inline VertexBuffer* vertex_buffer = nullptr;
	static inline IndexBuffer* index_buffer = nullptr;
	static inline Image* image = nullptr;
	static inline const std::vector<Vertex> vertices =
	{
		{{-0.5f, -0.5f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f, 1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
		{{0.5f, 0.5f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}
	};
	static inline const std::vector<uint32_t> indices =
	{
		0, 1, 2, 2, 3, 0
	};

private:
	static constexpr std::string stringifyResult(VkResult result);
};