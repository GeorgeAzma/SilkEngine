#include "buffer_layout.h"
#include "gfx/enums.h"

BufferLayout::BufferLayout(const std::initializer_list<Type>& elements)
{
	uint32_t offset = 0;
	uint32_t location = 0;

	for (const Type& element : elements)
	{
		VkVertexInputAttributeDescription attribute_description{};
		attribute_description.binding = 0;
		attribute_description.location = location;
		attribute_description.offset = offset;
		attribute_description.format = EnumInfo::type(element);

		attribute_descriptions.emplace_back(std::move(attribute_description));

		const auto size = EnumInfo::size(element);

		location += size / sizeof(glm::vec4);
		location += ((size % sizeof(glm::vec4)) > 0);

		offset += size;
	}

	binding_descriptions.emplace_back();
	binding_descriptions[0].binding = 0;
	binding_descriptions[0].stride = offset;
	binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}
