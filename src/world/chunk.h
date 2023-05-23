#pragma once

#include "block.h"

class Buffer;

class Chunk : NoCopy
{
public:
	using Coord = ivec3;

public:
	static constexpr int32_t X = 32;
	static constexpr int32_t Y = 32;
	static constexpr int32_t Z = 32;
	static constexpr int32_t EDGE_X = X - 1;
	static constexpr int32_t EDGE_Y = Y - 1;
	static constexpr int32_t EDGE_Z = Z - 1;
	static constexpr Coord DIM = Coord(X, Y, Z);
	static constexpr int32_t AREA = X * Z;
	static constexpr int32_t VOLUME = Y * AREA;

public:
	static constexpr size_t idx(size_t x, size_t y, size_t z) { return z * AREA + y * X + x; }
	static std::array<Coord, 6> getNeighborCoords(const Chunk::Coord& position)
	{
		return 
		{
			position + Coord( 0,  1,  0),
			position + Coord( 0, -1,  0),
			position + Coord(-1,  0,  0),
			position + Coord( 1,  0,  0),
			position + Coord( 0,  0, -1),
			position + Coord( 0,  0,  1)
		};
	}

public:
	Chunk(const Coord& position)
		: position(position) {}

	void allocate();
	void generate();
	void generateMesh();
	void buildVertexBuffer();

	bool hasAllNeighbors() const
	{
		for (Chunk* neighbor : neighbors)
			if (!neighbor)
				return false;
		return true;
	}

	void addNeighbor(size_t index, Chunk* neighbor) 
	{ 
		if (neighbors[index] || neighbor == nullptr)
			return;
		neighbors[index] = neighbor;
		dirty = true;
		size_t my_index = 0;
		switch (index)
		{
		case 0: my_index = 1; break;
		case 1: my_index = 0; break;
		case 2: my_index = 3; break;
		case 3: my_index = 2; break;
		case 4: my_index = 5; break;
		case 5: my_index = 4; break;
		}
		neighbor->addNeighbor(my_index, this);
	}

	Block& at(size_t x, size_t y, size_t z) { return blocks[idx(x, y, z)]; }
	const Coord& getPosition() const { return position; }
	const shared<Buffer>& getVertexBuffer() const { return vertex_buffer; }
	uint32_t getVertexCount() const;
	uint32_t getIndexCount() const { return getVertexCount() / 4 * 6; }

	bool operator==(const Chunk& other) const { return position == other.position; }
	bool operator==(const Chunk::Coord& chunk_coord) const { return position == chunk_coord; }

private:
	Coord position = Coord(0);
	std::vector<Block> blocks = {}; 
	std::vector<uint32_t> vertices = {};
	shared<Buffer> vertex_buffer = nullptr;
	std::array<Chunk*, 6> neighbors = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	bool dirty = true;
	bool vertex_buffer_dirty = true;
	bool empty = true;
};