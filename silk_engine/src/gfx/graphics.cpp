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

	instance = new Instance();
	surface = new Surface();
	physical_device = new PhysicalDevice();
	logical_device = new LogicalDevice();
	allocator = new Allocator();

	command_buffer = new CommandBuffer();

	descriptor_pool = new DescriptorPool();
	descriptor_pool->addSize(vk::DescriptorType::eUniformBuffer, 64)
		.addSize(vk::DescriptorType::eCombinedImageSampler, 64)
		.addSize(vk::DescriptorType::eStorageBuffer, 64)
		.setMaxSets(1024).build();

	swap_chain = new SwapChain();

	previous_frame_finished = Graphics::logical_device->createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
	swap_chain_image_available = Graphics::logical_device->createSemaphore({});
	render_finished = Graphics::logical_device->createSemaphore({});

	Font::init();
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
}

void Graphics::update()
{
	stats.reset();
	//Destroy old unused command pools
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

void Graphics::screenshot(const std::string& file)
{
	DebugTimer t0("Screenshot");
	int width = swap_chain->getExtent().width;
	int height = swap_chain->getExtent().height;
	int channels = Image::channelCount(swap_chain->getSurfaceFormat().format);
	size_t pixels = width * height;

	DebugTimer t1("Create/Copy Image");
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
	t1.stop();

	if (!blit_supported)
	{
		DebugTimer t2("Change layout to RGBA");
		StorageBuffer image_storage(destination->getSize(), VMA_MEMORY_USAGE_GPU_TO_CPU, vk::BufferUsageFlagBits::eTransferDst);
		destination->copyToBuffer(image_storage);

		auto compute = Resources::getComputeShaderEffect("BGRA To RGBA")->pipeline;
		compute->bind();
		compute->getShader()->set("image", { image_storage });
		compute->getShader()->getDescriptorSets().at(0)->bind();
		compute->dispatch(width * height);

		t2.stop();
		
		void* buffer_data;
		image_storage.map(&buffer_data);
		stbi_write_png(file.c_str(), width, height, channels, buffer_data, 0);
		image_storage.unmap();
	}
	else
	{
		std::vector<uint8_t> image_data(destination->getSize());
		destination->getData(image_data.data());
		stbi_write_png(file.c_str(), width, height, channels, image_data.data(), 0);
	}
	t0.stop();
	
	SK_TRACE("Screenshot created: at {0}", file);
}

void Graphics::vulkanAssert(vk::Result result)
{
	SK_ASSERT(result == vk::Result::eSuccess, std::string("Vulkan: ") + EnumInfo::stringifyResult(result));
}