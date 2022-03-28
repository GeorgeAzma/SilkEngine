#include "mesh2D.h"

Mesh2D::Mesh2D(const std::vector<Vertex2D>& vertices, const std::vector<uint32_t>& indices)
	: vertices(vertices), Mesh(indices, Type::_2D)
{
	SK_TRACE("Created 2D mesh with {} vertices and {} indices", vertices.size(), indices.size());
}

void Mesh2D::createVertexArray()
{
	if (vertex_array.get())
		return;
	auto vbo = makeShared<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex2D));
	auto ibo = makeShared<IndexBuffer>(indices.data(), indices.size());
	vertex_array = makeShared<VertexArray>();
	vertex_array->setIndexBuffer(ibo).addVertexBuffer(vbo);
}

void Mesh2D::calculateAABB()
{
	has_aabb = true;
	aabb.max = glm::vec2(-std::numeric_limits<float>::max());
	aabb.min = glm::vec2(std::numeric_limits<float>::max()); 

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		aabb.max = glm::max(vertices[i].position, aabb.max);
		aabb.min = glm::min(vertices[i].position, aabb.min);
	}
}

Mesh2D::operator shared<Mesh3D>() const
{
	shared<Mesh3D> mesh3D = nullptr;
	std::vector<Vertex3D> vertices3D(vertices.size());
	for (size_t i = 0; i < vertices.size(); ++i)
	{
		auto& vertex2D = vertices[i];
		auto& vertex3D = vertices3D[i];
		vertex3D.position = glm::vec3(vertex2D.position, 0.0f);
		vertex3D.normal = glm::vec3(0.0);
		vertex3D.texture_coordinate = vertex2D.texture_coordinate;
		vertex3D.color = vertex2D.color;
	}

	mesh3D = makeShared<Mesh3D>(vertices3D, indices);
	if (has_aabb)
	{
		mesh3D->aabb = AABB3D(glm::vec3(aabb.min, 0.0f), glm::vec3(aabb.max, 0.0f));
		mesh3D->has_aabb = true;
	}
	else mesh3D->has_aabb = false;

	if (vertex_array.get())
	{
		auto vbo = makeShared<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex2D)); 
		//WARN: Am not creating new ibo here, because I assume ibo's content won't be changed so there will be no sharing problems
		mesh3D->vertex_array = makeShared<VertexArray>();
		mesh3D->vertex_array->setIndexBuffer(vertex_array->getIndexBuffer()).addVertexBuffer(vbo);
	}

	return mesh3D;
}
