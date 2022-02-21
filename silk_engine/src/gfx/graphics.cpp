#include "graphics.h"
#include "enums.h"
#include "instance.h"
#include "window/surface.h"
#include "devices/physical_device.h"
#include "devices/logical_device.h"
#include "window/swap_chain.h"
#include "allocators/command_pool.h"
#include "descriptors/descriptor_pool.h"
#include "allocators/allocator.h"
#include "buffers/uniform_buffer.h"
#include "descriptors/descriptor_set.h"
#include "gfx/ui/font.h"
#include "utils/alarm.h"
#include "window/window.h"
#include "scene/resources.h"
#include "buffers/storage_buffer.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

void Graphics::init()
{
	SK_ASSERT(!instance, "Vulkan: Reinitializing vulkan instance is not allowed");

	//These most likely won't change
	instance = new Instance(); //70ms
	surface = new Surface(); //0.05ms
	physical_device = new PhysicalDevice(); //10ms
	logical_device = new LogicalDevice(); //80ms
	allocator = new Allocator();

	descriptor_pool = new DescriptorPool();
	descriptor_pool->addSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 64)
		.addSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 64)
		.addSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 64)
		.setMaxSets(1024).build();

	swap_chain = new SwapChain(); //16ms

	global_uniform = new UniformBuffer(sizeof(GlobalUniformData));

	Font::init();
}

void Graphics::cleanup() //25ms
{
	Font::cleanup();
	delete global_uniform;
	delete swap_chain;
	delete descriptor_pool;
	command_pools.clear();
	delete allocator;
	delete logical_device;
	delete physical_device;
	delete surface;
	delete instance;

	glfwTerminate();
}

void Graphics::update()
{
	//Destroy old unused command pools
	if (command_pool_purge_alarm)
	{
		screenshot("data/images/screenshots/screenshot.png");
		for (auto it = command_pools.begin(); it != command_pools.end();)
		{
			if (it->second.use_count() <= 1)
			{
				it = command_pools.erase(it);
				continue;
			}
	
			++it;
		}
	}
}

shared<CommandPool> Graphics::getCommandPool()
{
	auto it = command_pools.find(std::this_thread::get_id());
	if (it != command_pools.end())
		return it->second;
	return command_pools.emplace(std::this_thread::get_id(), makeShared<CommandPool>()).first->second;
}

void Graphics::screenshot(const std::string& file)
{
	//DebugTimer t0("Screenshot");
	//int width = swap_chain->getExtent().width;
	//int height = swap_chain->getExtent().height;
	//int channels = Image::channelCount(swap_chain->getSurfaceFormat().format);
	//size_t pixels = width * height;
	//
	//DebugTimer t1("Copy From swapchain");
	//Image2DProps props{};
	//props.width = width;
	//props.height = height;
	//props.create_sampler = false;
	//props.create_view = false;
	//props.layout = VK_IMAGE_LAYOUT_UNDEFINED;
	//props.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	//props.memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	//props.format = swap_chain->getSurfaceFormat().format;
	//props.tiling = VK_IMAGE_TILING_LINEAR;
	//props.mipmap = false;
	//props.sampler_props.anisotropy = false;
	//shared<Image2D> destination = makeShared<Image2D>(props);
	//auto image = swap_chain->getActiveImage();
	//image->copyImage(destination);
	//t1.stop();
	//
	//DebugTimer t2("Host image transformation");
	//
	//VkImageSubresource image_subresource = {};
	//image_subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//image_subresource.mipLevel = 0;
	//image_subresource.arrayLayer = 0;
	//
	//VkSubresourceLayout destination_subresource_layout;
	//vkGetImageSubresourceLayout(*Graphics::logical_device, *destination, &image_subresource, &destination_subresource_layout);
	//
	//void* data;
	//std::vector<uint8_t> bitmap(destination_subresource_layout.size);
	//destination->getData(bitmap.data());
	//
	//CommandBuffer command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_QUEUE_COMPUTE_BIT);
	//command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	//auto compute = Resources::getComputeShaderEffect("BGRA To RGBA")->pipeline;
	//compute->bind();
	//
	//props.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	//props.format = VK_FORMAT_R8G8B8A8_UNORM;
	//props.layout = VK_IMAGE_LAYOUT_GENERAL;
	//props.tiling = VK_IMAGE_TILING_OPTIMAL;
	//props.create_sampler = true;
	//props.create_view = true;
	//Image2D image_storage(props);
	//
	//compute->getShader()->getDescirptorSets().at(0)->setImageInfo(0, { *image });
	//compute->getShader()->getDescirptorSets().at(0)->update();
	//compute->getShader()->getDescirptorSets().at(0)->bind();
	//compute->dispatch(width * height);
	//command_buffer.submitIdle();
	//
	//image_storage.getData(bitmap.data());
	//
	//t2.stop();
	//
	//DebugTimer t3("Writing png");
	//
	//stbi_write_png(file.c_str(), width, height, channels, bitmap.data(), 0);
	//
	//t3.stop();
	//t0.stop();
	//SK_TRACE("Screenshot created: at {0}", file);
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
		return "Out of host memory";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "Out of device memory";
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