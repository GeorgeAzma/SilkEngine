#include "buffer_layout.h"

BufferLayout::BufferLayout(const std::vector<BufferElement>& elements)
{
	if (!elements.size())
		return;

	uint32_t offset = 0;
	uint32_t instance_offset = 0;
	uint32_t location = 0;

	bool is_instanced = false;

	for (const BufferElement& element : elements)
	{
		size_t size = GpuTypeEnum::getSize(element.type);
		size_t actual_rows = (float)size / sizeof(vec4);
		size_t rows = actual_rows + ((size % sizeof(vec4)) > 0);
		for (size_t i = 0; i < rows; ++i)
		{
			VkVertexInputAttributeDescription attribute_description{};
			attribute_description.format = GpuTypeEnum::toVulkanType(element.type);
			attribute_description.location = location;

			if (element.instanced)
			{
				is_instanced = true;
				attribute_description.offset = instance_offset;
				attribute_description.binding = 1;

				instance_offset += i < actual_rows ? sizeof(vec4) : (size % sizeof(vec4));
			}
			else
			{
				attribute_description.offset = offset;
				attribute_description.binding = 0;

				offset += i < actual_rows ? sizeof(vec4) : (size % sizeof(vec4));
			}

			++location;
			attribute_descriptions.emplace_back(std::move(attribute_description));
		}
	}

	binding_descriptions.emplace_back();
	binding_descriptions[0].binding = 0;
	binding_descriptions[0].stride = offset;
	binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	if (is_instanced)
	{
		binding_descriptions.emplace_back();
		binding_descriptions[1].binding = 1;
		binding_descriptions[1].stride = instance_offset;
		binding_descriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
	}
}
