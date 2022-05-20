#include "mesh3D.h"

Mesh3D::Mesh3D(const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices)
	: vertices(vertices), Mesh(indices)
{
	SK_TRACE("Created 3D mesh with {} vertices and {} indices", vertices.size(), indices.size());
}

void Mesh3D::createVertexArray()
{
	if (vertex_array.get())
		return;
	auto vbo = makeShared<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex3D));
	auto ibo = makeShared<IndexBuffer>(indices.data(), indices.size());
	vertex_array = makeShared<VertexArray>();
	vertex_array->setIndexBuffer(ibo).addVertexBuffer(vbo);
}

void Mesh3D::calculateAABB()
{
	has_aabb = true;
	aabb.max = glm::vec3(-std::numeric_limits<float>::max());
	aabb.min = glm::vec3(std::numeric_limits<float>::max()); 

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		aabb.max = glm::max(vertices[i].position, aabb.max);
		aabb.min = glm::min(vertices[i].position, aabb.min);
	}
}

//void Mesh::calculateTangents()
//{
//	std::vector<glm::vec3> positions(vertices.size());
//	std::vector<glm::vec3> normals(vertices.size());
//	std::vector<glm::vec2> texcoords(vertices.size());
//	
//	for (size_t i = 0; i < vertices.size(); ++i)
//	{
//		positions[i] = vertices[i].position;
//		normals[i] = vertices[i].normal;
//		texcoords[i] = vertices[i].texcoord;
//	}
//	
//	std::vector<glm::vec4> tangents = Maths::calculateTangents(vertices.size(), positions.data(), normals.data(), texcoords.data(), indices.size() / 3, indices.data());
//	for (size_t i = 0; i < tangents.size(); ++i)
//	{
//		vertices[i].tangent = tangents[i];
//	}
//}