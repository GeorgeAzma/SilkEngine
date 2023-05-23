#pragma once

#include "chunk.h"
#include "silk_engine/utils/thread_pool.h"

class Material;
class Image;
class Buffer;
class Entity;

class World
{
public:
	using Coord = ivec3;

	struct PushConstantData
	{
		ivec4 chunk_position;
		vec4 light_position;
		vec4 light_color;
	};

public:
	static Chunk::Coord toChunkCoord(const World::Coord& position) 
	{ 
		return glm::ivec3(
			(position.x < 0) ? ((position.x + 1 - Chunk::X) / Chunk::X) : (position.x / Chunk::X),
			(position.y < 0) ? ((position.y + 1 - Chunk::Y) / Chunk::Y) : (position.y / Chunk::Y),
			(position.z < 0) ? ((position.z + 1 - Chunk::Z) / Chunk::Z) : (position.z / Chunk::Z)
		); 
	}
	static Chunk::Coord toBlockCoord(const World::Coord& position) { return (Chunk::DIM + (position % Chunk::DIM)) % Chunk::DIM; }
	static World::Coord toWorldCoord(const Chunk::Coord& position) { return position * Chunk::DIM; }

public:
	World();

	void update();
	void render();

	Chunk* findChunk(const Chunk::Coord& position)
	{
		for (auto& chunk : chunks)
			if (*chunk == position)
				return chunk.get();
		return nullptr;
	}

private:
	std::vector<unique<Chunk>> chunks;
	struct Hash
	{
		size_t operator()(const Chunk::Coord& coord) const { return ((int64_t(coord.x) << 32) | coord.z) ^ (int64_t(coord.y) << 16); }
	};
	std::unordered_set<Chunk::Coord, Hash> queued_chunks;
	shared<Material> material = nullptr;
	shared<Image> texture_atlas = nullptr;
	shared<Buffer> index_buffer = nullptr;
	shared<Entity> camera = nullptr;
	ThreadPool pool = ThreadPool();
};