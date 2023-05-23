#include "render_context.h"
#include "silk_engine/core/application.h"
#include "instance.h"
#include "devices/physical_device.h"
#include "devices/logical_device.h"
#include "allocators/allocator.h"
#include "pipeline/pipeline_cache.h"
#include "ui/font.h"
#include "descriptors/descriptor_allocator.h"
#include "window/window.h"
#include "window/swap_chain.h"
#include "window/surface.h"
#include "buffers/buffer.h"
#include "descriptors/descriptor_set_layout.h"
#include "silk_engine/gfx/pipeline/shader.h"
#include "silk_engine/gfx/pipeline/graphics_pipeline.h"
#include "silk_engine/gfx/pipeline/compute_pipeline.h"
#include "silk_engine/gfx/buffers/command_buffer.h"
#include "silk_engine/scene/meshes/mesh.h"
#include "silk_engine/scene/model.h"
#include "silk_engine/gfx/material.h"
#include <stb_image_write.h>

void RenderContext::init(std::string_view app_name)
{
	instance = new Instance(app_name);
	physical_device = instance->selectPhysicalDevice();

	using enum PhysicalDevice::Feature;
	logical_device = new LogicalDevice(*physical_device, 
		{ 
			SAMPLER_ANISOTROPY, 
			OCCLUSION_QUERY_PRECISE, 
			MULTI_DRAW_INDIRECT, 
			FRAGMENT_STORES_AND_ATOMICS,  
			FILL_MODE_NON_SOLID,
			GEOMETRY_SHADER,
			TESSELLATION_SHADER,
			WIDE_LINES,
			HOST_QUERY_RESET,
			DRAW_INDIRECT_COUNT,
			MAINTENANCE4,
			PIPELINE_STATISTICS_QUERY
		});

	getCommandQueues();
	getComputeCommandQueues();
	getTransferCommandQueues();

	allocator = new Allocator(*logical_device);
	pipeline_cache = new PipelineCache();
	Font::init();
}

void RenderContext::destroy()
{
	Model::destroy();
	Mesh::destroy();
	Image::destroy();
	Font::destroy();
	ComputePipeline::destroy();
	GraphicsPipeline::destroy();
	Shader::destroy();
	Sampler::destroy();
	DescriptorSetLayout::destroy();
	DescriptorAllocator::destroy();
	delete pipeline_cache;
	delete allocator;
	command_queues.clear();
	delete logical_device;
	delete physical_device;
	delete instance;
}

void RenderContext::update()
{
	uint32_t last_frame = (int64_t(frame) - 1) >= 0 ? (frame - 1) : 2;

	for (auto&& [tid, command_queue] : command_queues)
		command_queue[last_frame]->reset();

	if (physical_device->getComputeQueue() != -1)
		for (auto&& [tid, command_queue] : compute_command_queues)
			command_queue[last_frame]->reset();

	if (physical_device->getTransferQueue() != -1)
		for (auto&& [tid, command_queue] : transfer_command_queues)
			command_queue[last_frame]->reset();

	DescriptorAllocator::reset();

	frame = (frame + 1) % 3;
}


void RenderContext::screenshot(const fs::path& file)
{
	// TODO: Fix sync error (though this works)
	if (!std::filesystem::exists("res/images/screenshots"))
		std::filesystem::create_directories("res/images/screenshots");
	
	logical_device->wait();

	auto& img = Window::getActive().getSwapChain().getImages()[Window::getActive().getSwapChain().getImageIndex()];

	Image::Props props{};
	props.width = Window::getActive().getWidth();
	props.height = Window::getActive().getHeight();
	props.format = Image::Format::RGBA;
	props.usage = Image::TRANSFER_DST | Image::TRANSFER_SRC;
	props.sampler_props.mipmap_mode = Sampler::MipmapMode::NONE;
	auto image = makeShared<Image>(props);
	img->copyToImage(*image);
	execute();

	std::vector<uint8_t> data(img->getSize());
	image->getData(data.data());

	const fs::path folder = "res/images/screenshots";
	fs::path file_path = folder / file;
	stbi_write_png(file_path.string().c_str(), Window::getActive().getWidth(), Window::getActive().getHeight(), Image::getFormatChannels(Image::Format(Window::getActive().getSurface().getFormat())), data.data(), 0);
	SK_TRACE("Screenshot saved at {}", file_path);
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

const std::vector<shared<CommandQueue>>& RenderContext::getCommandQueues()
{
	auto it = command_queues.emplace(std::this_thread::get_id(), std::vector<shared<CommandQueue>>{});
	if (it.second)
		for (size_t i = 0; i < 3; ++i)
			it.first->second.emplace_back(makeShared<CommandQueue>(physical_device->getGraphicsQueue(), VK_QUEUE_GRAPHICS_BIT));
	return it.first->second;
}

const std::vector<shared<CommandQueue>>& RenderContext::getComputeCommandQueues()
{
	auto it = compute_command_queues.emplace(std::this_thread::get_id(), std::vector<shared<CommandQueue>>{});
	if (it.second)
	{
		if (physical_device->getComputeQueue() != physical_device->getGraphicsQueue())
			for (size_t i = 0; i < 3; ++i)
				it.first->second.emplace_back(makeShared<CommandQueue>(physical_device->getComputeQueue(), VK_QUEUE_COMPUTE_BIT));
		else it.first->second = getCommandQueues();
	}
	return it.first->second;
}

const std::vector<shared<CommandQueue>>& RenderContext::getTransferCommandQueues()
{
	auto it = transfer_command_queues.emplace(std::this_thread::get_id(), std::vector<shared<CommandQueue>>{});
	if (it.second)
	{
		if (physical_device->getTransferQueue() != physical_device->getGraphicsQueue())
			for (size_t i = 0; i < 3; ++i)
				it.first->second.emplace_back(makeShared<CommandQueue>(physical_device->getTransferQueue(), VK_QUEUE_TRANSFER_BIT));
		else it.first->second = getCommandQueues();
	}
	return it.first->second;
}