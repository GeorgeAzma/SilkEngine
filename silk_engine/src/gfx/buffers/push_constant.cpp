#include "push_constant.h"
#include "gfx/graphics.h"
#include "gfx/devices/physical_device.h"

void PushContant::addConstant(const void* data, uint32_t size, VkShaderStageFlags shader_stage)
{
	Data constant_data;
	constant_data.data.resize(size);
	std::memcpy(constant_data.data.data(), data, size);
	constant_data.offset = offset;
	offsets[(uint32_t)shader_stage] = constant_data;
	offset += size;
	SK_ASSERT(offset <= Graphics::physical_device->getProperties().limits.maxPushConstantsSize, "Couldn't add data to the push constant, out of size limit: {0}/{1}", offset, Graphics::physical_device->getProperties().limits.maxPushConstantsSize);
}

void PushContant::push(const void* data, VkShaderStageFlags shader_stage)
{
	Data& constant_data = offsets.at(shader_stage);
	std::memcpy(constant_data.data.data(), data, constant_data.data.size());
	vkCmdPushConstants(Graphics::active.command_buffer, Graphics::active.pipeline_layout, shader_stage, constant_data.offset, constant_data.data.size(), constant_data.data.data());
}
