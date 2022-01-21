#include "graphics.h"
#include "core/event.h"
#include "enums.h"
#include "buffers/buffer_layout.h"
#include "core/time.h"
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
#include "buffers/uniform_buffer.h"

void Graphics::init()
{
	SK_ASSERT(!instance, "Vulkan: Reinitializing vulkan instance is not allowed");

	//These most likely won't change
	instance = new Instance(); //70ms
	surface = new Surface(); //0.05ms
	physical_device = new PhysicalDevice(); //10ms
	logical_device = new LogicalDevice(); //80ms
	allocator = new Allocator();
	command_pool = new CommandPool(); //0.025ms

	descriptor_pool = new DescriptorPool();
	descriptor_pool->addSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8)
		.addSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8)
		.addSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 8)
		.setMaxSets(8).build();

	swap_chain = new SwapChain(); //16ms

	global_uniform = new UniformBuffer(sizeof(GlobalUniformData));
}

void Graphics::beginFrame()
{
	swap_chain->beginFrame();
}

void Graphics::beginRenderPass()
{
	swap_chain->beginRenderPass();
}

void Graphics::endFrame()
{
	swap_chain->endFrame();
}

void Graphics::endRenderPass()
{
	swap_chain->endRenderPass();
}

void Graphics::cleanup() //25ms
{
	delete global_uniform;
	delete swap_chain;
	delete descriptor_pool;
	delete command_pool;
	delete allocator;
	delete logical_device;
	delete physical_device;
	delete surface;
	delete instance;

	glfwTerminate();
}

void Graphics::vulkanAssert(VkResult result)
{
	SK_ASSERT(result == VK_SUCCESS, std::string("Vulkan: ") + stringifyResult(result));
}

constexpr std::string Graphics::stringifyResult(VkResult result)
{
	switch (result) 
	{
	case VK_SUCCESS:
		return "Success";
	case VK_NOT_READY:
		return "A fence or query has not yet completed";
	case VK_TIMEOUT:
		return "A wait operation has not completed in the specified time";
	case VK_EVENT_SET:
		return "An event is signaled";
	case VK_EVENT_RESET:
		return "An event is unsignaled";
	case VK_INCOMPLETE:
		return "A return array was too small for the result";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "A host memory allocation has failed";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "A device memory allocation has failed";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "Initialization of an object could not be completed for implementation-specific reasons";
	case VK_ERROR_DEVICE_LOST:
		return "The logical or physical device has been lost";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "Mapping of a memory object has failed";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "A requested layer is not present or could not be loaded";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "A requested extension is not supported";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "A requested feature is not supported";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "Too many objects of the type have already been created";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "A requested format is not supported on this device";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "A surface is no longer available";
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		return "A allocation failed due to having no more space in the descriptor pool";
	case VK_SUBOPTIMAL_KHR:
		return "A swapchain no longer matches the surface properties exactly, but can still be used";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "A surface has changed in such a way that it is no longer compatible with the swapchain";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "The display used by a swapchain does not use the same presentable image layout";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API";
	case VK_ERROR_VALIDATION_FAILED_EXT:
		return "A validation layer found an error";
	default:
		return "Unknown Vulkan error";
	}
}