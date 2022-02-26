#include "logical_device.h"
#include "physical_device.h"
#include "gfx/graphics.h"

LogicalDevice::LogicalDevice()
{
	const auto& queue_family_indices = Graphics::physical_device->getQueueFamilyIndices();

	std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;

	std::vector<uint32_t> queue_families = queue_family_indices.getIndices();
	std::set<uint32_t> unique_queue_families(queue_families.begin(), queue_families.end());

	float queue_priority = 1.0f;
	for (uint32_t queue_family : unique_queue_families)
	{
		VkDeviceQueueCreateInfo queue_create_info{};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = queue_family;
		queue_create_info.queueCount = 1; 
		queue_create_info.pQueuePriorities = &queue_priority;
		queue_create_infos.emplace_back(std::move(queue_create_info));
	}

	// Specifies which device features we want by enabling them
	vk::PhysicalDeviceFeatures device_features{};
	device_features.samplerAnisotropy = VK_TRUE;
	device_features.occlusionQueryPrecise = VK_TRUE;
	device_features.multiDrawIndirect = VK_TRUE;
	device_features.fragmentStoresAndAtomics = VK_TRUE;

	vk::PhysicalDeviceVulkan12Features vulkan_12_device_features{};
	vulkan_12_device_features.hostQueryReset = VK_TRUE;
	vulkan_12_device_features.drawIndirectCount = VK_TRUE;

	vk::DeviceCreateInfo ci{};
	ci.queueCreateInfoCount = queue_create_infos.size();
	ci.pQueueCreateInfos = queue_create_infos.data();
	ci.pEnabledFeatures = &device_features;
	auto required_extensions = getRequiredExtensions();
	ci.enabledExtensionCount = required_extensions.size();
	ci.ppEnabledExtensionNames = required_extensions.data();
	ci.pNext = &vulkan_12_device_features;

	logical_device = Graphics::physical_device->createLogicalDevice(ci);

	//Get handles of the requried queues
	graphics_queue = logical_device.getQueue(*queue_family_indices.graphics, 0);
	transfer_queue = logical_device.getQueue(*queue_family_indices.transfer, 0);
	present_queue = logical_device.getQueue(*queue_family_indices.present, 0);
	compute_queue = logical_device.getQueue(*queue_family_indices.compute, 0);
}

LogicalDevice::~LogicalDevice()
{
	logical_device.destroy();
}

vk::CommandPool LogicalDevice::createCommandPool(const vk::CommandPoolCreateInfo& ci) const
{
	return logical_device.createCommandPool(ci);
}

void LogicalDevice::destroyCommandPool(vk::CommandPool command_pool) const
{
	logical_device.destroyCommandPool(command_pool);
}

vk::QueryPool LogicalDevice::createQueryPool(const vk::QueryPoolCreateInfo& ci) const
{
	return logical_device.createQueryPool(ci);
}

void LogicalDevice::destroyQueryPool(vk::QueryPool query_pool) const
{
	logical_device.destroyQueryPool(query_pool);
}

void LogicalDevice::resetQueryPool(vk::QueryPool query_pool, uint32_t first_query, uint32_t query_count) const
{
	logical_device.resetQueryPool(query_pool, first_query, query_count);
}

std::vector<vk::CommandBuffer> LogicalDevice::allocateCommandBuffers(const vk::CommandBufferAllocateInfo& allocate_info) const
{
	return logical_device.allocateCommandBuffers(allocate_info);
}

void LogicalDevice::freeCommandBuffers(vk::CommandPool command_pool, const std::vector<vk::CommandBuffer>& command_buffers) const
{
	logical_device.freeCommandBuffers(command_pool, command_buffers);
}

void LogicalDevice::resetFences(const std::vector<vk::Fence>& fences) const
{
	logical_device.resetFences(fences); 
}

vk::Fence LogicalDevice::createFence(const vk::FenceCreateInfo& fence_info) const
{
	return logical_device.createFence(fence_info);
}

void LogicalDevice::destroyFence(vk::Fence fence) const
{
	logical_device.destroyFence(fence);
}

vk::Semaphore LogicalDevice::createSemaphore(const vk::SemaphoreCreateInfo& semaphore_info) const
{
	return logical_device.createSemaphore(semaphore_info);
}

void LogicalDevice::destroySemaphore(vk::Semaphore semaphore) const
{
	logical_device.destroySemaphore(semaphore);
}

void LogicalDevice::waitForFences(const std::vector<vk::Fence>& fences, vk::Bool32 wait_all, uint64_t timeout) const
{
	logical_device.waitForFences(fences, wait_all, timeout); 
}

vk::Framebuffer LogicalDevice::createFramebuffer(const vk::FramebufferCreateInfo& framebuffer_info) const
{
	return logical_device.createFramebuffer(framebuffer_info);
}

void LogicalDevice::destroyFramebuffer(vk::Framebuffer framebuffer) const
{
	logical_device.destroyFramebuffer(framebuffer);
}

