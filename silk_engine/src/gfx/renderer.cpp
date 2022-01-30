#include "renderer.h"
#include "scene/resources.h"

void Renderer::cleanup()
{
	batcher.batches.clear();
}

void Renderer::beginBatch()
{
	batcher.index_offset = 0;
	batcher.batches.clear();
	
	batcher.active = true;
}

void Renderer::endBatch()
{
	for (size_t i = 0; i < batcher.batches.size(); ++i)
	{
		if (batcher.batches[i].needs_update)
		{
			if (!batcher.batches[i].vertices_index || !batcher.batches[i].indices_index) 
				continue;

			batcher.batches[i].vertex_array = makeShared<VertexArray>();
			auto vbo = makeShared<VertexBuffer>(batcher.batches[i].vertices.data(), batcher.batches[i].vertices_index * sizeof(BatchVertex), VMA_MEMORY_USAGE_GPU_ONLY);
			auto ibo = makeShared<IndexBuffer>(batcher.batches[i].indices.data(), batcher.batches[i].indices_index, IndexType::UINT32, VMA_MEMORY_USAGE_GPU_ONLY);
			batcher.batches[i].vertex_array->addVertexBuffer(vbo).setIndexBuffer(ibo);
			batcher.batches[i].needs_update = false;
		}
	}

	batcher.active = false;
}

void Renderer::drawLastBatch()
{
	drawBatch(batcher.batches);
}
#include "devices/logical_device.h"
void Renderer::drawBatch(const std::vector<Batch>& batches)
{
	for (size_t i = 0; i < batches.size(); ++i)
	{
		if (!batches[i].vertex_array.get())
			continue;

		batches[i].material_data->material->pipeline->bind();
		batches[i].vertex_array->bind();
		for (size_t j = 0; j < batches[i].material_data->descriptor_sets.size(); ++j)
			batches[i].material_data->descriptor_sets[j]->bind(j);
		
		vkCmdDrawIndexed(Graphics::active.command_buffer, batches[i].indices_index, 1, 0, 0, 0);
	}
}

void Renderer::addBatch()
{
	batcher.batches.emplace_back();
	Batch& batch = batcher.batches.back();
	batch.vertices.resize(Graphics::MAX_BATCH_VERTICES);
	batch.indices.resize(Graphics::MAX_BATCH_INDICES);
}
