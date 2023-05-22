#pragma once

struct RawMesh : NoCopy
{
	RawMesh(size_t vertex_type_size = 0, size_t index_type_size = 0)
		: vertex_type_size(vertex_type_size), index_type_size(index_type_size) {}

	void init(const void* vertices, size_t vertices_size, const void* indices = nullptr, size_t indices_size = 0)
	{
		if (!vertices_size)
			return;
		resize(vertices_size, indices_size);
		memcpy(data.data(), vertices, vertices_size);
		if (indices_size)
			memcpy(data.data() + vertices_size, indices, indices_size);
	}

	void resize(size_t vertices_size, size_t indices_size, const void* vertex_init = nullptr)
	{
		if (!vertices_size)
			return;
		size_t old_vertices_size = this->vertices_size;
		size_t old_indices_size = this->indices_size;
		this->vertices_size = vertices_size;
		this->indices_size = indices_size;
		data.resize(vertices_size + indices_size, 0);
		if (vertex_init)
			for (size_t i = old_vertices_size; i < vertices_size; i += vertex_type_size)
				memcpy(data.data() + i, vertex_init, vertex_type_size);

	}

	RawMesh& move(RawMesh&& other) noexcept
	{
		data = std::move(other.data);
		vertices_size = std::move(other.vertices_size);
		indices_size = std::move(other.indices_size);
		vertex_type_size = std::move(other.vertex_type_size);
		index_type_size = std::move(other.index_type_size);
		return *this;
	}

	const std::vector<uint8_t>& getData() const { return data; }
	size_t getVerticesSize() const { return vertices_size; }
	size_t getIndicesSize() const { return indices_size; }
	size_t getVertexCount() const { return vertices_size / vertex_type_size; }
	size_t getIndexCount() const { return indices_size / index_type_size; }
	size_t getVertexTypeSize() const { return vertex_type_size; }
	size_t getIndexTypeSize() const { return index_type_size; }

protected:
	std::vector<uint8_t>& getData() { return data; }

private:
	std::vector<uint8_t> data{};
	size_t vertices_size = 0;
	size_t indices_size = 0;
	size_t vertex_type_size = 0;
	size_t index_type_size = 0;
};

template<typename V, typename I> requires std::same_as<I, uint32_t> || std::same_as<I, uint16_t>
struct CustomRawMesh : public RawMesh
{
	CustomRawMesh(const std::vector<V>& vertices = {}, const std::vector<I>& indices = {})
		: RawMesh(sizeof(V), sizeof(I)) 
	{
		init(vertices.data(), vertices.size() * sizeof(V), indices.data(), indices.size() * sizeof(I));
	}

	void resize(size_t vertices_count, size_t indices_count = 0)
	{
		V v{};
		RawMesh::resize(vertices_count * sizeof(V), indices_count * sizeof(I), &v);
	}

	V& getVertex(size_t index) { return *((V*)(getData().data()) + index); };
	I& getIndex(size_t index) { return *((I*)(getData().data() + getVerticesSize()) + index); };
};

struct Vertex2D
{
	vec2 position = vec2(0);
	vec2 uv = vec2(0);
	vec4 color = vec4(1);
};

struct RawMesh2D : public CustomRawMesh<Vertex2D, uint32_t>
{
	using CustomRawMesh<Vertex2D, uint32_t>::CustomRawMesh;
};

struct Vertex3D
{
	vec3 position = vec3(0);
	vec2 uv = vec2(0);
	vec3 normal = vec3(0);
	vec4 color = vec4(1);
	vec4 tangent = vec4(0);
};

struct RawMesh3D : public CustomRawMesh<Vertex3D, uint32_t>
{
	using CustomRawMesh<Vertex3D, uint32_t>::CustomRawMesh;
};