vk::DescriptorPool LogicalDevice::createDescriptorPool(const vk::DescriptorPoolCreateInfo& descriptor_pool_info) const
{
	return logical_device.createDescriptorPool(descriptor_pool_info);
}

void LogicalDevice::destroyDescriptorPool(vk::DescriptorPool descriptor_pool) const
{
	logical_device.destroyDescriptorPool(descriptor_pool);
}

std::vector<vk::DescriptorSet> LogicalDevice::allocateDescriptorSets(const vk::DescriptorSetAllocateInfo& descriptor_set_allocate_info) const
{
	return logical_device.allocateDescriptorSets(descriptor_set_allocate_info);
}

void LogicalDevice::updateDescriptorSets(const std::vector<vk::WriteDescriptorSet>& writes) const
{
	logical_device.updateDescriptorSets(writes, {});
}

vk::DescriptorSetLayout LogicalDevice::createDescriptorSetLayout(const vk::DescriptorSetLayoutCreateInfo& descriptor_set_layout_create_info) const
{
	return logical_device.createDescriptorSetLayout(descriptor_set_layout_create_info);
}

void LogicalDevice::destroyDescriptorSetLayout(vk::DescriptorSetLayout descriptor_set_layout) const
{
	logical_device.destroyDescriptorSetLayout(descriptor_set_layout);
}

vk::SubresourceLayout LogicalDevice::getImageSubresourceLayout(vk::Image image, const vk::ImageSubresource& image_subresource) const
{
	return logical_device.getImageSubresourceLayout(image, image_subresource);
}

vk::ImageView LogicalDevice::createImageView(const vk::ImageViewCreateInfo& image_view_info) const
{
	return logical_device.createImageView(image_view_info);
}

void LogicalDevice::destroyImageView(vk::ImageView image_view) const
{
	logical_device.destroyImageView(image_view);
}

vk::Sampler LogicalDevice::createSampler(const vk::SamplerCreateInfo& sampler_info) const
{
	return logical_device.createSampler(sampler_info);
}

void LogicalDevice::destroySampler(vk::Sampler sampler) const
{
	logical_device.destroySampler(sampler);
}

vk::PipelineLayout LogicalDevice::createPipelineLayout(const vk::PipelineLayoutCreateInfo& pipeline_layout_info) const
{
	return logical_device.createPipelineLayout(pipeline_layout_info);
}

void LogicalDevice::LogicalDevice::destroyPipelineLayout(vk::PipelineLayout pipeline_layout) const
{
	logical_device.destroyPipelineLayout(pipeline_layout);
}

vk::Pipeline LogicalDevice::createComputePipeline(vk::PipelineCache pipeline_cache, const vk::ComputePipelineCreateInfo& compute_pipeline_info) const
{
	return logical_device.createComputePipeline(pipeline_cache, compute_pipeline_info);
}

vk::Pipeline LogicalDevice::createGraphicsPipeline(vk::PipelineCache pipeline_cache, const vk::GraphicsPipelineCreateInfo& graphics_pipeline_info) const
{
	return logical_device.createGraphicsPipeline(pipeline_cache, graphics_pipeline_info);
}

void LogicalDevice::LogicalDevice::destroyPipeline(vk::Pipeline pipeline) const
{
	logical_device.destroyPipeline(pipeline);
}

vk::RenderPass LogicalDevice::createRenderPass(const vk::RenderPassCreateInfo& render_pass_info) const
{
	return logical_device.createRenderPass(render_pass_info);
}

void LogicalDevice::destroyRenderPass(vk::RenderPass render_pass) const
{
	logical_device.destroyRenderPass(render_pass);
}

void LogicalDevice::waitIdle() const
{
	logical_device.waitIdle();
}

vk::ShaderModule LogicalDevice::createShaderModule(const vk::ShaderModuleCreateInfo& shader_module_info) const
{
	return logical_device.createShaderModule(shader_module_info);
}

void LogicalDevice::destroyShaderModule(vk::ShaderModule shader_module) const
{
	logical_device.destroyShaderModule(shader_module);
}

vk::SwapchainKHR LogicalDevice::createSwapChain(const vk::SwapchainCreateInfoKHR& swap_chain_info) const
{
	return logical_device.createSwapchainKHR(swap_chain_info);
}

void LogicalDevice::destroySwapChain(vk::SwapchainKHR swap_chain) const
{
	logical_device.destroySwapchainKHR(swap_chain);
}

uint32_t LogicalDevice::acquireNextImage(vk::SwapchainKHR swap_chain, uint64_t timeout, VkSemaphore semaphore, VkFence fence) const
{
	return logical_device.acquireNextImageKHR(swap_chain, timeout, semaphore, fence);
}

std::vector<vk::Image> LogicalDevice::getSwapChainImages(vk::SwapchainKHR swap_chain) const
{
	return logical_device.getSwapchainImagesKHR(swap_chain);
}