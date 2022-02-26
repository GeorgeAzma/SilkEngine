#include "buffer_layout.h"
#include "gfx/enums.h"

BufferLayout::BufferLayout(const std::initializer_list<BufferElement>& elements)
{
	uint32_t offset = 0;
	uint32_t instance_offset = 0;
	uint32_t location = 0;

	bool is_instanced = false;

	for (const BufferElement& element : elements)
	{
		size_t size = EnumInfo::size(element.type);
		size_t actual_rows = (float)size / sizeof(glm::vec4);
		size_t rows = actual_rows + ((size % sizeof(glm::vec4)) > 0);
		for (size_t i = 0; i < rows; ++i)
		{
			vk::VertexInputAttributeDescription attribute_description{};
			attribute_description.format = EnumInfo::type(element.type);
			attribute_description.location = location;

			if (element.instanced)
			{
				is_instanced = true;
				attribute_description.offset = instance_offset;
				attribute_description.binding = 1;

				instance_offset += i < actual_rows ? sizeof(glm::vec4) : (size % sizeof(glm::vec4));
			}
			else
			{
				attribute_description.offset = offset;
				attribute_description.binding = 0;

				offset += i < actual_rows ? sizeof(glm::vec4) : (size % sizeof(glm::vec4));
			}

			++location;
			attribute_descriptions.emplace_back(std::move(attribute_description));
		}
	}

	binding_descriptions.emplace_back();
	binding_descriptions[0].binding = 0;
	binding_descriptions[0].stride = offset;
	binding_descriptions[0].inputRate = vk::VertexInputRate::eVertex;

	if (is_instanced)
	{
		binding_descriptions.emplace_back();
		binding_descriptions[1].binding = 1;
		binding_descriptions[1].stride = instance_offset;
		binding_descriptions[1].inputRate = vk::VertexInputRate::eInstance;
	}
}
