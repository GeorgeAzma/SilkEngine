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
	RenderContext::record([&](CommandBuffer& cb) { cb.bindVertexBuffers(first, { *buffer }, { offset }); });
	if (isIndexed())
		RenderContext::record([&](CommandBuffer& cb) { cb.bindIndexBuffer(*buffer, vertices_size + offset, VkIndexType(index_type)); });
}

void VertexArray::draw() const
{
	bind();
	if (isIndexed())
		RenderContext::record([&](CommandBuffer& cb) { cb.drawIndexed(index_count, 1, 0, 0, 0); });
	else
		RenderContext::record([&](CommandBuffer& cb) { cb.draw(vertex_count, 1, 0, 0); });
}