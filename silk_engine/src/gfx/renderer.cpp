#include "renderer.h"
#include "scene/resources.h"

void Renderer::cleanup()
{
	batcher.batches.clear();
}

void Renderer::beginBatch()
{
	batcher.batches.clear();
	
	batcher.active = true;
}

void Renderer::endBatch()
{
	for (auto& batch : batcher.batches)
	{
		if (!batch.vertices_index || !batch.indices_index)
			continue;

		batch.vertex_array = makeShared<VertexArray>();
		auto vbo = makeShared<VertexBuffer>(batch.vertices.data(), batch.vertices_index * sizeof(BatchVertex), VMA_MEMORY_USAGE_GPU_ONLY);
		auto ibo = makeShared<IndexBuffer>(batch.indices.data(), batch.indices_index, IndexType::UINT32, VMA_MEMORY_USAGE_GPU_ONLY);
		batch.vertex_array->addVertexBuffer(vbo).setIndexBuffer(ibo);
	}

	batcher.active = false;
}

void Renderer::updateBatch()
{
	for (auto& batch : batcher.batches)
	{
		if (batch.needs_update)
		{
			batch.needs_update = false;

			if (!batch.vertices_index || !batch.indices_index)
				continue;

			//TODO: Fix error
			//StagingBuffer staging_buffer_vertex_array(batch.vertices.data(), batch.vertices_index * sizeof(BatchVertex));
			//staging_buffer_vertex_array.copy(*batch.vertex_array->getVertexBuffer(0));
			//StagingBuffer staging_buffer_index_buffer(batch.indices.data(), batch.indices_index * sizeof(uint32_t));
			//staging_buffer_index_buffer.copy(*batch.vertex_array->getIndexBuffer());
		}
	}
}

void Renderer::drawLastBatch()
{
	drawBatch(batcher.batches);
}

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
	batcher.batches.back().vertices.resize(Graphics::MAX_BATCH_VERTICES);
	batcher.batches.back().indices.resize(Graphics::MAX_BATCH_INDICES);
}
