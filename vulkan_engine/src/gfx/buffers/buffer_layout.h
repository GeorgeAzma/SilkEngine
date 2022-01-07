#pragma once

#include "gfx/enums.h"

struct BufferElement
{
	Type type;

private:
	size_t offset = 0;
	friend class BufferLayout;
};

class BufferLayout 
{
public:
	BufferLayout(const std::initializer_list<BufferElement>& elements = {});

	size_t getStride() const { return stride; }
	size_t getDataSize() const { return data_size; }

	std::vector<BufferElement>::iterator begin() { return elements.begin(); }
	std::vector<BufferElement>::iterator end() { return elements.end(); }
	std::vector<BufferElement>::const_iterator begin() const { return elements.begin(); }
	std::vector<BufferElement>::const_iterator end() const { return elements.end(); }

private:
	std::vector<BufferElement> elements;
	size_t stride = 0;
	size_t data_size = 0;
};