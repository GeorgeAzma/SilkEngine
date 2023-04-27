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
	void draw() const;

	bool isIndexed() const { return index_type != IndexType::NONE; }
	uint32_t getVertexCount() const { return vertex_count; }
	uint32_t getIndexCount() const { return index_count; }

private:
	shared<Buffer> buffer = nullptr;
	IndexType index_type = IndexType::NONE;
	VkDeviceSize vertices_size = 0;
	uint32_t vertex_count = 0;
	uint32_t index_count = 0;
};