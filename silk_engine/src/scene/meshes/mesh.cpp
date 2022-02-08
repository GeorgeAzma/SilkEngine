#include "mesh.h"
#include "gfx/buffers/buffer_layout.h"
#include "gfx/graphics.h"
#include "scene/instance.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	: vertices(vertices), indices(indices)
{
}

void Mesh::createVertexArray()
{
	auto vbo = makeShared<VertexBuffer>(this->vertices.data(), this->vertices.size() * sizeof(Vertex));
	auto ibo = makeShared<IndexBuffer>(this->indices.data(), this->indices.size());
	vertex_array = shared<VertexArray>(new VertexArray());
	vertex_array->setIndexBuffer(ibo).addVertexBuffer(vbo);
}

void Mesh::calculateAABB()
{
	aabb.max = glm::vec3(-std::numeric_limits<float>::max());
	aabb.min = glm::vec3(std::numeric_limits<float>::max()); 

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		aabb.max = glm::max(vertices[i].position, aabb.max);
		aabb.min = glm::min(vertices[i].position, aabb.min);
	}
}

void Mesh::calculateTangents()
{
	//std::vector<glm::vec3> positions(vertices.size());
	//std::vector<glm::vec3> normals(vertices.size());
	//std::vector<glm::vec2> texcoords(vertices.size());
	//
	//for (size_t i = 0; i < vertices.size(); ++i)
	//{
	//	positions[i] = vertices[i].position;
	//	normals[i] = vertices[i].normal;
	//	texcoords[i] = vertices[i].texcoord;
	//}
	//
	//std::vector<glm::vec4> tangents = Maths::calculateTangents(vertices.size(), positions.data(), normals.data(), texcoords.data(), indices.size() / 3, indices.data());
	//for (size_t i = 0; i < tangents.size(); ++i)
	//{
	//	vertices[i].tangent = tangents[i];
	//}
}