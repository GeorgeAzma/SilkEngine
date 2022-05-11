#include "graphics.h"
#include "enums.h"
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

void Graphics::cleanup()
{
	Font::cleanup();
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
	int width = swap_chain->getExtent().width;
	int height = swap_chain->getExtent().height;
	int channels = ImageFormatEnum::getChannelCount(ImageFormatEnum::fromVulkanType(swap_chain->getFormat()));
	size_t pixels = width * height;
	 
	Image2DProps props{};
	props.width = width;
	props.height = height;
	props.create_sampler = false;
	props.create_view = false;
	props.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	props.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	props.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
	props.format = ImageFormatEnum::fromVulkanType(swap_chain->getFormat());
	props.tiling = VK_IMAGE_TILING_LINEAR;
	props.mipmap = false;
	props.sampler_props.anisotropy = false;
	shared<Image2D> destination = makeShared<Image2D>(props);
	auto image = swap_chain->getActiveImage();
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
	SK_ASSERT(result == VK_SUCCESS, std::string("Vulkan: ") + EnumInfo::stringifyResult(result));
}