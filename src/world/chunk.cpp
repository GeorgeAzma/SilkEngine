#include "chunk.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/buffers/buffer.h"
#include "silk_engine/gfx/pipeline/shader.h"
#include "silk_engine/gfx/pipeline/graphics_pipeline.h"
#include "silk_engine/gfx/pipeline/compute_pipeline.h"
#include "silk_engine/gfx/material.h"
#include "silk_engine/utils/random.h"
#include "silk_engine/utils/debug_timer.h"
#include "world.h"

Chunk::Chunk(const Coord& position)
    : position(position), gen_material(makeShared<Material>(ComputePipeline::get("Chunk Gen")))
{
}

Chunk::~Chunk()
{
    for (Chunk* neighbor : neighbors)
    {
        if (neighbor)
        {
            neighbor->neighbors[getNeighborIndexFromCoord(position - neighbor->getPosition())] = nullptr;
            neighbor->dirty = true;
        }
    }
}

void Chunk::generate()
{
    fill = Block::AIR;
#if 0
    std::vector<uint32_t> height_map;
    height_map.resize(AREA, 0);
    blocks.resize(VOLUME, Block::AIR);
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
#else
    static shared<Buffer> blocks_buffer = makeShared<Buffer>(SHARED_VOLUME * sizeof(Block) + sizeof(fill), Buffer::STORAGE | Buffer::TRANSFER_SRC);

    gen_material->set("Blocks", *blocks_buffer);
    gen_material->bind();
    ivec4 push_constant = ivec4(position, 0);
    RenderContext::getCommandBuffer().pushConstants(Shader::Stage::COMPUTE, 0, sizeof(push_constant), &push_constant);
    gen_material->dispatch(SIZE, SIZE, SIZE);
    RenderContext::executeCompute(); // TODO: defer this wait

    blocks.resize(SHARED_VOLUME, Block::AIR);
    blocks_buffer->getDataRanges({ { blocks.data(), SHARED_VOLUME * sizeof(Block) }, { &fill, sizeof(fill), SHARED_VOLUME * sizeof(Block) } });
#endif
    if (fill != Block::NONE)
        blocks.clear();
}

