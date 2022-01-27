#include "renderer.h"
#include "scene/resources.h"

void Renderer::cleanup()
{
	batcher.batches.clear();
}

void Renderer::beginBatch()
{
	batcher.index_offset = 0;
	batcher.vertices_index = 0;
	batcher.indices_index = 0;
	batcher.batches.clear();
	
	batcher.active = true;
}

void Renderer::endBatch()
{
	for (size_t i = 0; i < batcher.batches.size(); ++i)
	{
		if (batcher.batches[i].needs_update)
		{
			batcher.batches[i].vertex_array->getVertexBuffer(0)->setData(batcher.batches[i].vertices.data(), batcher.batches[i].vertices.size() * sizeof(BatchVertex));
			batcher.batches[i].vertex_array->getIndexBuffer()->setData(batcher.batches[i].indices.data(), batcher.batches[i].indices.size() * sizeof(uint32_t));
			batcher.batches[i].needs_update = false;
		}
	}

	batcher.active = false;
}

void Renderer::drawLastBatch()
{
	drawBatch(batcher.batches);
}

void Renderer::drawBatch(const std::vector<Batch>& batches)
{
	for (size_t i = 0; i < batches.size(); ++i)
	{
		//TODO: check if doing these is necessary
		VkDrawIndexedIndirectCommand draw_command{};
		draw_command.firstIndex = 0;
		draw_command.firstInstance = 0;
		draw_command.indexCount = batches[i].indices.size();
		draw_command.instanceCount = 1;
		draw_command.vertexOffset = 0;
		batches[i].indirect_buffer->setData(&draw_command, sizeof(VkDrawIndexedIndirectCommand));
		
		batches[i].material_data->material->pipeline->bind();
		batches[i].vertex_array->bind();
		for (size_t j = 0; j < batches[i].material_data->descriptor_sets.size(); ++j)
			batches[i].material_data->descriptor_sets[j]->bind(j);

		vkCmdDrawIndexedIndirect(Graphics::active.command_buffer, *batches[i].indirect_buffer, 0, 1, sizeof(VkDrawIndexedIndirectCommand));
	}
}

void Renderer::addBatch()
{
	batcher.batches.emplace_back();
	Batch& batch = batcher.batches.back();
	batch.vertices.resize(Graphics::MAX_BATCH_VERTICES);
	batch.indices.resize(Graphics::MAX_BATCH_INDICES);
	
	batch.vertex_array = makeShared<VertexArray>();
	auto vbo = makeShared<VertexBuffer>(nullptr, Graphics::MAX_BATCH_VERTICES * sizeof(BatchVertex), VMA_MEMORY_USAGE_CPU_TO_GPU);
	auto ibo = makeShared<IndexBuffer>(nullptr, Graphics::MAX_BATCH_INDICES * sizeof(uint32_t), IndexType::UINT32, VMA_MEMORY_USAGE_CPU_TO_GPU);
	batch.vertex_array->addVertexBuffer(vbo).setIndexBuffer(ibo);
	
	batch.indirect_buffer = makeShared<IndirectBuffer>(sizeof(VkDrawIndexedIndirectCommand));
}
