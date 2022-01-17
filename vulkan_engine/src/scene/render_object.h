#pragma once

struct RenderObject
{
	std::shared_ptr<Mesh> mesh = nullptr;
};

struct IndirectBatch
{
	RenderObject render_object;
	size_t first = 0;
	size_t count = 0;

	bool operator==(std::shared_ptr<Mesh> mesh) const 
	{ 
		return *this->mesh == *mesh; 
	}
};