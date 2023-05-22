#pragma once

struct RawMesh;
class Buffer;

enum class IndexType
{
	NONE = 0,
	UINT16 = VK_INDEX_TYPE_UINT16,
	UINT32 = VK_INDEX_TYPE_UINT32
};

class VertexArray
{
public:
	VertexArray(const RawMesh& raw_mesh);

	void bind(uint32_t first = 0, VkDeviceSize offset = 0) const;

	// @param count is index_count in case of indexed draw and vertex_count otherwise
	// @param vertex_offset is vertex_offset in case of indexed draw and first_vertex otherwise
	void draw(uint32_t count = 0, uint32_t instances = 1, uint32_t first_index = 0, uint32_t vertex_offset = 0, uint32_t first_instance = 0) const;

	bool isIndexed() const { return index_type != IndexType::NONE; }
	VkDeviceSize getVerticesSize() const { return vertices_size; }
	VkDeviceSize getIndicesSize() const { return indices_size; }
	uint32_t getVertexCount() const { return vertex_count; }
	uint32_t getIndexCount() const { return index_count; }
	IndexType getIndexType() const { return index_type; }
	const shared<Buffer>& getBuffer() const { return buffer; }

	bool operator==(const VertexArray& other) const { return buffer == other.buffer; }

private:
	shared<Buffer> buffer = nullptr;
	IndexType index_type = IndexType::NONE;
	VkDeviceSize vertices_size = 0;
	VkDeviceSize indices_size = 0;
	uint32_t vertex_count = 0;
	uint32_t index_count = 0;
};