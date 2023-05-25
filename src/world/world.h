#pragma once

#include "chunk.h"
#include "silk_engine/utils/thread_pool.h"

class Material;
class Image;
class Buffer;
class Entity;
class Camera;

class World
{
public:
	using Coord = ivec3;

	struct PushConstantData
	{
		vec4 chunk_position;
		vec4 light_position;
		vec4 light_color;
	};

public:
	static Chunk::Coord toChunkCoord(const World::Coord& position) 
	{ 
		return glm::ivec3(
			(position.x < 0) ? ((position.x + 1 - Chunk::SIZE) / Chunk::SIZE) : (position.x / Chunk::SIZE),
			(position.y < 0) ? ((position.y + 1 - Chunk::SIZE) / Chunk::SIZE) : (position.y / Chunk::SIZE),
			(position.z < 0) ? ((position.z + 1 - Chunk::SIZE) / Chunk::SIZE) : (position.z / Chunk::SIZE)
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
	bool isChunkVisible(const Chunk::Coord& position) const;

private:
	std::vector<shared<Chunk>> chunks;
	struct Hash
	{
		size_t operator()(const Chunk::Coord& coord) const { return ((int64_t(coord.x) << 32) | coord.z) ^ (int64_t(coord.y) << 16); }
	};
	shared<Material> material = nullptr;
	shared<Image> texture_atlas = nullptr;
	shared<Buffer> index_buffer = nullptr;
	shared<Entity> player = nullptr;
	Camera* camera = nullptr;
	ThreadPool pool = ThreadPool();
};