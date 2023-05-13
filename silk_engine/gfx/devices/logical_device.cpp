#include "logical_device.h"
#include "gfx/render_context.h"
#include "gfx/queue.h"
#include "gfx/window/surface.h"
#include "gfx/instance.h"

LogicalDevice::LogicalDevice(const PhysicalDevice& physical_device, const std::vector<PhysicalDevice::Feature>& features)
	: physical_device(physical_device)
{
	std::vector<uint32_t> queue_family_indices = physical_device.getQueueFamilyIndices();
	queue_family_indices.erase(std::unique(queue_family_indices.begin(), queue_family_indices.end()), queue_family_indices.end());

	float queue_priority = 1.0f;
	std::vector<VkDeviceQueueCreateInfo> device_queues(queue_family_indices.size());
	for (size_t i = 0; i < queue_family_indices.size(); ++ i)
	{
		VkDeviceQueueCreateInfo device_queue_ci{};
		device_queue_ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		device_queue_ci.pQueuePriorities = &queue_priority;
		device_queue_ci.queueCount = 1;
		device_queue_ci.queueFamilyIndex = queue_family_indices[i];
		device_queues[i] = std::move(device_queue_ci);
	}

	// Specifies which device features we want by enabling them
	VkPhysicalDeviceFeatures physical_device_features{};

	VkPhysicalDeviceVulkan13Features physical_device_vulkan_13_features{};
	physical_device_vulkan_13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

	VkPhysicalDeviceVulkan12Features physical_device_vulkan_12_features{};
	physical_device_vulkan_12_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	physical_device_vulkan_12_features.pNext = &physical_device_vulkan_13_features;

	VkPhysicalDeviceVulkan11Features physical_device_vulkan_11_features{};
	physical_device_vulkan_11_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	physical_device_vulkan_11_features.pNext = &physical_device_vulkan_12_features;

	auto getFeature = [&](PhysicalDevice::Feature feature) -> VkBool32*
	{
		if (!physical_device.supportsFeature(feature))
			return nullptr;
		constexpr size_t off = (sizeof(VkStructureType) + sizeof(void*)) / sizeof(VkBool32);
		if (feature <= PhysicalDevice::Feature::VULKAN10_LAST)
			return ((VkBool32*)&physical_device_features) + ecast(feature);
		if (feature <= PhysicalDevice::Feature::VULKAN11_LAST)
			return ((VkBool32*)&physical_device_vulkan_11_features) + off + (ecast(feature) - ecast(PhysicalDevice::Feature::VULKAN10_LAST) - 1);
		if (feature <= PhysicalDevice::Feature::VULKAN12_LAST)
			return ((VkBool32*)&physical_device_vulkan_12_features) + off + (ecast(feature) - ecast(PhysicalDevice::Feature::VULKAN11_LAST) - 1);
		if (feature <= PhysicalDevice::Feature::VULKAN13_LAST)
			return ((VkBool32*)&physical_device_vulkan_13_features) + off + (ecast(feature) - ecast(PhysicalDevice::Feature::VULKAN12_LAST) - 1);
		return nullptr;
	};

	for (size_t i = 0; i < features.size(); ++i)
	{
		VkBool32* feature = getFeature(features[i]);
		if (feature)
		{
			*feature = VK_TRUE;
			enabled_features.emplace(features[i]);
		}
		else SK_WARN("Device feature not supported: {}", ecast(features[i]));
	}

	VkDeviceCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	ci.queueCreateInfoCount = device_queues.size();
	ci.pQueueCreateInfos = device_queues.data();
	ci.pEnabledFeatures = &physical_device_features;

	std::vector<const char*> enabled_extensions;

	for (const auto& required_extension : getRequiredExtensions())
		enabled_extensions.emplace_back(required_extension);

	for (const auto& preferred_extension : getPreferredExtensions())
		if (physical_device.supportsExtension(preferred_extension))
			enabled_extensions.emplace_back(preferred_extension);

	this->enabled_extensions = std::unordered_set<std::string_view>(enabled_extensions.begin(), enabled_extensions.end());

	ci.enabledExtensionCount = enabled_extensions.size();
	ci.ppEnabledExtensionNames = enabled_extensions.data();
	ci.pNext = &physical_device_vulkan_11_features;

	logical_device = physical_device.createLogicalDevice(ci);

	queues.resize(device_queues.size());
	for (size_t i = 0; i < device_queues.size(); ++i)
	{
		const auto& device_queue = device_queues[i];
		queues[i].resize(device_queue.queueCount, Queue(nullptr, 0.0f));
		for (size_t queue_index = 0; queue_index < device_queue.queueCount; ++queue_index)
		{
			VkQueue queue = getQueue(device_queue.queueFamilyIndex, queue_index);
			queues[i][queue_index] = Queue(queue, device_queue.pQueuePriorities[queue_index]);
		}
	}
}

