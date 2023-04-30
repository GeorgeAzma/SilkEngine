#pragma once

class Queue;
class QueueFamily;
class PhysicalDevice;
class Surface;

class LogicalDevice : NoCopy
{
public:
	LogicalDevice(const PhysicalDevice& physical_device);
	~LogicalDevice();

	bool hasExtension(const char* extension) const;

	void wait() const;
	VkCommandPool createCommandPool(const VkCommandPoolCreateInfo& ci) const;
	VkResult resetCommandPool(VkCommandPool command_pool, VkCommandPoolResetFlags reset_flags = 0) const;
	void destroyCommandPool(VkCommandPool command_pool) const;
	VkQueryPool createQueryPool(const VkQueryPoolCreateInfo& ci) const;
	void destroyQueryPool(VkQueryPool query_pool) const;
	void resetQueryPool(VkQueryPool query_pool, uint32_t first_query = 0, uint32_t query_count = 1) const;
	template<typename T>
	auto getQueryPoolResults(VkQueryPool query_pool, uint32_t first_query, uint32_t query_count, size_t data_size, VkDeviceSize stride, VkQueryResultFlags flags) const
	{
		T data{};
		vkGetQueryPoolResults(logical_device, query_pool, first_query, query_count, data_size, &data, stride, flags);
		return data;
	}
	std::vector<VkCommandBuffer> allocateCommandBuffers(const VkCommandBufferAllocateInfo& allocate_info) const;
	void freeCommandBuffers(VkCommandPool command_pool, const std::vector<VkCommandBuffer>& command_buffers) const;
	void resetFences(const std::vector<VkFence>& fences) const;
	VkFence createFence(bool signaled = false) const;
	void destroyFence(VkFence fence) const;
	VkResult getFenceStatus(VkFence fence) const;
	VkSemaphore createSemaphore(const VkSemaphoreCreateFlags& flags = {}) const;
	void destroySemaphore(VkSemaphore semaphore) const;
	VkResult waitForFences(const std::vector<VkFence>& fences, VkBool32 wait_all = VK_TRUE, uint64_t timeout = UINT64_MAX) const;
	VkResult waitForSemaphores(const std::vector<VkSemaphore>& semaphores, const std::vector<uint64_t>& values, VkBool32 wait_any = false, uint64_t timeout = UINT64_MAX) const;
	VkResult signalSemaphore(const VkSemaphore& semaphore, uint64_t value) const;
	VkFramebuffer createFramebuffer(const VkFramebufferCreateInfo& framebuffer_info) const;
	void destroyFramebuffer(VkFramebuffer framebuffer) const;
	VkDescriptorPool createDescriptorPool(const VkDescriptorPoolCreateInfo& descriptor_pool_info) const;
	void resetDescriptorPool(VkDescriptorPool descriptor_pool, VkDescriptorPoolResetFlags flags = {}) const;
	void destroyDescriptorPool(VkDescriptorPool descriptor_pool) const;
	std::vector<VkDescriptorSet> allocateDescriptorSets(const VkDescriptorSetAllocateInfo& descriptor_set_allocate_info) const;
	VkResult allocateDescriptorSets(const VkDescriptorSetAllocateInfo& alloc_info, VkDescriptorSet& descriptor_set) const;
	void updateDescriptorSets(const std::vector<VkWriteDescriptorSet>& writes) const;
	VkDescriptorSetLayout createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& descriptor_set_layout_create_info) const;
	void destroyDescriptorSetLayout(VkDescriptorSetLayout descriptor_set_layout) const;
	VkSubresourceLayout getImageSubresourceLayout(VkImage image, const VkImageSubresource& image_subresource) const;
	VkImageView createImageView(const VkImageViewCreateInfo& image_view_info) const;
	void destroyImageView(VkImageView image_view) const;
	VkSampler createSampler(const VkSamplerCreateInfo& sampler_info) const;
	void destroySampler(VkSampler sampler) const;
	VkPipelineLayout createPipelineLayout(const VkPipelineLayoutCreateInfo& pipeline_layout_info) const;
	void destroyPipelineLayout(VkPipelineLayout pipeline_layout) const;
	VkPipeline createComputePipeline(VkPipelineCache pipeline_cache, const VkComputePipelineCreateInfo& compute_pipeline_info) const;
	VkPipeline createGraphicsPipeline(VkPipelineCache pipeline_cache, const VkGraphicsPipelineCreateInfo& graphics_pipeline_info) const;
	void destroyPipeline(VkPipeline pipeline) const;
	VkPipelineCache createPipelineCache(const VkPipelineCacheCreateInfo& pipeline_cache_info) const;
	void destroyPipelineCache(VkPipelineCache pipeline_cache) const;
	std::vector<uint8_t> getPipelineCacheData(VkPipelineCache pipeline_cache) const;
	VkRenderPass createRenderPass(const VkRenderPassCreateInfo& render_pass_info) const;
	void destroyRenderPass(VkRenderPass render_pass) const;
	VkShaderModule createShaderModule(const VkShaderModuleCreateInfo& shader_module_info) const;
	void destroyShaderModule(VkShaderModule shader_module) const;
	VkSwapchainKHR createSwapChain(const VkSwapchainCreateInfoKHR& swap_chain_info) const;
	void destroySwapChain(VkSwapchainKHR swap_chain) const;
	VkResult acquireNextImage(VkSwapchainKHR swap_chain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* image_index) const;
	std::vector<VkImage> getSwapChainImages(VkSwapchainKHR swap_chain) const;
	VkQueue getQueue(uint32_t queue_family_index, uint32_t queue_index) const;
	VkResult setDebugUtilsObjectName(VkObjectType object_type, std::string_view name, const void* handle);

	const Queue& getGraphicsQueue() const;
	const Queue& getTransferQueue() const;
	const Queue& getComputeQueue() const;
	const Queue& getPresentQueue(const Surface& surface) const;
	const Queue& getQueue(VkQueueFlags queue) const;
	const PhysicalDevice& getPhysicalDevice() const { return physical_device; }
	operator const VkDevice& () const { return logical_device; }

	static constexpr std::vector<const char*> getRequiredExtensions() { return { "VK_KHR_swapchain" }; }
	static constexpr std::vector<const char*> getPreferredExtensions() { return { "VK_EXT_memory_priority" }; }

private:
	VkDevice logical_device = nullptr;
	std::vector<std::vector<Queue>> queues;
	std::unordered_set<const char*> enabled_extensions;
	const PhysicalDevice& physical_device;
};