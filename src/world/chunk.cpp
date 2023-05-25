#include "chunk.h"
#include "silk_engine/gfx/buffers/buffer.h"
#include "silk_engine/utils/random.h"
#include "silk_engine/utils/debug_timer.h"
#include "world.h"

Chunk::~Chunk()
{
    for (Chunk* neighbor : neighbors)
    {
        if (neighbor)
        {
            neighbor->neighbors[getIndexFromCoord(position - neighbor->getPosition())] = nullptr;
            neighbor->dirty = true;
        }
    }
}

void Chunk::allocate()
{
	blocks.resize(VOLUME, Block::AIR);
    height_map.resize(AREA, 0);
}

void Chunk::generate()
{
	fill = Block::AIR;
    height_map.resize(AREA, 0);
    for (int32_t z = 0; z < SIZE; ++z)
        for (int32_t x = 0; x < SIZE; ++x)
        {
            float h = Random::fbm(4, float(position.x * SIZE + x) * 0.005f, float(position.z * SIZE + z) * 0.005f, 2.0f, 0.5f) * 0.5f + 0.5f;
            h *= h;
            height_map[z * SIZE + x] = 64.0f * h;
        }

	for (int32_t y = 0; y < SIZE; ++y)
	{
        int32_t level = (position.y * SIZE + y);
        for (int32_t z = 0; z < SIZE; ++z)
        {
            for (int32_t x = 0; x < SIZE; ++x)
            {
				Block& block = at(x, y, z);
                int32_t height = height_map[z * SIZE + x];
                int32_t delta = height - level;
				if (level <= height)
				{
                    if (delta <= 1)
                        block = Block::GRASS;
                    else if (delta <= 3)
                        block = Block::DIRT;
                    else
                        block = Block::STONE;
					fill = Block::NONE;
				}
				else
				{
					block = Block::AIR;
				}
			}
		}
	}
}

