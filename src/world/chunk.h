#pragma once

#include "block.h"

class Buffer;

class Chunk : NoCopy
{
public:
	using Coord = ivec3;

public:
	static constexpr int32_t SIZE = 64;
	static constexpr int32_t EDGE = SIZE - 1;
	static constexpr Coord DIM = Coord(SIZE);
	static constexpr int32_t AREA = SIZE * SIZE;
	static constexpr int32_t VOLUME = SIZE * AREA;

public:
	static constexpr uint32_t idx(int32_t x, int32_t y, int32_t z) { return z * SIZE + y * AREA + x; }
	static constexpr uint32_t idx(const Chunk::Coord& position) { return idx(position.x, position.y, position.z); }
	static std::array<Coord, 6> getAdjacentNeighborCoords(const Chunk::Coord& position)
	{
		return 
		{
			position + Chunk::Coord(00, -1, 00),
			position + Chunk::Coord(00, 00, -1),
			position + Chunk::Coord(-1, 00, 00),
			position + Chunk::Coord(+1, 00, 00),
			position + Chunk::Coord(00, 00, +1),
			position + Chunk::Coord(00, +1, 00)
		};
	}
	
	static std::array<Coord, 26> getNeighborCoords(const Chunk::Coord& position)
	{
		return 
		{
			position + Chunk::Coord(-1, -1, -1), //  0
			position + Chunk::Coord(00, -1, -1), //  1
			position + Chunk::Coord(+1, -1, -1), //  2
			position + Chunk::Coord(-1, -1, 00), //  3
			position + Chunk::Coord(00, -1, 00), //  4 *
			position + Chunk::Coord(+1, -1, 00), //  5
			position + Chunk::Coord(-1, -1, +1), //  6
			position + Chunk::Coord(00, -1, +1), //  7
			position + Chunk::Coord(+1, -1, +1), //  8
			position + Chunk::Coord(-1, 00, -1), //  9
			position + Chunk::Coord(00, 00, -1), // 10 *
			position + Chunk::Coord(+1, 00, -1), // 11
			position + Chunk::Coord(-1, 00, 00), // 12 *
			position + Chunk::Coord(+1, 00, 00), // 13 *
			position + Chunk::Coord(-1, 00, +1), // 14
			position + Chunk::Coord(00, 00, +1), // 15 *
			position + Chunk::Coord(+1, 00, +1), // 16
			position + Chunk::Coord(-1, +1, -1), // 17
			position + Chunk::Coord(00, +1, -1), // 18
			position + Chunk::Coord(+1, +1, -1), // 19
			position + Chunk::Coord(-1, +1, 00), // 20
			position + Chunk::Coord(00, +1, 00), // 21 *
			position + Chunk::Coord(+1, +1, 00), // 22
			position + Chunk::Coord(-1, +1, +1), // 23
			position + Chunk::Coord(00, +1, +1), // 24
			position + Chunk::Coord(+1, +1, +1)  // 25
		};
	}

	static size_t getIndexFromCoord(const Chunk::Coord& position)
	{
		size_t index = (position.x + 1) + (position.y + 1) * 3 * 3 + (position.z + 1) * 3;
		return index - (index >= 13); // Excluding self
	}

	static Chunk::Coord getCoordFromIndex(size_t index)
	{
		index += index >= 13;
		return { index % 3 - 1, index / 9 - 1, index % 9 / 3 - 1 };
	}

public:
	Chunk(const Coord& position)
		: position(position) {}

	void allocate();
	void generate();
	void generateMesh();
	void buildVertexBuffer();

	void addNeighbor(size_t index, Chunk* neighbor) 
	{ 
		if (neighbors[index] || neighbor == nullptr)
			return;
		neighbors[index] = neighbor;
		dirty = true;
		neighbor->addNeighbor(getIndexFromCoord(position - neighbor->getPosition()), this);
	}

	Block& at(int32_t x, int32_t y, int32_t z) { return blocks[idx(x, y, z)]; }
	Block& at(const Chunk::Coord& position) { return blocks[idx(position)]; }
	Block at(int32_t x, int32_t y, int32_t z) const { return blocks[idx(x, y, z)]; }
	Block at(const Chunk::Coord& position) const { return blocks[idx(position)]; }
	Block atSafe(const Chunk::Coord& position) const;
	const Coord& getPosition() const { return position; }
	const shared<Buffer>& getVertexBuffer() const { return vertex_buffer; }
	uint32_t getVertexCount() const;
	uint32_t getIndexCount() const { return getVertexCount() / 4 * 6; }

	bool operator==(const Chunk& other) const { return position == other.position; }
	bool operator==(const Chunk::Coord& chunk_coord) const { return position == chunk_coord; }

private:
	static uint32_t getAO(int32_t side1, int32_t side2, int32_t corner)
	{
		return (side1 && side2) ? 3 : (side1 + side2 + corner);
	}
	
	std::array<Block, 6> getAdjacentNeighborBlocks(const Chunk::Coord& position) const
	{
		uint32_t i = idx(position);
		return std::array<Block, 6>
		{
			(position.y !=    0) ? blocks[i - AREA] : (neighbors[ 4] ? neighbors[ 4]->at(position.x,	   EDGE, position.z) : Block::AIR),
			(position.z !=	  0) ? blocks[i - SIZE] : (neighbors[10] ? neighbors[10]->at(position.x, position.y,	   EDGE) : Block::AIR),
			(position.x !=	  0) ? blocks[i -    1] : (neighbors[12] ? neighbors[12]->at(	   EDGE, position.y, position.z) : Block::AIR),
			(position.x != EDGE) ? blocks[i +	 1] : (neighbors[13] ? neighbors[13]->at(		  0, position.y, position.z) : Block::AIR),
			(position.z != EDGE) ? blocks[i + SIZE] : (neighbors[15] ? neighbors[15]->at(position.x, position.y,	      0) : Block::AIR),
			(position.y != EDGE) ? blocks[i + AREA] : (neighbors[21] ? neighbors[21]->at(position.x,          0, position.z) : Block::AIR)
		};
	}

private:
	Coord position = Coord(0);
	std::vector<Block> blocks = {}; 
	std::vector<uint64_t> vertices = {};
	shared<Buffer> vertex_buffer = nullptr;
	std::array<Chunk*, 26> neighbors = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	bool dirty = true;
	bool vertex_buffer_dirty = true;
	bool empty = true;
};