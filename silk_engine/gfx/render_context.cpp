#include "render_context.h"
#include "core/application.h"
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
#include "gfx/pipeline/shader.h"
#include "gfx/pipeline/graphics_pipeline.h"
#include "gfx/pipeline/compute_pipeline.h"
#include "gfx/buffers/command_buffer.h"
#include "scene/meshes/mesh.h"
#include "scene/model.h"
#include "gfx/material.h"
#include <stb_image_write.h>

void RenderContext::init(std::string_view app_name)
{
	instance = new Instance(app_name);
	physical_device = instance->selectPhysicalDevice();
	logical_device = new LogicalDevice(*physical_device);
	for (size_t i = 0; i < 3; ++i)
		command_queues.emplace_back(makeShared<CommandQueue>(physical_device->getGraphicsQueue(), VK_QUEUE_GRAPHICS_BIT));
	if (physical_device->getComputeQueue() != -1)
		for (size_t i = 0; i < 3; ++i)
			compute_command_queues.emplace_back(makeShared<CommandQueue>(physical_device->getComputeQueue(), VK_QUEUE_COMPUTE_BIT));
	if (physical_device->getTransferQueue() != -1)
		for (size_t i = 0; i < 3; ++i)
			transfer_command_queues.emplace_back(makeShared<CommandQueue>(physical_device->getTransferQueue(), VK_QUEUE_TRANSFER_BIT));
	allocator = new Allocator(*physical_device, *logical_device);
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
	command_queues[(int64_t(frame) - 1) >= 0 ? (frame - 1) : 2]->reset();
	if (physical_device->getComputeQueue() != -1)
	compute_command_queues[(int64_t(frame) - 1) >= 0 ? (frame - 1) : 2]->reset(); 
	if (physical_device->getTransferQueue() != -1)
		transfer_command_queues[(int64_t(frame) - 1) >= 0 ? (frame - 1) : 2]->reset();
	DescriptorAllocator::reset();
	frame = (frame + 1) % 3;
}


void RenderContext::screenshot(const path& file)
{
	if (!std::filesystem::exists("res/images/screenshots"))
		std::filesystem::create_directories("res/images/screenshots");
	
	logical_device->wait();

	auto& img = Window::getActive().getSwapChain().getImages()[Window::getActive().getSwapChain().getImageIndex()];

	Image::Props props{};
	props.width = Window::getActive().getWidth();
	props.height = Window::getActive().getHeight();
	props.format = Image::Format::BGRA;
	props.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	props.tiling = VK_IMAGE_TILING_OPTIMAL;
	props.sampler_props.mipmap_mode = Sampler::MipmapMode::NONE;
	props.create_view = false;
	auto image = makeShared<Image>(props);
	img->copyImage(*image);
	execute();

	shared<ComputePipeline> cp = ComputePipeline::get("BGRA To RGBA");
	if (!cp) cp = ComputePipeline::add("BGRA To RGBA", makeShared<ComputePipeline>(makeShared<Shader>("bgra_to_rgba")));
	Material mat(cp);
	Buffer ssbo(img->getSize(), Buffer::UsageBits::STORAGE | Buffer::UsageBits::TRANSFER_DST, Allocation::Props{ .flags = Allocation::RANDOM_ACCESS | Allocation::MAPPED });
	image->copyToBuffer(ssbo);
	execute();
	mat.set("Image", ssbo);
	mat.bind();
	cp->dispatch(img->getSize());
	execute();

	std::vector<byte> data(img->getSize());
	ssbo.getData(data.data(), img->getSize());
	const path folder = "res/images/screenshots";
	path file_path = folder / file;
	stbi_write_png(file_path.string().c_str(), Window::getActive().getWidth(), Window::getActive().getHeight(), Image::getFormatChannelCount(Image::Format(Window::getActive().getSurface().getFormat().format)), data.data(), 0);
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