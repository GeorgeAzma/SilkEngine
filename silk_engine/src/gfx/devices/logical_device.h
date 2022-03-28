#pragma once

#include <vulkan/vulkan.hpp>

class LogicalDevice : NonCopyable
{
public:
	LogicalDevice();
	~LogicalDevice();

	void waitIdle() const;
	vk::CommandPool createCommandPool(const vk::CommandPoolCreateInfo& ci) const;
	void destroyCommandPool(vk::CommandPool command_pool) const;
	vk::QueryPool createQueryPool(const vk::QueryPoolCreateInfo& ci) const;
	void destroyQueryPool(vk::QueryPool query_pool) const;
	void resetQueryPool(vk::QueryPool query_pool, uint32_t first_query = 0, uint32_t query_count = 1) const;
	template<typename T>
	auto getQueryPoolResults(vk::QueryPool query_pool, uint32_t first_query, uint32_t query_count, size_t data_size, vk::DeviceSize stride, vk::QueryResultFlags query_result_flags) const 
	{
		return logical_device.getQueryPoolResults<T>(query_pool, first_query, query_count, data_size, stride, query_result_flags);
	}
	std::vector<vk::CommandBuffer> allocateCommandBuffers(const vk::CommandBufferAllocateInfo& allocate_info) const;
	void freeCommandBuffers(vk::CommandPool command_pool, const std::vector<vk::CommandBuffer>& command_buffers) const;
	void resetFences(const std::vector<vk::Fence>& fences) const;
	vk::Fence createFence(const vk::FenceCreateInfo& fence_info) const;
	void destroyFence(vk::Fence fence) const;
	vk::Semaphore createSemaphore(const vk::SemaphoreCreateInfo& semaphore_info) const;
	void destroySemaphore(vk::Semaphore semaphore) const;
	vk::Result waitForFences(const std::vector<vk::Fence>& fences, vk::Bool32 wait_all = VK_TRUE, uint64_t timeout = UINT64_MAX) const;
	vk::Framebuffer createFramebuffer(const vk::FramebufferCreateInfo& framebuffer_info) const;
	void destroyFramebuffer(vk::Framebuffer framebuffer) const;
	vk::DescriptorPool createDescriptorPool(const vk::DescriptorPoolCreateInfo& descriptor_pool_info) const;
	void resetDescriptorPool(vk::DescriptorPool descriptor_pool, vk::DescriptorPoolResetFlags flags = {}) const;
	void destroyDescriptorPool(vk::DescriptorPool descriptor_pool) const;
	std::vector<vk::DescriptorSet> allocateDescriptorSets(const vk::DescriptorSetAllocateInfo& descriptor_set_allocate_info) const;
	vk::Result allocateDescriptorSets(const vk::DescriptorSetAllocateInfo& alloc_info, vk::DescriptorSet& descriptor_set) const;
	void updateDescriptorSets(const std::vector<vk::WriteDescriptorSet>& writes) const;
	vk::DescriptorSetLayout createDescriptorSetLayout(const vk::DescriptorSetLayoutCreateInfo& descriptor_set_layout_create_info) const;
	void destroyDescriptorSetLayout(vk::DescriptorSetLayout descriptor_set_layout) const;
	vk::SubresourceLayout getImageSubresourceLayout(vk::Image image, const vk::ImageSubresource& image_subresource) const;
	vk::ImageView createImageView(const vk::ImageViewCreateInfo& image_view_info) const;
	void destroyImageView(vk::ImageView image_view) const;
	vk::Sampler createSampler(const vk::SamplerCreateInfo& sampler_info) const;
	void destroySampler(vk::Sampler sampler) const;
	vk::PipelineLayout createPipelineLayout(const vk::PipelineLayoutCreateInfo& pipeline_layout_info) const;
	void destroyPipelineLayout(vk::PipelineLayout pipeline_layout) const;
	vk::Pipeline createComputePipeline(vk::PipelineCache pipeline_cache, const vk::ComputePipelineCreateInfo& compute_pipeline_info) const;
	vk::Pipeline createGraphicsPipeline(vk::PipelineCache pipeline_cache, const vk::GraphicsPipelineCreateInfo& graphics_pipeline_info) const;
	void destroyPipeline(vk::Pipeline pipeline) const;
	vk::RenderPass createRenderPass(const vk::RenderPassCreateInfo& render_pass_info) const;
	void destroyRenderPass(vk::RenderPass render_pass) const;
	vk::ShaderModule createShaderModule(const vk::ShaderModuleCreateInfo& shader_module_info) const;
	void destroyShaderModule(vk::ShaderModule shader_module) const;
	vk::SwapchainKHR createSwapChain(const vk::SwapchainCreateInfoKHR& swap_chain_info) const;
	void destroySwapChain(vk::SwapchainKHR swap_chain) const;
	uint32_t acquireNextImage(vk::SwapchainKHR swap_chain, uint64_t timeout, VkSemaphore semaphore, VkFence fence) const;
	vk::Result acquireNextImage(vk::SwapchainKHR swap_chain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* image_index) const;
	std::vector<vk::Image> getSwapChainImages(vk::SwapchainKHR swap_chain) const;

	operator const vk::Device& () const { return logical_device; }
	const vk::Queue& getGraphicsQueue() const { return graphics_queue; }
	const vk::Queue& getTransferQueue() const { return transfer_queue; }
	const vk::Queue& getPresentQueue() const { return present_queue; }
	const vk::Queue& getComputeQueue() const { return compute_queue; }

	static constexpr std::vector<const char*> getRequiredExtensions() { return { "VK_KHR_swapchain" }; }

private:
	vk::Queue graphics_queue;
	vk::Queue transfer_queue;
	vk::Queue present_queue;
	vk::Queue compute_queue;
	vk::Device logical_device;
};