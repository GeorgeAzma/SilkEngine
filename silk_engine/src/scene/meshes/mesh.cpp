#include "mesh.h"

RawMesh::RawMesh(const void* vertices, size_t vertices_size, size_t vertex_type_size, const void* indices, size_t indices_size, size_t index_type_size)
	: vertices(vertices_size), vertex_type_size(vertex_type_size), indices(indices_size), index_type_size(index_type_size)
{
	std::memcpy(this->vertices.data(), vertices, vertices_size);
	std::memcpy(this->indices.data(), indices, indices_size);
}

Mesh::Mesh(const RawMesh& raw_mesh) 
	: vertex_count(raw_mesh.getVertexCount()), index_count(raw_mesh.getIndexCount())
	, vertex_type_size(raw_mesh.getVertexTypeSize()), index_type_size(raw_mesh.getIndexTypeSize())
{
	auto ibo = makeShared<IndexBuffer>(raw_mesh.indices.data(), index_count, index_type_size == 4 ? IndexType::UINT32 : IndexType::UINT16);
	auto vbo = makeShared<VertexBuffer>(raw_mesh.vertices.data(), raw_mesh.vertices.size());
	vertex_array = makeShared<VertexArray>();
	vertex_array->setIndexBuffer(ibo).addVertexBuffer(vbo);
}

Mesh& Mesh::operator=(const RawMesh& raw_mesh)
{
	vertex_count = raw_mesh.getVertexCount(); 
	index_count = raw_mesh.getIndexCount();
	vertex_type_size = raw_mesh.getVertexTypeSize();
	index_type_size = raw_mesh.getIndexTypeSize();

	auto ibo = makeShared<IndexBuffer>(raw_mesh.indices.data(), index_count, index_type_size == 4 ? IndexType::UINT32 : IndexType::UINT16);
	auto vbo = makeShared<VertexBuffer>(raw_mesh.vertices.data(), raw_mesh.vertices.size());
	vertex_array = makeShared<VertexArray>();
	vertex_array->setIndexBuffer(ibo).addVertexBuffer(vbo);

	return *this;
}