#pragma once

#include "gfx/buffers/indirect_buffer.h"
#include "gfx/buffers/vertex_array.h"
#include "material.h"

struct BatchVertex
{
	glm::vec3 position = glm::vec3(0);
	glm::vec2 texture_coordinates = glm::vec2(0);
	glm::vec3 normal = glm::vec3(0);
	uint32_t texture_index = 0;
	glm::vec4 color = glm::vec4(1);
};

struct Batch
{
	std::vector<BatchVertex> vertices;
	std::vector<uint32_t> indices;
	shared<VertexArray> vertex_array = nullptr;
	shared<IndirectBuffer> indirect_buffer = nullptr;
	shared<MaterialData> material_data = nullptr;
	bool needs_update = true;
};

struct Batcher
{
	std::vector<Batch> batches;
	size_t vertices_index = 0;
	size_t indices_index = 0;
	size_t index_offset = 0;
	bool active = false;
};