void Chunk::generateMesh()
{
	if (!dirty)
		return;
	dirty = false;
	if (fill == Block::AIR)
		return;
    vertex_count = 0;

	static constexpr Chunk::Coord ao_table[6 * 4 * 3] =
    {
        ////////////////////////// FACE = X- //////////////////////
        // CORNER = [X-][Y-]
        { -1, -1, 00 }, // Side 1
        { -1, 00, -1 }, // Side 2
        { -1, -1, -1 }, // Corner
        // CORNER = [X+][Y-]
        { -1, +1, 00 }, // Side 1
        { -1, 00, -1 }, // Side 2
        { -1, +1, -1 }, // Corner
        // CORNER = [X-][Y+]
        { -1, +1, 00 }, // Side 1
        { -1, 00, +1 }, // Side 2
        { -1, +1, +1 }, // Corner
        // CORNER = [X+][Y+]
        { -1, 00, +1 }, // Side 1
        { -1, -1, 00 }, // Side 2
        { -1, -1, +1 }, // Corner

        ////////////////////////// FACE = X+ //////////////////////
        // CORNER = [X+][Y-]
        { +1, -1, 00 }, // Side 1
        { +1, 00, +1 }, // Side 2
        { +1, -1, +1 }, // Corner
        // CORNER = [X-][Y-]
        { +1, -1, 00 }, // Side 1
        { +1, 00, -1 }, // Side 2
        { +1, -1, -1 }, // Corner
        // CORNER = [X+][Y+]
        { +1, +1, 00 }, // Side 1
        { +1, 00, +1 }, // Side 2
        { +1, +1, +1 }, // Corner
        // CORNER = [X-][Y+]
        { +1, +1, 00 }, // Side 1
        { +1, 00, -1 }, // Side 2
        { +1, +1, -1 }, // Corner

        ////////////////////////// FACE = Y- //////////////////////
        // CORNER = [X-][Y-]
        { -1, -1, -1 }, // Side 1
        { 00, -1, -1 }, // Side 2
        { -1, -1, 00 }, // Corner
        // CORNER = [X+][Y-]
        { +1, -1, -1 }, // Side 1
        { 00, -1, -1 }, // Side 2
        { +1, -1, 00 }, // Corner
        // CORNER = [X-][Y+]
        { +1, -1, -1 }, // Side 1
        { 00, -1, +1 }, // Side 2
        { +1, -1, +1 }, // Corner
        // CORNER = [X+][Y+]
        { -1, -1, +1 }, // Side 1
        { -1, -1, 00 }, // Side 2
        { 00, -1, +1 }, // Corner

        ////////////////////////// FACE = Y+ //////////////////////
        // CORNER = [X-][Y+]
        { -1, +1, +1 }, // Side 1
        { 00, +1, -1 }, // Side 2
        { -1, +1, -1 }, // Corner
        // CORNER = [X+][Y+]
        { +1, +1, +1 }, // Side 1
        { 00, +1, -1 }, // Side 2
        { +1, +1, -1 }, // Corner
        // CORNER = [X-][Y-]
        { +1, +1, +1 }, // Side 1
        { 00, +1, +1 }, // Side 2
        { +1, +1, 00 }, // Corner
        // CORNER = [X+][Y-]
        { -1, +1, -1 }, // Side 1
        { -1, +1, 00 }, // Side 2
        { 00, +1, -1 }, // Corner

        ////////////////////////// FACE = Z- //////////////////////
        // CORNER = [X-][Y-]
        { -1, 00, -1 }, // Side 1
        { 00, -1, -1 }, // Side 2
        { -1, -1, -1 }, // Corner
        // CORNER = [X+][Y-]
        { +1, 00, -1 }, // Side 1
        { 00, -1, -1 }, // Side 2
        { +1, -1, -1 }, // Corner
        // CORNER = [X-][Y+]
        { +1, 00, -1 }, // Side 1
        { 00, +1, -1 }, // Side 2
        { +1, +1, -1 }, // Corner
        // CORNER = [X+][Y+]
        { -1, -1, -1 }, // Side 1
        { -1, 00, -1 }, // Side 2
        { 00, +1, -1 }, // Corner

        ////////////////////////// FACE = Z+ //////////////////////
        // CORNER = [X-][Y-]
        { -1, -1, +1 }, // Side 1
        { 00, -1, +1 }, // Side 2
        { -1, 00, +1 }, // Corner
        // CORNER = [X+][Y-]
        { +1, -1, +1 }, // Side 1
        { 00, -1, +1 }, // Side 2
        { +1, 00, +1 }, // Corner
        // CORNER = [X-][Y+]
        { +1, -1, +1 }, // Side 1
        { 00, +1, +1 }, // Side 2
        { +1, +1, +1 }, // Corner
        // CORNER = [X+][Y+]
        { -1, 00, +1 }, // Side 1
        { -1, -1, +1 }, // Side 2
        { 00, +1, +1 } // Corner
    };

    static thread_local std::vector<uint64_t> vertices(Chunk::MAX_VERTICES);
    for(uint32_t y = 0; y < SIZE; ++y)
    {
        for (uint32_t z = 0; z < SIZE; ++z)
        {
            for (uint32_t x = 0; x < SIZE; ++x)
            {
                uint32_t i = idx(x, y, z);
                Block& block = blocks[i];
                if (block == Block::AIR)
                    continue;
    
                Block neighboring_blocks[6] =
                {
                    (y != 0) ? blocks[i - AREA] : (neighbors[4] ? neighbors[4]->at(x, EDGE, z) : Block::AIR),
                    (z != 0) ? blocks[i - SIZE] : (neighbors[10] ? neighbors[10]->at(x, y, EDGE) : Block::AIR),
                    (x != 0) ? blocks[i - 1] : (neighbors[12] ? neighbors[12]->at(EDGE, y, z) : Block::AIR),
                    (x != EDGE) ? blocks[i + 1] : (neighbors[13] ? neighbors[13]->at(0, y, z) : Block::AIR),
                    (z != EDGE) ? blocks[i + SIZE] : (neighbors[15] ? neighbors[15]->at(x, y, 0) : Block::AIR),
                    (y != EDGE) ? blocks[i + AREA] : (neighbors[21] ? neighbors[21]->at(x, 0, z) : Block::AIR)
                };
                for (uint32_t face = 0; face < 6; ++face)
                {
                    if (!BlockInfo::isSolid(neighboring_blocks[face]))
                    {
                        uint32_t face_data = (face << 20) | (BlockInfo::getTextureIndex(block, face) << 23);
                        for (uint32_t vert = 0; vert < 4; ++vert)
                            vertices[vertex_count++] = i | (vert << 18) | face_data;
                    }
                }
            }
        }
    }

    if (vertex_count)
    {
        static std::mutex mux;
        std::scoped_lock lock(mux);
        if (!(vertex_buffer && vertex_count * VERTEX_SIZE == vertex_buffer->getSize()))
            vertex_buffer = makeShared<Buffer>(vertex_count * VERTEX_SIZE, Buffer::VERTEX | Buffer::TRANSFER_DST | Buffer::TRANSFER_SRC);
        vertex_buffer->setData(vertices.data());
    }
    else vertex_buffer = nullptr;
}

Block Chunk::atSafe(const Chunk::Coord& position) const
{
    Chunk::Coord chunk_pos = World::toChunkCoord(position);
    if (chunk_pos == Chunk::Coord(0))
        return at(position);
    Chunk* neighbor = neighbors[getIndexFromCoord(chunk_pos)];
    if (neighbor)
        return neighbor->at(position - chunk_pos * DIM);
    return Block::AIR;
}