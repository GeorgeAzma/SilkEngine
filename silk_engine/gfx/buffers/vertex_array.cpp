#include "vertex_array.h"
#include "gfx/render_context.h"
#include "scene/meshes/raw_mesh.h"
#include "command_buffer.h"
#include "buffer.h"

VertexArray::VertexArray(const RawMesh& raw_mesh)
	: buffer(makeShared<Buffer>(raw_mesh.getData().size(), Buffer::VERTEX | Buffer::INDEX | Buffer::TRANSFER_DST)),
	index_type(raw_mesh.getIndexTypeSize() == sizeof(uint32_t) ? IndexType::UINT32 : (raw_mesh.getIndexTypeSize() == sizeof(uint16_t) ? IndexType::UINT16 : IndexType::NONE)),
	vertices_size(raw_mesh.getVerticesSize()),
	indices_size(raw_mesh.getIndicesSize()),
	vertex_count(raw_mesh.getVertexCount()),
	index_count(raw_mesh.getIndexCount())
{
	buffer->setData(raw_mesh.getData().data());
}

void VertexArray::bind(uint32_t first, VkDeviceSize offset) const
{
	CommandBuffer& cb = RenderContext::getCommandBuffer();
	cb.bindVertexBuffers(first, { *buffer }, { offset });
	if (isIndexed())
		cb.bindIndexBuffer(*buffer, vertices_size + offset, VkIndexType(index_type));
}

void VertexArray::draw(uint32_t count, uint32_t instances, uint32_t first_index, uint32_t vertex_offset, uint32_t first_instance) const
{
	bind();
	CommandBuffer& cb = RenderContext::getCommandBuffer();
	if (isIndexed())
		cb.drawIndexed(count ? count : index_count, instances, first_index, vertex_offset, first_instance);
	else
		cb.draw(count ? count : vertex_count, instances, vertex_offset, first_instance);
}