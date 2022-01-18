#include "mesh.h"
#include "gfx/buffers/buffer_layout.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	: vertices(vertices), indices(indices)
{
	init();
}

void Mesh::init()
{
	auto ibo = std::make_shared<IndexBuffer>(this->indices.data(), this->indices.size());
	auto vbo = std::make_shared<VertexBuffer>(this->vertices.data(), this->vertices.size() * sizeof(Vertex));
	InstanceData data{};
	auto instance_vbo = std::make_shared<VertexBuffer>(&data, sizeof(InstanceData), VMA_MEMORY_USAGE_CPU_TO_GPU);
	vertex_array = std::shared_ptr<VertexArray>(new VertexArray());
	vertex_array->setIndexBuffer(ibo)
		.addVertexBuffer(vbo)
		.addVertexBuffer(instance_vbo);
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