void Chunk::generateMesh()
{
    if (!dirty)
        return;
    dirty = false;
    vertex_count = 0;
    index_count = 0;
    if (fill != Block::NONE)
        return;

    static constexpr int32_t ao_table[6 * 4 * 3] =
    {
        -1 - SHARED_AREA,
         -SHARED_AREA - SHARED_SIZE,
        -1 - SHARED_AREA - SHARED_SIZE,
        -1 - SHARED_AREA,
         -SHARED_AREA + SHARED_SIZE,
        -1 - SHARED_AREA + SHARED_SIZE,
        1 - SHARED_AREA,
         -SHARED_AREA + SHARED_SIZE,
        1 - SHARED_AREA + SHARED_SIZE,
        1 - SHARED_AREA,
         -SHARED_AREA - SHARED_SIZE,
        1 - SHARED_AREA - SHARED_SIZE,
        -1 - SHARED_SIZE,
         +SHARED_AREA - SHARED_SIZE,
        -1 + SHARED_AREA - SHARED_SIZE,
        -1 - SHARED_SIZE,
         -SHARED_AREA - SHARED_SIZE,
        -1 - SHARED_AREA - SHARED_SIZE,
        1 - SHARED_SIZE,
         -SHARED_AREA - SHARED_SIZE,
        1 - SHARED_AREA - SHARED_SIZE,
        1 - SHARED_SIZE,
         +SHARED_AREA - SHARED_SIZE,
        1 + SHARED_AREA - SHARED_SIZE,
        -1 + SHARED_AREA,
        -1 + SHARED_SIZE,
        -1 + SHARED_AREA + SHARED_SIZE,
        -1 - SHARED_AREA,
        -1 + SHARED_SIZE,
        -1 - SHARED_AREA + SHARED_SIZE,
        -1 - SHARED_AREA,
        -1 - SHARED_SIZE,
        -1 - SHARED_AREA - SHARED_SIZE,
        -1 + SHARED_AREA,
        -1 - SHARED_SIZE,
        -1 + SHARED_AREA - SHARED_SIZE,
        1 + SHARED_AREA,
        1 - SHARED_SIZE,
        1 + SHARED_AREA - SHARED_SIZE,
        1 - SHARED_AREA,
        1 - SHARED_SIZE,
        1 - SHARED_AREA - SHARED_SIZE,
        1 - SHARED_AREA,
        1 + SHARED_SIZE,
        1 - SHARED_AREA + SHARED_SIZE,
        1 + SHARED_AREA,
        1 + SHARED_SIZE,
        1 + SHARED_AREA + SHARED_SIZE,
        1 + SHARED_SIZE,
         +SHARED_AREA + SHARED_SIZE,
        1 + SHARED_AREA + SHARED_SIZE,
        1 + SHARED_SIZE,
         -SHARED_AREA + SHARED_SIZE,
        1 - SHARED_AREA + SHARED_SIZE,
        -1 + SHARED_SIZE,
         -SHARED_AREA + SHARED_SIZE,
        -1 - SHARED_AREA + SHARED_SIZE,
        -1 + SHARED_SIZE,
         +SHARED_AREA + SHARED_SIZE,
        -1 + SHARED_AREA + SHARED_SIZE,
        -1 + SHARED_AREA,
         +SHARED_AREA + SHARED_SIZE,
        -1 + SHARED_AREA + SHARED_SIZE,
        -1 + SHARED_AREA,
         +SHARED_AREA - SHARED_SIZE,
        -1 + SHARED_AREA - SHARED_SIZE,
        1 + SHARED_AREA,
         +SHARED_AREA - SHARED_SIZE,
        1 + SHARED_AREA - SHARED_SIZE,
        1 + SHARED_AREA,
         +SHARED_AREA + SHARED_SIZE,
        1 + SHARED_AREA + SHARED_SIZE,
    };

    static thread_local std::vector<Vertex> vertices(Chunk::MAX_VERTICES);
    static thread_local std::vector<Index> indices(Chunk::MAX_INDICES);

    // Sides
    if (neighbors[4]) // (00, -1, 00)
        for (uint32_t z = 0; z < SIZE; ++z)
            for (uint32_t x = 0; x < SIZE; ++x)
                blocks[(z + 1) * SHARED_SIZE + (x + 1)] = neighbors[4]->at(x, EDGE, z);
    if (neighbors[10]) // (00, 00, -1)
        for (uint32_t y = 0; y < SIZE; ++y)
            for (uint32_t x = 0; x < SIZE; ++x)
                blocks[(y + 1) * SHARED_AREA + (x + 1)] = neighbors[10]->at(x, y, EDGE);
    if (neighbors[12]) // (-1, 00, 00)
        for (uint32_t y = 0; y < SIZE; ++y)
            for (uint32_t z = 0; z < SIZE; ++z)
                blocks[(y + 1) * SHARED_AREA + (z + 1) * SHARED_SIZE] = neighbors[12]->at(EDGE, y, z);
    if (neighbors[13]) // (+1, 00, 00)
        for (uint32_t y = 0; y < SIZE; ++y)
            for (uint32_t z = 0; z < SIZE; ++z)
                blocks[(y + 1) * SHARED_AREA + (z + 1) * SHARED_SIZE + SHARED_EDGE] = neighbors[13]->at(0, y, z);
    if (neighbors[15]) // (00, 00, +1)
        for (uint32_t y = 0; y < SIZE; ++y)
            for (uint32_t x = 0; x < SIZE; ++x)
                blocks[(y + 1) * SHARED_AREA + SHARED_EDGE * SHARED_SIZE + (x + 1)] = neighbors[15]->at(x, y, 0);
    if (neighbors[21]) // (00, +1, 00)
        for (uint32_t z = 0; z < SIZE; ++z)
            for (uint32_t x = 0; x < SIZE; ++x)
                blocks[SHARED_EDGE * SHARED_AREA + (z + 1) * SHARED_SIZE + (x + 1)] = neighbors[21]->at(x, 0, z);

    // Edges
    /* (00, -1, -1) */ if (neighbors[ 1]) for (uint32_t x = 0; x < SIZE; ++x) blocks[SHARED_EDGE * SHARED_AREA + SHARED_EDGE * SHARED_SIZE + (x + 1)] = neighbors[1]->at(x, 0, 0);
    /* (-1, -1, 00) */ if (neighbors[ 3]) for (uint32_t z = 0; z < SIZE; ++z) blocks[SHARED_EDGE * SHARED_AREA + (z + 1) * SHARED_SIZE + SHARED_EDGE] = neighbors[3]->at(0, 0, z);
    /* (+1, -1, 00) */ if (neighbors[ 5]) for (uint32_t z = 0; z < SIZE; ++z) blocks[SHARED_EDGE * SHARED_AREA + (z + 1) * SHARED_SIZE] = neighbors[5]->at(EDGE, 0, z);
    /* (00, -1, +1) */ if (neighbors[ 7]) for (uint32_t x = 0; x < SIZE; ++x) blocks[SHARED_EDGE * SHARED_AREA + (x + 1)] = neighbors[7]->at(x, 0, EDGE);
    /* (-1, 00, -1) */ if (neighbors[ 9]) for (uint32_t y = 0; y < SIZE; ++y) blocks[(y + 1) * SHARED_AREA + SHARED_EDGE * SHARED_SIZE + SHARED_EDGE] = neighbors[9]->at(0, y, 0);
    /* (+1, 00, -1) */ if (neighbors[11]) for (uint32_t y = 0; y < SIZE; ++y) blocks[(y + 1) * SHARED_AREA + SHARED_EDGE * SHARED_SIZE] = neighbors[11]->at(EDGE, y, 0);
    /* (-1, 00, +1) */ if (neighbors[14]) for (uint32_t y = 0; y < SIZE; ++y) blocks[(y + 1) * SHARED_AREA + SHARED_EDGE] = neighbors[14]->at(0, y, EDGE);
    /* (+1, 00, +1) */ if (neighbors[16]) for (uint32_t y = 0; y < SIZE; ++y) blocks[(y + 1) * SHARED_AREA] = neighbors[16]->at(EDGE, y, EDGE);
    /* (00, +1, -1) */ if (neighbors[18]) for (uint32_t x = 0; x < SIZE; ++x) blocks[SHARED_EDGE * SHARED_SIZE + (x + 1)] = neighbors[18]->at(x, EDGE, 0);
    /* (-1, +1, 00) */ if (neighbors[20]) for (uint32_t z = 0; z < SIZE; ++z) blocks[(z + 1) * SHARED_SIZE + SHARED_EDGE] = neighbors[20]->at(0, EDGE, z);
    /* (+1, +1, 00) */ if (neighbors[22]) for (uint32_t z = 0; z < SIZE; ++z) blocks[(z + 1) * SHARED_SIZE] = neighbors[22]->at(EDGE, EDGE, z);
    /* (00, +1, +1) */ if (neighbors[24]) for (uint32_t x = 0; x < SIZE; ++x) blocks[+(x + 1)] = neighbors[24]->at(x, EDGE, EDGE);

    // Corners
    /* (-1, -1, -1) */ if (neighbors[ 0]) blocks[0] = neighbors[0]->at(EDGE, EDGE, EDGE);
    /* (+1, -1, -1) */ if (neighbors[ 2]) blocks[SHARED_EDGE] = neighbors[2]->at(0, EDGE, EDGE);
    /* (-1, -1, +1) */ if (neighbors[ 6]) blocks[SHARED_EDGE * SHARED_SIZE] = neighbors[6]->at(EDGE, EDGE, 0);
    /* (+1, -1, +1) */ if (neighbors[ 8]) blocks[SHARED_EDGE + SHARED_EDGE * SHARED_SIZE] = neighbors[8]->at(0, EDGE, 0);
    /* (-1, +1, -1) */ if (neighbors[17]) blocks[SHARED_EDGE * SHARED_AREA] = neighbors[17]->at(EDGE, 0, EDGE);
    /* (+1, +1, -1) */ if (neighbors[19]) blocks[SHARED_EDGE + SHARED_EDGE * SHARED_AREA] = neighbors[19]->at(0, 0, EDGE);
    /* (-1, +1, +1) */ if (neighbors[23]) blocks[SHARED_EDGE * SHARED_AREA + SHARED_EDGE * SHARED_SIZE] = neighbors[23]->at(EDGE, 0, 0);
    /* (+1, +1, +1) */ if (neighbors[25]) blocks[SHARED_VOLUME - 1] = neighbors[25]->at(0, 0, 0);

    for (uint32_t y = 0; y < SIZE; ++y)
    {
        for (uint32_t z = 0; z < SIZE; ++z)
        {
            for (uint32_t x = 0; x < SIZE; ++x)
            {
                uint32_t i = (y + 1) * SHARED_AREA + (z + 1) * SHARED_SIZE + (x + 1);
                Block& block = blocks[i];
                if (block == Block::AIR)
                    continue;

                Block neighboring_blocks[6] =
                {
                    blocks[i - SHARED_AREA],
                    blocks[i - SHARED_SIZE],
                    blocks[i - 1],
                    blocks[i + 1],
                    blocks[i + SHARED_SIZE],
                    blocks[i + SHARED_AREA]
                };

                uint32_t idx = y * AREA + z * SIZE + x;
                for (uint32_t face = 0; face < 6; ++face)
                {
                    if (BlockInfo::isSolid(neighboring_blocks[face]))
                        continue;

                    uint64_t face_data = idx | (face << 17) | (BlockInfo::getTextureIndex(block, face) << 20);
                    uint32_t face12 = face * 12;
                    Vertex ao0 = getAO(BlockInfo::isSolid(blocks[i + ao_table[face12 +  0]]),
                                       BlockInfo::isSolid(blocks[i + ao_table[face12 +  1]]),
                                       BlockInfo::isSolid(blocks[i + ao_table[face12 +  2]]));
                    Vertex ao1 = getAO(BlockInfo::isSolid(blocks[i + ao_table[face12 +  3]]),
                                       BlockInfo::isSolid(blocks[i + ao_table[face12 +  4]]),
                                       BlockInfo::isSolid(blocks[i + ao_table[face12 +  5]]));
                    Vertex ao2 = getAO(BlockInfo::isSolid(blocks[i + ao_table[face12 +  6]]),
                                       BlockInfo::isSolid(blocks[i + ao_table[face12 +  7]]),
                                       BlockInfo::isSolid(blocks[i + ao_table[face12 +  8]]));
                    Vertex ao3 = getAO(BlockInfo::isSolid(blocks[i + ao_table[face12 +  9]]),
                                       BlockInfo::isSolid(blocks[i + ao_table[face12 + 10]]),
                                       BlockInfo::isSolid(blocks[i + ao_table[face12 + 11]]));

                    if (ao1 + ao3 > ao0 + ao2)
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

                    vertices[vertex_count++] = (0 << 15) | face_data | (ao0 << 28);
                    vertices[vertex_count++] = (1 << 15) | face_data | (ao1 << 28);
                    vertices[vertex_count++] = (2 << 15) | face_data | (ao2 << 28);
                    vertices[vertex_count++] = (3 << 15) | face_data | (ao3 << 28);

                }
            }
        }
    }

    if (vertex_count)
    {
        static std::mutex mux;
        std::scoped_lock lock(mux);
        size_t vertices_size = vertex_count * sizeof(Vertex);
        if (!(vertex_buffer && vertices_size == vertex_buffer->getSize()))
            vertex_buffer = makeShared<Buffer>(vertices_size, Buffer::VERTEX | Buffer::TRANSFER_DST | Buffer::TRANSFER_SRC);
        vertex_buffer->setData(vertices.data());

        size_t indices_size = index_count * sizeof(Index);
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
    vertex_buffer->bindVertex();
    index_buffer->bindIndex();
    RenderContext::getCommandBuffer().drawIndexed(index_count);
}