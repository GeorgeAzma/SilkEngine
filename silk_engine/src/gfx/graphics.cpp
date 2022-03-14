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
#include "buffers/command_buffer.h"
#include "scene/components.h"
#include "scene/scene_manager.h"
#include "renderer.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <GLFW/glfw3.h>

void Graphics::init()
{
	SK_ASSERT(!instance, "Reinitializing vulkan instance is not allowed");

	instance = new Instance();
	surface = new Surface();
	physical_device = new PhysicalDevice();
	logical_device = new LogicalDevice();
	allocator = new Allocator();

	command_buffer = new CommandBuffer();

	descriptor_pool = new DescriptorPool();
	descriptor_pool->addSize(vk::DescriptorType::eUniformBuffer, 64)
		.addSize(vk::DescriptorType::eCombinedImageSampler, 256)
		.addSize(vk::DescriptorType::eStorageBuffer, 64)
		.setMaxSets(1024).build();

	swap_chain = new SwapChain();

	previous_frame_finished = Graphics::logical_device->createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
	swap_chain_image_available = Graphics::logical_device->createSemaphore({});
	render_finished = Graphics::logical_device->createSemaphore({});

	Font::init();

	SK_TRACE("Graphics objects initialized");
}

void Graphics::cleanup()
{
	Font::cleanup();
	Graphics::logical_device->destroyFence(previous_frame_finished);
	Graphics::logical_device->destroySemaphore(swap_chain_image_available);
	Graphics::logical_device->destroySemaphore(render_finished);
	delete command_buffer;
	delete swap_chain;
	delete descriptor_pool;
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

void Graphics::beginFrame()
{
	logical_device->waitForFences({ previous_frame_finished }, VK_TRUE, UINT64_MAX);
	logical_device->resetFences({ previous_frame_finished });
	swap_chain->acquireNextImage(swap_chain_image_available);
	command_buffer->begin();

	vk::Viewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = swap_chain->getExtent().height;
	viewport.width = swap_chain->getExtent().width;
	viewport.height = -(float)swap_chain->getExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	getActiveCommandBuffer().setViewport(0, {viewport});

	vk::Rect2D scissor = {};
	scissor.offset = vk::Offset2D{ 0, 0 };
	scissor.extent = vk::Extent2D{ (uint32_t)swap_chain->getExtent().width, (uint32_t)swap_chain->getExtent().height };
	getActiveCommandBuffer().setScissor(0, { scissor });
}

void Graphics::endFrame()
{
	CommandBufferSubmitInfo submit_info{};
	vk::PipelineStageFlags wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	submit_info.wait_stages = &wait_stage;
	submit_info.wait_semaphores = { swap_chain_image_available };
	submit_info.signal_semaphores = { render_finished };
	submit_info.fence = previous_frame_finished;
	command_buffer->submit(submit_info);
	Graphics::vulkanAssert(swap_chain->present(render_finished));
}

shared<CommandPool> Graphics::getCommandPool()
{
	auto it = command_pools.find(std::this_thread::get_id());
	if (it != command_pools.end())
		return it->second;
	return command_pools.emplace(std::this_thread::get_id(), makeShared<CommandPool>()).first->second;
}

vk::CommandBuffer Graphics::getActiveCommandBuffer()
{
	return Graphics::active.command_buffer.at(std::this_thread::get_id());
}

vk::CommandBuffer Graphics::getActivePrimaryCommandBuffer()
{
	return Graphics::active.primary_command_buffer.at(std::this_thread::get_id());
}

void Graphics::setActiveCommandBuffer(vk::CommandBuffer command_buffer)
{
	std::scoped_lock lock(active_command_buffer_mutex);
	Graphics::active.command_buffer[std::this_thread::get_id()] = command_buffer;
}

void Graphics::setActivePrimaryCommandBuffer(vk::CommandBuffer command_buffer)
{
	std::scoped_lock lock(active_primary_command_buffer_mutex);
	Graphics::active.primary_command_buffer[std::this_thread::get_id()] = command_buffer;
}

void Graphics::screenshot(const std::string& file)
{
	int width = swap_chain->getExtent().width;
	int height = swap_chain->getExtent().height;
	int channels = Image::channelCount(swap_chain->getSurfaceFormat().format);
	size_t pixels = width * height;
	 
	//Create/Copy Image - 2ms
	Image2DProps props{};
	props.width = width;
	props.height = height;
	props.create_sampler = false;
	props.create_view = false;
	props.layout = vk::ImageLayout::eTransferDstOptimal;
	props.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;
	props.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	props.format = swap_chain->getSurfaceFormat().format;
	props.tiling = vk::ImageTiling::eLinear;
	props.mipmap = false;
	props.sampler_props.anisotropy = false;
	shared<Image2D> destination = makeShared<Image2D>(props);
	auto image = swap_chain->getActiveImage();
	bool blit_supported = image->copyImage(destination);

	if (!blit_supported)
	{
		//Change layout to RGBA - 4.5ms
		StorageBuffer image_storage(destination->getSize(), VMA_MEMORY_USAGE_GPU_TO_CPU, vk::BufferUsageFlagBits::eTransferDst);
		destination->copyToBuffer(image_storage);

		CommandBuffer command_buffer;
		command_buffer.begin();
		auto compute = Resources::getComputePipeline("BGRA To RGBA");
		compute->bind();
		compute->getShader()->set("Image", { image_storage });
		compute->getShader()->getDescriptorSets().at(0)->bind();
		compute->dispatch(width * height);
		command_buffer.submitIdle();
		

		//Write PNG - 350ms
		void* buffer_data;
		image_storage.map(&buffer_data);
		stbi_write_png(file.c_str(), width, height, channels, buffer_data, 0);
		image_storage.unmap();
	}
	else
	{
		//Write PNG 350ms
		std::vector<uint8_t> image_data(destination->getSize());
		destination->getData(image_data.data());
		stbi_write_png(file.c_str(), width, height, channels, image_data.data(), 0);
	}
	
	SK_TRACE("Screenshot saved at {0}", file);
}

void Graphics::vulkanAssert(vk::Result result)
{
	SK_ASSERT(result == vk::Result::eSuccess, std::string("Vulkan: ") + EnumInfo::stringifyResult(result));
}