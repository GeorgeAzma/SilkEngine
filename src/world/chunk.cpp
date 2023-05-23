#include "chunk.h"
#include "silk_engine/gfx/buffers/buffer.h"
#include "silk_engine/utils/random.h"
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
	empty = true;
    height_map.resize(AREA, 0);
    for (int32_t z = 0; z < SIZE; ++z)
        for (int32_t x = 0; x < SIZE; ++x)
            height_map[z * SIZE + x] = 32.0f * Random::fbm(4, float(position.x * SIZE + x) * 0.005f, float(position.z * SIZE + z) * 0.005f, 2.0f, 0.5f);

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
					empty = false;
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
	vertices.clear();
	if (empty)
		return;
    vertex_buffer_dirty = true;

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
	
	for (uint32_t y = 0; y < SIZE; ++y)
	{
		for (uint32_t z = 0; z < SIZE; ++z)
		{
			for (uint32_t x = 0; x < SIZE; ++x)
			{
				size_t i = idx(x, y, z);
				Block& block = blocks[i]; 
				if (block == Block::AIR)
					continue;
                
                Chunk::Coord position = Chunk::Coord(x, y, z);
                const auto& neighboring_blocks = getAdjacentNeighborBlocks(position);
				for (uint32_t face = 0; face < 6; ++face)
				{
					if (!BlockInfo::isSolid(neighboring_blocks[face]))
					{
						for (uint32_t vert = 0; vert < 4; ++vert)
						{
                            //const Chunk::Coord& side1_off  = position + ao_table[face * 4 * 3 + vert * 3 + 0];
                            //const Chunk::Coord& side2_off  = position + ao_table[face * 4 * 3 + vert * 3 + 1];
                            //const Chunk::Coord& corner_off = position + ao_table[face * 4 * 3 + vert * 3 + 2];
                            //uint32_t ao = getAO(BlockInfo::isSolid(at(side1_off)), BlockInfo::isSolid(at(side2_off)), BlockInfo::isSolid(at(corner_off)));
							vertices.emplace_back(x | (y << 6) | (z << 12) | (vert << 18) | (face << 20) | (BlockInfo::getTextureIndex(block, face) << 23));
						}
					}
				}
			}
		}
	}
}

void Chunk::buildVertexBuffer()
{
	if (!vertex_buffer_dirty)
		return;
	vertex_buffer_dirty = false;
	if (vertices.size())
	{
		if (!(vertex_buffer && vertices.size() * sizeof(vertices[0]) == vertex_buffer->getSize()))
			vertex_buffer = makeShared<Buffer>(vertices.size() * sizeof(vertices[0]), Buffer::VERTEX | Buffer::TRANSFER_DST | Buffer::TRANSFER_SRC);
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

uint32_t Chunk::getVertexCount() const { return vertex_buffer->getSize() / sizeof(uint32_t); }