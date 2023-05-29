#pragma once

#include "block.h"

class Buffer;
class Material;

class Chunk : NoCopy
{
	friend class World;

public:
	using Coord = ivec3;

public:
	static constexpr int32_t SIZE = 32;
	static constexpr int32_t EDGE = SIZE - 1;
	static constexpr int32_t AREA = SIZE * SIZE;
	static constexpr int32_t VOLUME = SIZE * AREA;
	static constexpr Coord DIM = Coord(SIZE);
	static constexpr size_t MAX_VERTICES = VOLUME * 4 * 6;
	static constexpr size_t MAX_INDICES = VOLUME * 6 * 6;
	static constexpr size_t VERTEX_SIZE = sizeof(uint64_t);
	static constexpr size_t INDEX_SIZE = sizeof(uint32_t);

public:
	Chunk(const Coord& position);
	~Chunk();

	void generate();
	void generateMesh();
	void render() const;

	void addNeighbor(size_t index, Chunk* neighbor) 
	{ 
		if (neighbors[index] || neighbor == nullptr)
			return;
		neighbors[index] = neighbor;
		dirty = true;
		neighbor->addNeighbor(getIndexFromCoord(position - neighbor->getPosition()), this);
	}

	std::vector<Chunk::Coord> getMissingNeighborLocations() const
	{
		std::vector<Chunk::Coord> missing_neighbors;
		std::array<Chunk::Coord, 6> adjacent = getAdjacentNeighborCoords();
		for (size_t i = 0; i < adjacent.size(); ++i)
			if (!neighbors[getIndexFromCoord(adjacent[i])])
				missing_neighbors.emplace_back(adjacent[i]);
		return missing_neighbors;
	}

	bool isInside(const Chunk::Coord& position) const { return position.x >= 0 && position.y >= 0 && position.z >= 0 && position.x < SIZE && position.y < SIZE && position.z < SIZE; }
	Block& at(uint32_t idx) { return blocks[idx]; }
	Block& at(uint32_t x, uint32_t y, uint32_t z) { return blocks[idx(x, y, z)]; }
	Block& at(const Chunk::Coord& position) { return blocks[idx(position)]; }
	Block at(uint32_t idx) const { return blocks[idx]; }
	Block at(uint32_t x, uint32_t y, uint32_t z) const { return blocks[idx(x, y, z)]; }
	Block at(const Chunk::Coord& position) const { return blocks[idx(position)]; }
	Block atSafe(const Chunk::Coord& position) const;
	const Coord& getPosition() const { return position; }
	const shared<Buffer>& getVertexBuffer() const { return vertex_buffer; }
	uint32_t getVertexCount() const { return vertex_count; }
	uint32_t getIndexCount() const { return index_count; }
	Block getFill() const { return fill; }

	bool operator==(const Chunk& other) const { return position == other.position; }
	bool operator==(const Chunk::Coord& chunk_coord) const { return position == chunk_coord; }

public:
	static constexpr uint32_t idx(uint32_t x, uint32_t y, uint32_t z) { return z * SIZE + y * AREA + x; }
	static constexpr uint32_t idx(const Chunk::Coord& position) { return idx(position.x, position.y, position.z); }
	static constexpr Chunk::Coord getCoordFromIdx(uint32_t idx) { return { idx % SIZE, idx / AREA, idx % AREA / SIZE }; }
	static std::array<Chunk::Coord, 6> getAdjacentNeighborCoords()
	{
		return
		{
			Chunk::Coord(00, -1, 00),
			Chunk::Coord(00, 00, -1),
			Chunk::Coord(-1, 00, 00),
			Chunk::Coord(+1, 00, 00),
			Chunk::Coord(00, 00, +1),
			Chunk::Coord(00, +1, 00)
		};
	}

	static std::array<Coord, 26> getNeighborCoords()
	{
		return
		{
			Chunk::Coord(-1, -1, -1), //  0
			Chunk::Coord(00, -1, -1), //  1
			Chunk::Coord(+1, -1, -1), //  2
			Chunk::Coord(-1, -1, 00), //  3
			Chunk::Coord(00, -1, 00), //  4 *
			Chunk::Coord(+1, -1, 00), //  5
			Chunk::Coord(-1, -1, +1), //  6
			Chunk::Coord(00, -1, +1), //  7
			Chunk::Coord(+1, -1, +1), //  8
			Chunk::Coord(-1, 00, -1), //  9
			Chunk::Coord(00, 00, -1), // 10 *
			Chunk::Coord(+1, 00, -1), // 11
			Chunk::Coord(-1, 00, 00), // 12 *
			Chunk::Coord(+1, 00, 00), // 13 *
			Chunk::Coord(-1, 00, +1), // 14
			Chunk::Coord(00, 00, +1), // 15 *
			Chunk::Coord(+1, 00, +1), // 16
			Chunk::Coord(-1, +1, -1), // 17
			Chunk::Coord(00, +1, -1), // 18
			Chunk::Coord(+1, +1, -1), // 19
			Chunk::Coord(-1, +1, 00), // 20
			Chunk::Coord(00, +1, 00), // 21 *
			Chunk::Coord(+1, +1, 00), // 22
			Chunk::Coord(-1, +1, +1), // 23
			Chunk::Coord(00, +1, +1), // 24
			Chunk::Coord(+1, +1, +1)  // 25
		};
	}

	static uint32_t getIndexFromCoord(const Chunk::Coord& position)
	{
		uint32_t index = (position.x + 1) + (position.y + 1) * 9 + (position.z + 1) * 3;
		return index - (index >= 13); // Excluding self
	}

	static Chunk::Coord getCoordFromIndex(uint32_t index)
	{
		index += index >= 13;
		return { index % 3 - 1, index / 9 - 1, index % 9 / 3 - 1 };
	}

private:
	static uint64_t getAO(uint32_t side1, uint32_t side2, uint32_t corner)
	{
		return (side1 && side2) ? 3 : (side1 + side2 + corner);
	}

private:
	Coord position = Coord(0);
	std::vector<Block> blocks = {}; 
	uint32_t vertex_count = 0;
	uint32_t index_count = 0;
	shared<Buffer> blocks_buffer = nullptr;
	shared<Buffer> vertex_buffer = nullptr;
	shared<Buffer> index_buffer = nullptr;
	shared<Material> gen_material = nullptr;
	std::array<Chunk*, 26> neighbors = {};
	bool dirty = true;
	Block fill = Block::AIR;
};