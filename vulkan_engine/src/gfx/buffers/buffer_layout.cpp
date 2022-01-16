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
		VkVertexInputAttributeDescription attribute_description{};
		attribute_description.binding = 0;
		attribute_description.location = location;
		attribute_description.offset = offset;
		attribute_description.format = EnumInfo::type(element.type);

		if (element.instanced)
		{
			is_instanced = true;
			attribute_description.binding = 1;
			attribute_description.offset = instance_offset;
			attribute_descriptions.emplace_back(std::move(attribute_description)); 
			
			const auto size = EnumInfo::size(element.type);

			location += size / sizeof(glm::vec4);
			location += ((size % sizeof(glm::vec4)) > 0);

			instance_offset += size;
		}
		else
		{
			attribute_descriptions.emplace_back(std::move(attribute_description));

			const auto size = EnumInfo::size(element.type);

			location += size / sizeof(glm::vec4);
			location += ((size % sizeof(glm::vec4)) > 0);

			offset += size;
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
