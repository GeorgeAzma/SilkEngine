#pragma once

class RawMesh : NonCopyable
{
public:
	RawMesh() = default;
	RawMesh(const void* vertices, size_t vertices_size, size_t vertex_type_size, const void* indices, size_t indices_size, size_t index_type_size);

	size_t getVertexCount() const { return vertices.size() / vertex_type_size; }
	size_t getIndexCount() const { return indices.size() / index_type_size; }
	size_t getVertexTypeSize() const { return vertex_type_size; }
	size_t getIndexTypeSize() const { return index_type_size; }

	RawMesh& operator=(RawMesh&& other) noexcept
	{
		vertices = std::move(other.vertices);
		indices = std::move(other.indices);
		vertex_type_size = std::move(other.vertex_type_size);
		index_type_size = std::move(other.index_type_size);
		return *this;
	}

public:
	std::vector<uint8_t> vertices{};
	std::vector<uint8_t> indices{};

private:
	size_t vertex_type_size = 0;
	size_t index_type_size = 0;
};

template<typename V, typename I>
class CustomRawMesh : public RawMesh
{
public:
	CustomRawMesh()
		: RawMesh(vertices.data(), vertices.size() * sizeof(V), sizeof(V), indices.data(), indices.size() * sizeof(I), sizeof(I))
	{}
	CustomRawMesh(const std::vector<V>& vertices, const std::vector<I>& indices)
		: RawMesh(vertices.data(), vertices.size() * sizeof(V), sizeof(V), indices.data(), indices.size() * sizeof(I), sizeof(I))
	{}

	V& getVertex(size_t index) { return *((V*)vertices.data() + index); };
	I& getIndex(size_t index) { return *((I*)indices.data() + index); };
	void resizeVertices(size_t size)
	{
		size_t old_size = vertices.size() / sizeof(V);
		vertices.resize(size * sizeof(V));
		for (size_t i = old_size; i < size; ++i)
			*((V*)vertices.data() + i) = V{};
	}
	void resizeIndices(size_t size) { indices.resize(size * sizeof(I), 0u); }
};

struct Vertex2D
{
	vec2 position = vec2(0);
	vec2 texture_coordinate = vec2(0);
	vec4 color = vec4(1);
};

class RawMesh2D : public CustomRawMesh<Vertex2D, uint32_t>
{
public:
	RawMesh2D() : CustomRawMesh<Vertex2D, uint32_t>({}, {}) {}
	RawMesh2D(const std::vector<Vertex2D>& vertices, const std::vector<uint32_t>& indices)
		: CustomRawMesh<Vertex2D, uint32_t>(vertices, indices)
	{}
};

struct Vertex3D
{
	vec3 position = vec3(0);
	vec2 texture_coordinate = vec2(0);
	vec3 normal = vec3(0);
	vec4 color = vec4(1);
};

class RawMesh3D : public CustomRawMesh<Vertex3D, uint32_t>
{
public:
	RawMesh3D() : CustomRawMesh<Vertex3D, uint32_t>({}, {}) {}
	RawMesh3D(const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices)
		: CustomRawMesh<Vertex3D, uint32_t>(vertices, indices)
	{}
};