LogicalDevice::~LogicalDevice()
{
	wait();
	vkDestroyDevice(logical_device, nullptr);
}

bool LogicalDevice::hasExtension(const char* extension) const
{
	return enabled_extensions.contains(extension);
}

#pragma region Commands
VkCommandPool LogicalDevice::createCommandPool(const VkCommandPoolCreateInfo& ci) const
{
	VkCommandPool command_pool = nullptr;
	RenderContext::vulkanAssert(vkCreateCommandPool(logical_device, &ci, nullptr, &command_pool));
	return command_pool;
}

VkResult LogicalDevice::resetCommandPool(VkCommandPool command_pool, VkCommandPoolResetFlags reset_flags) const
{
	return vkResetCommandPool(logical_device, command_pool, reset_flags);
}

void LogicalDevice::destroyCommandPool(VkCommandPool command_pool) const
{
	vkDestroyCommandPool(logical_device, command_pool, nullptr);
}

VkQueryPool LogicalDevice::createQueryPool(const VkQueryPoolCreateInfo& ci) const
{
	VkQueryPool query_pool = nullptr;
	RenderContext::vulkanAssert(vkCreateQueryPool(logical_device, &ci, nullptr, &query_pool));
	return query_pool;
}

void LogicalDevice::destroyQueryPool(VkQueryPool query_pool) const
{
	vkDestroyQueryPool(logical_device, query_pool, nullptr);
}

void LogicalDevice::resetQueryPool(VkQueryPool query_pool, uint32_t first_query, uint32_t query_count) const
{
	vkResetQueryPool(logical_device, query_pool, first_query, query_count);
}

std::vector<VkCommandBuffer> LogicalDevice::allocateCommandBuffers(const VkCommandBufferAllocateInfo& allocate_info) const
{
	std::vector<VkCommandBuffer> command_buffers(allocate_info.commandBufferCount, nullptr);
	RenderContext::vulkanAssert(vkAllocateCommandBuffers(logical_device, &allocate_info, command_buffers.data()));
	return command_buffers;
}

void LogicalDevice::freeCommandBuffers(VkCommandPool command_pool, const std::vector<VkCommandBuffer>& command_buffers) const
{
	vkFreeCommandBuffers(logical_device, command_pool, command_buffers.size(), command_buffers.data());
}

VkFence LogicalDevice::createFence(bool signaled) const
{
	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = signaled * VK_FENCE_CREATE_SIGNALED_BIT;
	VkFence fence = nullptr;
	RenderContext::vulkanAssert(vkCreateFence(logical_device, &fence_info, nullptr, &fence));
	return fence;
}

void LogicalDevice::destroyFence(VkFence fence) const
{
	vkDestroyFence(logical_device, fence, nullptr);
}

void LogicalDevice::resetFences(const std::vector<VkFence>& fences) const
{
	RenderContext::vulkanAssert(vkResetFences(logical_device, fences.size(), fences.data()));
}

VkResult LogicalDevice::waitForFences(const std::vector<VkFence>& fences, VkBool32 wait_all, uint64_t timeout) const
{
	return vkWaitForFences(logical_device, fences.size(), fences.data(), wait_all, timeout);
}

VkResult LogicalDevice::getFenceStatus(VkFence fence) const
{
	return vkGetFenceStatus(logical_device, fence);
}

VkSemaphore LogicalDevice::createSemaphore(const VkSemaphoreCreateFlags& flags) const
{
	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_info.flags = flags;
	VkSemaphore semaphore = nullptr;
	RenderContext::vulkanAssert(vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &semaphore));
	return semaphore;
}

void LogicalDevice::destroySemaphore(VkSemaphore semaphore) const
{
	vkDestroySemaphore(logical_device, semaphore, nullptr);
}

