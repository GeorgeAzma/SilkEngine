#pragma once

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

class Graphics
{
public:
	struct GlobalUniformData
	{
		glm::mat4 projection_view;
	};
	static struct Statistics
	{
		size_t batches = 0;
		size_t instances = 0;
	} stats;
public:
	static void init();

	static void beginFrame();
	static void beginRenderPass();
	static void endFrame();
	static void endRenderPass();

	static void cleanup();

	static void vulkanAssert(VkResult result);

public:
	static constexpr size_t MAX_INSTANCES = 1024 * 1024; //1mb * sizeof(InstanceData)
	static constexpr size_t MAX_BATCHES = 1024;
	static constexpr uint32_t API_VERSION = VK_API_VERSION_1_2;

	static inline Instance* instance = nullptr;
	static inline Surface* surface = nullptr;
	static inline PhysicalDevice* physical_device = nullptr;
	static inline LogicalDevice* logical_device = nullptr;
	static inline Allocator* allocator = nullptr;
	static inline CommandPool* command_pool = nullptr;
	static inline SwapChain* swap_chain = nullptr;
	static inline DescriptorPool* descriptor_pool = nullptr;
	static inline UniformBuffer* global_uniform = nullptr;

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