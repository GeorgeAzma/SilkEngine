#include "image_barrier.h"
#include "gfx/enums.h"
#include "gfx/graphics.h"

ImageBarrier::ImageBarrier(const ImageBarrierProps& props)
{
	if (props.new_layout == VK_IMAGE_LAYOUT_UNDEFINED || props.old_layout == props.new_layout)
		return;

	VkPipelineStageFlags source_stage;
	VkPipelineStageFlags destination_stage;
	VkAccessFlags source_access_mask;
	VkAccessFlags destination_access_mask;

	//TODO: This only supports few cases, add more in future
	if (props.old_layout == VK_IMAGE_LAYOUT_UNDEFINED && props.new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		source_access_mask = 0;
		destination_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (props.old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && props.new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		source_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
		destination_access_mask = VK_ACCESS_SHADER_READ_BIT;

		source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (props.old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && props.new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		source_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
		destination_access_mask = VK_ACCESS_TRANSFER_READ_BIT;

		source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (props.old_layout == VK_IMAGE_LAYOUT_UNDEFINED && props.new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		source_access_mask = 0;
		destination_access_mask = VK_ACCESS_SHADER_READ_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (props.old_layout == VK_IMAGE_LAYOUT_UNDEFINED && props.new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		source_access_mask = 0;
		destination_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else if (props.old_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && props.new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		source_access_mask = VK_ACCESS_TRANSFER_READ_BIT;
		destination_access_mask = VK_ACCESS_SHADER_READ_BIT;

		source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		SK_ERROR("Vulkan: Unsupported layout transition: old layout - {0}, new layout - {1}", props.old_layout, props.new_layout);
	}

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = props.image;
	barrier.oldLayout = props.old_layout;
	barrier.newLayout = props.new_layout;
	barrier.subresourceRange.aspectMask = props.aspect;
	barrier.subresourceRange.baseMipLevel = props.base_mip_level;
	barrier.subresourceRange.levelCount = props.mip_levels;
	barrier.subresourceRange.baseArrayLayer = props.base_layer;
	barrier.subresourceRange.layerCount = props.layers;
	barrier.srcAccessMask = source_access_mask;
	barrier.dstAccessMask = destination_access_mask;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	vkCmdPipelineBarrier(Graphics::active.command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}
