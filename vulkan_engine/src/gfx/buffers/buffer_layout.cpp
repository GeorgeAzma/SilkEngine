#include "buffer_layout.h"
#include "gfx/enums.h"

BufferLayout::BufferLayout(const std::initializer_list<BufferElement>& elements) 
	: elements{elements}
{
	size_t offset = 0;
	for (BufferElement& element : this->elements)
	{
		element.offset = offset;
		offset += EnumInfo::size(element.type);
		stride += EnumInfo::size(element.type);
	}

	data_size = offset;
}
