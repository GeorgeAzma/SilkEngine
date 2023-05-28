#include "chunk.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/buffers/buffer.h"
#include "silk_engine/gfx/pipeline/shader.h"
#include "silk_engine/gfx/material.h"
#include "silk_engine/utils/random.h"
#include "silk_engine/utils/debug_timer.h"
#include "world.h"

Chunk::Chunk(const Coord& position, const shared<Pipeline>& pipeline)
    : position(position), material(makeShared<Material>(pipeline))
{
}

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
            h *= h * h * h;
            height_map[z * SIZE + x] = 256.0f * h;
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
                    if (delta == 1)
                        block = Block::SNOW;
                    else if (delta <= 2)
                        block = Block::SNOW;
                    else
                        block = Block::SNOW;
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
    index_count = 0;
    
	static constexpr Chunk::Coord ao_table[6 * 4 * 3] =
    {
        ////////////////////////// FACE = Y- //////////////////////
        { -1, -1, 00 }, // Side 1
        { 00, -1, -1 }, // Side 2
        { -1, -1, -1 }, // Corner

        { -1, -1, 00 }, // Side 1
        { 00, -1, +1 }, // Side 2
        { -1, -1, +1 }, // Corner

        { +1, -1, 00 }, // Side 1
        { 00, -1, +1 }, // Side 2
        { +1, -1, +1 }, // Corner

        { +1, -1, 00 }, // Side 1
        { 00, -1, -1 }, // Side 2
        { +1, -1, -1 }, // Corner

        ////////////////////////// FACE = Z- //////////////////////
        { -1, 00, -1 }, // Side 1
        { 00, +1, -1 }, // Side 2
        { -1, +1, -1 }, // Corner

        { -1, 00, -1 }, // Side 1
        { 00, -1, -1 }, // Side 2
        { -1, -1, -1 }, // Corner

        { +1, 00, -1 }, // Side 1
        { 00, -1, -1 }, // Side 2
        { +1, -1, -1 }, // Corner

        { +1, 00, -1 }, // Side 1
        { 00, +1, -1 }, // Side 2
        { +1, +1, -1 }, // Corner

        ////////////////////////// FACE = X- //////////////////////
        { -1, +1, 00 }, // Side 1
        { -1, 00, +1 }, // Side 2
        { -1, +1, +1 }, // Corner

        { -1, -1, 00 }, // Side 1
        { -1, 00, +1 }, // Side 2
        { -1, -1, +1 }, // Corner

        { -1, -1, 00 }, // Side 1
        { -1, 00, -1 }, // Side 2
        { -1, -1, -1 }, // Corner

        { -1, +1, 00 }, // Side 1
        { -1, 00, -1 }, // Side 2
        { -1, +1, -1 }, // Corner

        ////////////////////////// FACE = X+ //////////////////////
        { +1, +1, 00 }, // Side 1
        { +1, 00, -1 }, // Side 2
        { +1, +1, -1 }, // Corner

        { +1, -1, 00 }, // Side 1
        { +1, 00, -1 }, // Side 2
        { +1, -1, -1 }, // Corner

        { +1, -1, 00 }, // Side 1
        { +1, 00, +1 }, // Side 2
        { +1, -1, +1 }, // Corner

        { +1, +1, 00 }, // Side 1
        { +1, 00, +1 }, // Side 2
        { +1, +1, +1 }, // Corner

        ////////////////////////// FACE = Z+ //////////////////////
        { +1, 00, +1 }, // Side 1
        { 00, +1, +1 }, // Side 2
        { +1, +1, +1 }, // Corner

        { +1, 00, +1 }, // Side 1
        { 00, -1, +1 }, // Side 2
        { +1, -1, +1 }, // Corner

        { -1, 00, +1 }, // Side 1
        { 00, -1, +1 }, // Side 2
        { -1, -1, +1 }, // Corner

        { -1, 00, +1 }, // Side 1
        { 00, +1, +1 }, // Side 2
        { -1, +1, +1 }, // Corner

        ////////////////////////// FACE = Y+ //////////////////////
        { -1, +1, 00 }, // Side 1
        { 00, +1, +1 }, // Side 2
        { -1, +1, +1 }, // Corner

        { -1, +1, 00 }, // Side 1
        { 00, +1, -1 }, // Side 2
        { -1, +1, -1 }, // Corner

        { +1, +1, 00 }, // Side 1
        { 00, +1, -1 }, // Side 2
        { +1, +1, -1 }, // Corner

        { +1, +1, 00 }, // Side 1
        { 00, +1, +1 }, // Side 2
        { +1, +1, +1 } // Corner
    };

    static thread_local std::vector<uint64_t> vertices(Chunk::MAX_VERTICES);
    static thread_local std::vector<uint32_t> indices(Chunk::MAX_INDICES);

    static DebugTimer t;
    t.reset();
    for (uint32_t y = 0; y < SIZE; ++y)
    {
        for (uint32_t z = 0; z < SIZE; ++z)
        {
            for (uint32_t x = 0; x < SIZE; ++x)
            {
                Chunk::Coord position(x, y, z);
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
                        uint32_t face12 = face * 12;
                        
                        uint64_t ao0 = getAO(BlockInfo::isSolid(atSafe(position + ao_table[face12 + 0])),
                                             BlockInfo::isSolid(atSafe(position + ao_table[face12 + 1])),
                                             BlockInfo::isSolid(atSafe(position + ao_table[face12 + 2])));
                       
                        uint64_t ao1 = getAO(BlockInfo::isSolid(atSafe(position + ao_table[face12 + 3])),
                                             BlockInfo::isSolid(atSafe(position + ao_table[face12 + 4])),
                                             BlockInfo::isSolid(atSafe(position + ao_table[face12 + 5])));
                        
                        uint64_t ao2 = getAO(BlockInfo::isSolid(atSafe(position + ao_table[face12 + 6])),
                                             BlockInfo::isSolid(atSafe(position + ao_table[face12 + 7])),
                                             BlockInfo::isSolid(atSafe(position + ao_table[face12 + 8])));
                        
                        uint64_t ao3 = getAO(BlockInfo::isSolid(atSafe(position + ao_table[face12 + 9])),
                                             BlockInfo::isSolid(atSafe(position + ao_table[face12 + 10])),
                                             BlockInfo::isSolid(atSafe(position + ao_table[face12 + 11])));
 
                        bool not_flipped = ao1 + ao3 < ao0 + ao2;
                        if (not_flipped)
                        {
                            indices[index_count++] = 2 + vertex_count;
                            indices[index_count++] = 1 + vertex_count;
                            indices[index_count++] = 3 + vertex_count;
                            indices[index_count++] = 3 + vertex_count;
                            indices[index_count++] = 1 + vertex_count;
                            indices[index_count++] = 0 + vertex_count;
                        }
                        else
                        {
                            indices[index_count++] = 2 + vertex_count;
                            indices[index_count++] = 1 + vertex_count;
                            indices[index_count++] = 0 + vertex_count;
                            indices[index_count++] = 0 + vertex_count;
                            indices[index_count++] = 3 + vertex_count;
                            indices[index_count++] = 2 + vertex_count;
                        }

                        vertices[vertex_count++] = i | (0 << 18) | face_data | (ao0 << 32);
                        vertices[vertex_count++] = i | (1 << 18) | face_data | (ao1 << 32);
                        vertices[vertex_count++] = i | (2 << 18) | face_data | (ao2 << 32);
                        vertices[vertex_count++] = i | (3 << 18) | face_data | (ao3 << 32);
                    }
                }
            }
        }
    }
    t.sample(64);
    
    if (vertex_count)
    {
        static std::mutex mux;
        std::scoped_lock lock(mux);

        size_t vertices_size = vertex_count * VERTEX_SIZE;
        if (!(vertex_buffer && vertices_size == vertex_buffer->getSize()))
            vertex_buffer = makeShared<Buffer>(vertices_size, Buffer::STORAGE | Buffer::TRANSFER_DST | Buffer::TRANSFER_SRC);
        vertex_buffer->setData(vertices.data());

        size_t indices_size = index_count * INDEX_SIZE;
        if (!(index_buffer && indices_size == index_buffer->getSize()))
            index_buffer = makeShared<Buffer>(indices_size, Buffer::INDEX | Buffer::TRANSFER_DST | Buffer::TRANSFER_SRC);
        index_buffer->setData(indices.data());
    }
    else
    {
        vertex_buffer = nullptr;
        index_buffer = nullptr;
    }
}

void Chunk::render() const
{
    if (!vertex_buffer)
        return;

    struct PushConstantData
    {
        vec4 chunk_position;
        vec4 light_position;
        vec4 light_color;
    };
    PushConstantData push_constant_data{};
    push_constant_data.light_position = vec4(100000, 300000, -200000, 0);
    push_constant_data.light_color = vec4(0.8);
    push_constant_data.chunk_position = vec4(position, 0);
    RenderContext::getCommandBuffer().pushConstants(Shader::Stage::VERTEX, 0, sizeof(PushConstantData), &push_constant_data);
    material->set("Vertices", *vertex_buffer);
    material->bind();
    index_buffer->bindIndex();
    RenderContext::getCommandBuffer().drawIndexed(getIndexCount());
}

Block Chunk::atSafe(const Chunk::Coord& position) const
{
    if (isInside(position))
        return at(position);
    Chunk::Coord neighbor_pos = World::toChunkCoord(position);
    Chunk* neighbor = neighbors[getIndexFromCoord(neighbor_pos)];
    if (neighbor)
        return neighbor->at(position - neighbor_pos * DIM);
    return Block::AIR;
}