VkResult LogicalDevice::signalSemaphore(const VkSemaphore& semaphore, uint64_t value) const
{
	VkSemaphoreSignalInfo semaphore_signal_info{};
	semaphore_signal_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
	semaphore_signal_info.semaphore = semaphore;
	semaphore_signal_info.value = value;
	return vkSignalSemaphore(logical_device, &semaphore_signal_info);
}

VkResult LogicalDevice::waitForSemaphores(const std::vector<VkSemaphore>& semaphores, const std::vector<uint64_t>& values, VkBool32 wait_any, uint64_t timeout) const
{
	SK_VERIFY(semaphores.size() == values.size());
	VkSemaphoreWaitInfo semaphore_wait_info{};
	semaphore_wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	semaphore_wait_info.pSemaphores = semaphores.data();
	semaphore_wait_info.semaphoreCount = semaphores.size();
	semaphore_wait_info.pValues = values.data();
	semaphore_wait_info.flags = wait_any * VK_SEMAPHORE_WAIT_ANY_BIT;
	return vkWaitSemaphores(logical_device, &semaphore_wait_info, timeout);
}	

VkEvent LogicalDevice::createEvent(bool device_only) const
{
	VkEventCreateInfo event_info{};
	event_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
	event_info.flags = device_only * VK_EVENT_CREATE_DEVICE_ONLY_BIT;
	VkEvent event = nullptr;
	RenderContext::vulkanAssert(vkCreateEvent(logical_device, &event_info, nullptr, &event));
	return event;
}

void LogicalDevice::destroyEvent(VkEvent event) const
{
	vkDestroyEvent(logical_device, event, nullptr);
}

VkResult LogicalDevice::setEvent(VkEvent event) const
{
	return vkSetEvent(logical_device, event);
}

VkResult LogicalDevice::resetEvent(VkEvent event) const
{
	return vkResetEvent(logical_device, event);
}

VkResult LogicalDevice::getEventStatus(VkEvent event) const
{
	return vkGetEventStatus(logical_device, event);
}

VkFramebuffer LogicalDevice::createFramebuffer(const VkFramebufferCreateInfo& framebuffer_info) const
{
	VkFramebuffer framebuffer = nullptr;
	RenderContext::vulkanAssert(vkCreateFramebuffer(logical_device, &framebuffer_info, nullptr, &framebuffer));
	return framebuffer;
}

void LogicalDevice::destroyFramebuffer(VkFramebuffer framebuffer) const
{
	vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
}

VkDescriptorPool LogicalDevice::createDescriptorPool(const VkDescriptorPoolCreateInfo& descriptor_pool_info) const
{
	VkDescriptorPool descriptor_pool = nullptr;
	RenderContext::vulkanAssert(vkCreateDescriptorPool(logical_device, &descriptor_pool_info, nullptr, &descriptor_pool));
	return descriptor_pool;
}	

void LogicalDevice::resetDescriptorPool(VkDescriptorPool descriptor_pool, VkDescriptorPoolResetFlags flags) const
{
	RenderContext::vulkanAssert(vkResetDescriptorPool(logical_device, descriptor_pool, flags));
}

void LogicalDevice::destroyDescriptorPool(VkDescriptorPool descriptor_pool) const
{
	vkDestroyDescriptorPool(logical_device, descriptor_pool, nullptr);
}

std::vector<VkDescriptorSet> LogicalDevice::allocateDescriptorSets(const VkDescriptorSetAllocateInfo& descriptor_set_allocate_info) const
{
	std::vector<VkDescriptorSet> descriptor_sets(descriptor_set_allocate_info.descriptorSetCount, nullptr);
	vkAllocateDescriptorSets(logical_device, nullptr, descriptor_sets.data());
	return descriptor_sets;
}
VkResult LogicalDevice::allocateDescriptorSets(const VkDescriptorSetAllocateInfo& alloc_info, VkDescriptorSet& descriptor_set) const
{
	return vkAllocateDescriptorSets(logical_device, &alloc_info, &descriptor_set);
}

void LogicalDevice::updateDescriptorSets(const std::vector<VkWriteDescriptorSet>& writes) const
{
	vkUpdateDescriptorSets(logical_device, writes.size(), writes.data(), 0, nullptr);
}

