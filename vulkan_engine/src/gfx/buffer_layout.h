#pragma once

#include "enums.h"

class BufferElement
{
public:
	BufferElement();

	Type type;
};

class BufferLayout 
{
public:
	BufferLayout(const std::initializer_list<BufferElement>& elements = {});

private:
};