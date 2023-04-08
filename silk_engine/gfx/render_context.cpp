#include "render_context.h"
#include "core/application.h"
#include "instance.h"
#include "devices/physical_device.h"
#include "devices/logical_device.h"
#include "allocators/allocator.h"
#include "pipeline/pipeline_cache.h"
#include "ui/font.h"
#include "descriptors/descriptor_allocator.h"
#include "queues/command_queue.h"
#include "window/window.h"
#include "window/swap_chain.h"
#include "window/surface.h"
#include "buffers/buffer.h"
#include "descriptors/descriptor_set_layout.h"
#include <stb_image_write.h>

void RenderContext::init(std::string_view app_name)
{
	instance = new Instance(app_name);
	physical_device = new PhysicalDevice();
	logical_device = new LogicalDevice(*physical_device);
	command_queue = new CommandQueue();
	allocator = new Allocator(*physical_device, *logical_device);
	pipeline_cache = new PipelineCache();
	Font::init();
}

void RenderContext::destroy()
{
	Sampler::destroy();
	DescriptorSetLayout::destroy();
	DescriptorAllocator::destroy();
	Font::destroy();
	delete pipeline_cache;
	delete allocator;
	delete command_queue;
	delete logical_device;
	delete physical_device;
	delete instance;
}

void RenderContext::update()
{
	DescriptorAllocator::reset();
}

void RenderContext::submit(std::function<void(CommandBuffer&)>&& command)
{
	command_queue->submit(std::forward<std::function<void(CommandBuffer&)>>(command));
}

void RenderContext::execute()
{
	command_queue->execute();
}

void RenderContext::execute(const CommandBuffer::SubmitInfo& submit_info)
{
	command_queue->execute(submit_info);
}

void RenderContext::screenshot(const path& file)
{
	// TODO: fix
	int width = Window::getActive().getWidth();
	int height = Window::getActive().getHeight();
	int channels = Image::getFormatChannelCount(Image::Format(Window::getActive().getSurface().getFormat().format));
	 
	Image::Props props{};
	props.width = width;
	props.height = height;
	props.create_sampler = false;
	props.create_view = false;
	props.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	props.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	props.format = Image::Format(Window::getActive().getSurface().getFormat().format);
	props.tiling = VK_IMAGE_TILING_OPTIMAL;
	shared<Image> destination = makeShared<Image>(props);
	
	auto& img = Window::getActive().getSwapChain().getImages()[Window::getActive().getSwapChain().getImageIndex()];
	img->copyImage(*destination);

	void* data;
	Buffer sb(img->getSize(), Buffer::TRANSFER_DST, { Allocation::RANDOM_ACCESS | Allocation::MAPPED });
	img->copyToBuffer(sb);
	sb.getAllocation().map(&data);
	stbi_write_png(file.string().c_str(), width, height, channels, data, 0);
	
	SK_TRACE("Screenshot saved at {}", file);
}

void RenderContext::vulkanAssert(VkResult result)
{
	SK_VERIFY(result == VK_SUCCESS, std::string("Vulkan: ") + stringifyResult(result));
}

std::string RenderContext::stringifyResult(VkResult result)
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

const PhysicalDevice& RenderContext::getPhysicalDevice() { return logical_device->getPhysicalDevice(); }