VkDescriptorSetLayout LogicalDevice::createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& descriptor_set_layout_create_info) const
{
	VkDescriptorSetLayout descriptor_set_layout = nullptr;
	RenderContext::vulkanAssert(vkCreateDescriptorSetLayout(logical_device, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout));
	return descriptor_set_layout;
}

void LogicalDevice::destroyDescriptorSetLayout(VkDescriptorSetLayout descriptor_set_layout) const
{
	vkDestroyDescriptorSetLayout(logical_device, descriptor_set_layout, nullptr);
}

VkSubresourceLayout LogicalDevice::getImageSubresourceLayout(VkImage image, const VkImageSubresource& image_subresource) const
{
	VkSubresourceLayout subresource_layout{};
	vkGetImageSubresourceLayout(logical_device, image, &image_subresource, &subresource_layout);
	return subresource_layout;
}

VkImageView LogicalDevice::createImageView(const VkImageViewCreateInfo& image_view_info) const
{
	VkImageView image_view = nullptr;
	RenderContext::vulkanAssert(vkCreateImageView(logical_device, &image_view_info, nullptr, &image_view));
	return image_view;
}

void LogicalDevice::destroyImageView(VkImageView image_view) const
{
	vkDestroyImageView(logical_device, image_view, nullptr);
}

VkSampler LogicalDevice::createSampler(const VkSamplerCreateInfo& sampler_info) const
{
	VkSampler sampler = nullptr;
	RenderContext::vulkanAssert(vkCreateSampler(logical_device, &sampler_info, nullptr, &sampler));
	return sampler;
}

void LogicalDevice::destroySampler(VkSampler sampler) const
{
	vkDestroySampler(logical_device, sampler, nullptr);
}

VkPipelineLayout LogicalDevice::createPipelineLayout(const VkPipelineLayoutCreateInfo& pipeline_layout_info) const
{
	VkPipelineLayout pipeline_layout = nullptr;
	RenderContext::vulkanAssert(vkCreatePipelineLayout(logical_device, &pipeline_layout_info, nullptr, &pipeline_layout));
	return pipeline_layout;
}

void LogicalDevice::LogicalDevice::destroyPipelineLayout(VkPipelineLayout pipeline_layout) const
{
	vkDestroyPipelineLayout(logical_device, pipeline_layout, nullptr);
}

VkPipeline LogicalDevice::createComputePipeline(VkPipelineCache pipeline_cache, const VkComputePipelineCreateInfo& compute_pipeline_info) const
{
	VkPipeline compute_pipeline = nullptr;
	RenderContext::vulkanAssert(vkCreateComputePipelines(logical_device, pipeline_cache, 1, &compute_pipeline_info, nullptr, &compute_pipeline));
	return compute_pipeline;
}

VkPipeline LogicalDevice::createGraphicsPipeline(VkPipelineCache pipeline_cache, const VkGraphicsPipelineCreateInfo& graphics_pipeline_info) const
{
	VkPipeline graphics_pipeline = nullptr;
	RenderContext::vulkanAssert(vkCreateGraphicsPipelines(logical_device, pipeline_cache, 1, &graphics_pipeline_info, nullptr, &graphics_pipeline));
	return graphics_pipeline;
}

void LogicalDevice::LogicalDevice::destroyPipeline(VkPipeline pipeline) const
{
	vkDestroyPipeline(logical_device, pipeline, nullptr);
}

VkPipelineCache LogicalDevice::createPipelineCache(const VkPipelineCacheCreateInfo& pipeline_cache_info) const
{
	VkPipelineCache pipeline_cache = nullptr;
	vkCreatePipelineCache(logical_device, &pipeline_cache_info, nullptr, &pipeline_cache);
	return pipeline_cache;
}

void LogicalDevice::destroyPipelineCache(VkPipelineCache pipeline_cache) const
{
	vkDestroyPipelineCache(logical_device, pipeline_cache, nullptr);
}

std::vector<uint8_t> LogicalDevice::getPipelineCacheData(VkPipelineCache pipeline_cache) const
{
	size_t data_size = 0;
	vkGetPipelineCacheData(logical_device, pipeline_cache, &data_size, nullptr);
	std::vector<uint8_t> data(data_size);
	vkGetPipelineCacheData(logical_device, pipeline_cache, &data_size, data.data());
	return data;
}

