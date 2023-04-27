#include "mesh.h"
#include "gfx/render_context.h"

Mesh::Mesh(const RawMesh& raw_mesh)
	: vertex_array(makeShared<VertexArray>(raw_mesh))
{}

void Mesh::draw(uint32_t first_vertex)
{
	vertex_array->bind();
	RenderContext::record([&](CommandBuffer& cb) { cb.draw(getVertexCount(), 1, first_vertex, 0); });
}

void Mesh::drawIndexed(uint32_t first_index, uint32_t vertex_offset)
{
	vertex_array->bind();
	RenderContext::record([&](CommandBuffer& cb) { cb.drawIndexed(getIndexCount(), 1, first_index, vertex_offset, 0); });
}