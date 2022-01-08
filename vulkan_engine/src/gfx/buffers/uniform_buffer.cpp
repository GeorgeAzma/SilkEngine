#include "uniform_buffer.h"

UniformBuffer::UniformBuffer(size_t size)
	: Buffer(size, 
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
{

}