VkRenderPass LogicalDevice::createRenderPass(const VkRenderPassCreateInfo& render_pass_info) const
{
	VkRenderPass render_pass = nullptr;
	RenderContext::vulkanAssert(vkCreateRenderPass(logical_device, &render_pass_info, nullptr, &render_pass));
	return render_pass;
}

void LogicalDevice::destroyRenderPass(VkRenderPass render_pass) const
{
	vkDestroyRenderPass(logical_device, render_pass, nullptr);
}

void LogicalDevice::wait() const
{
	RenderContext::vulkanAssert(vkDeviceWaitIdle(logical_device));
}

VkShaderModule LogicalDevice::createShaderModule(const VkShaderModuleCreateInfo& shader_module_info) const
{
	VkShaderModule shader = nullptr;
	RenderContext::vulkanAssert(vkCreateShaderModule(logical_device, &shader_module_info, nullptr, &shader));
	return shader;
}

void LogicalDevice::destroyShaderModule(VkShaderModule shader_module) const
{
	vkDestroyShaderModule(logical_device, shader_module, nullptr);
}

VkSwapchainKHR LogicalDevice::createSwapChain(const VkSwapchainCreateInfoKHR& swap_chain_info) const
{
	VkSwapchainKHR swap_chain = nullptr;
	RenderContext::vulkanAssert(vkCreateSwapchainKHR(logical_device, &swap_chain_info, nullptr, &swap_chain));
	return swap_chain;
}

void LogicalDevice::destroySwapChain(VkSwapchainKHR swap_chain) const
{
	vkDestroySwapchainKHR(logical_device, swap_chain, nullptr);
}

VkResult LogicalDevice::acquireNextImage(VkSwapchainKHR swap_chain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* image_index) const
{
	return vkAcquireNextImageKHR(logical_device, swap_chain, timeout, semaphore, fence, image_index);
}
#pragma endregion
std::vector<VkImage> LogicalDevice::getSwapChainImages(VkSwapchainKHR swap_chain) const
{
	uint32_t image_count = 0;
	vkGetSwapchainImagesKHR(logical_device, swap_chain, &image_count, nullptr);
	std::vector<VkImage> images(image_count);
	vkGetSwapchainImagesKHR(logical_device, swap_chain, &image_count, images.data());
	return images;
}

VkQueue LogicalDevice::getQueue(uint32_t queue_family_index, uint32_t queue_index) const
{
	VkQueue queue = nullptr;
	vkGetDeviceQueue(logical_device, queue_family_index, queue_index, &queue);
	return queue;
}

VkResult LogicalDevice::setDebugUtilsObjectName(VkObjectType object_type, std::string_view name, const void* handle)
{
#ifdef SK_ENABLE_DEBUG_MESSENGER
	static const auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(physical_device.getInstance(), "vkSetDebugUtilsObjectNameEXT");

	VkDebugUtilsObjectNameInfoEXT name_info;
	name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	name_info.objectType = object_type;
	name_info.pObjectName = name.data();
	name_info.objectHandle = (uint64_t)handle;
	name_info.pNext = nullptr;
	return vkSetDebugUtilsObjectNameEXT(logical_device, &name_info);
#else
	return VK_ERROR_EXTENSION_NOT_PRESENT;
#endif
}

const Queue& LogicalDevice::getGraphicsQueue() const 
{ 
	return queues[RenderContext::getPhysicalDevice().getGraphicsQueue()][0];
}

const Queue& LogicalDevice::getTransferQueue() const 
{ 
	return queues[RenderContext::getPhysicalDevice().getTransferQueue()][0];
}

const Queue& LogicalDevice::getComputeQueue() const 
{
	return queues[RenderContext::getPhysicalDevice().getComputeQueue()][0];
}

const Queue& LogicalDevice::getPresentQueue(const Surface& surface) const
{
	return queues[surface.getPresentQueue()][0];
}

const Queue& LogicalDevice::getQueue(VkQueueFlags queue) const
{
	switch (queue)
	{
	case VK_QUEUE_GRAPHICS_BIT: return RenderContext::getLogicalDevice().getGraphicsQueue();
	case VK_QUEUE_TRANSFER_BIT: return RenderContext::getLogicalDevice().getTransferQueue();
	case VK_QUEUE_COMPUTE_BIT: return RenderContext::getLogicalDevice().getComputeQueue();
	}

	return RenderContext::getLogicalDevice().getGraphicsQueue();
}
