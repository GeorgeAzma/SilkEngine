#include "graphics.h"
#include "instance.h"
#include "window/surface.h"
#include "devices/physical_device.h"
#include "devices/logical_device.h"
#include "window/swap_chain.h"
#include "allocators/command_pool.h"
#include "descriptors/descriptor_pool.h"
#include "descriptors/descriptor_allocator.h"
#include "allocators/allocator.h"
#include "buffers/uniform_buffer.h"
#include "descriptors/descriptor_set.h"
#include "gfx/ui/font.h"
#include "utils/alarm.h"
#include "window/window.h"
#include "scene/resources.h"
#include "buffers/storage_buffer.h"
#include "buffers/command_buffer.h"
#include "scene/components.h"
#include "scene/scene_manager.h"
#include "renderer.h"
#include "gfx/pipeline/compute_pipeline.h"
#include <stb_image_write.h>

void Graphics::init()
{
	SK_ASSERT(!instance, "Reinitializing vulkan instance is not allowed");

	instance = new Instance();
	surface = new Surface();
	physical_device = new PhysicalDevice();
	logical_device = new LogicalDevice();
	allocator = new Allocator();
	swap_chain = new SwapChain();

	Font::init();

	SK_TRACE("Graphics objects initialized");
}

void Graphics::destroy()
{
	Font::destroy();
	delete swap_chain;
	command_pools.clear();
	delete allocator;
	delete logical_device;
	delete physical_device;
	delete surface;
	delete instance;
	glfwTerminate();

	SK_TRACE("Graphics objects destroyed");
}

void Graphics::update()
{
	stats.reset();
	if (command_pool_purge_alarm)
	{
		DescriptorAllocator::reset();
		for (auto it = command_pools.begin(); it != command_pools.end();)
		{
			if (!it->second->allocatedCommandBufferCount())
			{
				it = command_pools.erase(it);
				continue;
			}
	
			++it;
		}
	}
}

shared<CommandPool> Graphics::getActiveCommandPool()
{
	auto it = command_pools.find(std::this_thread::get_id());
	if (it != command_pools.end())
		return it->second;
	std::scoped_lock lock(active_command_pool_mutex);
	return command_pools.emplace(std::this_thread::get_id(), makeShared<CommandPool>()).first->second;
}

CommandBuffer& Graphics::getActiveCommandBuffer()
{
	return *command_buffers.at(std::this_thread::get_id());
}

CommandBuffer* Graphics::getActiveCommandBufferP()
{
	auto it = command_buffers.find(std::this_thread::get_id());
	if (it != command_buffers.end())
		return it->second;
	return nullptr;
}

CommandBuffer& Graphics::getActivePrimaryCommandBuffer()
{
	return *primary_command_buffers.at(std::this_thread::get_id());
}

void Graphics::setActiveCommandBuffer(CommandBuffer* command_buffer)
{
	std::scoped_lock lock(active_command_buffer_mutex);
	command_buffers[std::this_thread::get_id()] = command_buffer;
}

void Graphics::setActivePrimaryCommandBuffer(CommandBuffer* command_buffer)
{
	std::scoped_lock lock(active_primary_command_buffer_mutex);
	primary_command_buffers[std::this_thread::get_id()] = command_buffer;
}

void Graphics::screenshot(std::string_view file)
{
	int width = Window::getWidth();
	int height = Window::getHeight();
	int channels = ImageFormatEnum::getChannelCount(swap_chain->getFormat());
	size_t pixels = width * height;
	 
	Image2DProps props{};
	props.width = width;
	props.height = height;
	props.create_sampler = false;
	props.create_view = false;
	props.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	props.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	props.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	props.format = swap_chain->getFormat();
	props.tiling = VK_IMAGE_TILING_LINEAR;
	props.mipmap = false;
	props.sampler_props.anisotropy = false;
	shared<Image2D> destination = makeShared<Image2D>(props);
	auto image = swap_chain->getImages()[swap_chain->getImageIndex()];
	bool blit_supported = image->copyImage(destination);

	if (!blit_supported)
	{
		StorageBuffer image_storage(destination->getSize(), VMA_MEMORY_USAGE_GPU_TO_CPU, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		destination->copyToBuffer(image_storage);

		CommandBuffer command_buffer;
		command_buffer.begin();
		auto compute = Resources::getComputePipeline("BGRA To RGBA");
		compute->bind();
		compute->getShader()->set("Image", { image_storage });
		compute->getShader()->getDescriptorSets().at(0)->bind();
		compute->dispatch(width * height);
		command_buffer.submitIdle();
		
		void* buffer_data;
		image_storage.map(&buffer_data);
		stbi_write_png(file.data(), width, height, channels, buffer_data, 0);
		image_storage.unmap();
	}
	else
	{
		std::vector<uint8_t> image_data(destination->getSize());
		destination->getData(image_data.data());
		stbi_write_png(file.data(), width, height, channels, image_data.data(), 0);
	}
	
	SK_TRACE("Screenshot saved at {0}", file);
}

void Graphics::vulkanAssert(VkResult result)
{
	SK_ASSERT(result == VK_SUCCESS, std::string("Vulkan: ") + stringifyResult(result));
}

uint32_t Graphics::apiVersion(APIVersion api_version)
{
	switch (api_version)
	{
	case APIVersion::VULKAN_1_0: return VK_API_VERSION_1_0;
	case APIVersion::VULKAN_1_1: return VK_API_VERSION_1_1;
	case APIVersion::VULKAN_1_2: return VK_API_VERSION_1_2;
	}

	SK_ERROR("Unsupported api version specified: {0}.", api_version);
	return uint32_t(0);
}

std::string Graphics::stringifyResult(VkResult result)
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