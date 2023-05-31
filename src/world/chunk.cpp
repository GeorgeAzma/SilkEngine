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

void Chunk::generateStart()
{
    fill = Block::AIR;
    block_buffer = makeShared<Buffer>(SHARED_VOLUME * sizeof(Block) + sizeof(fill), Buffer::STORAGE, Allocation::Props{ Allocation::RANDOM_ACCESS });
    gen_material->set("Blocks", *block_buffer);
    gen_material->bind();
    ivec4 push_constant = ivec4(position, 0);
    RenderContext::getCommandBuffer().pushConstants(Shader::Stage::COMPUTE, 0, sizeof(push_constant), &push_constant);
    gen_material->dispatch(SIZE, SIZE, SIZE);
}

void Chunk::generateEnd()
{
    block_buffer->getData(&fill, sizeof(fill), SHARED_VOLUME * sizeof(Block));
    if (fill == Block::NONE)
    {
        blocks.resize(SHARED_VOLUME, Block::AIR);
        block_buffer->getData(blocks.data(), SHARED_VOLUME * sizeof(Block));
        fill = at(0, 0, 0);
        for (uint32_t y = 0; y < SIZE; ++y)
            for (uint32_t z = 0; z < SIZE; ++z)
                for (uint32_t x = 0; x < SIZE; ++x)
                {
                    if (at(x, y, z) != fill)
                    {
                        fill = Block::NONE;
                        goto down;
                    }
                }
        down:
        if (fill != Block::NONE)
            blocks.clear();
    }
    block_buffer = nullptr;
}

void Chunk::generateMesh()
{
    if (!dirty)
        return;
    dirty = false;
    vertex_count = 0;
    if (fill != Block::NONE || blocks.size() != SHARED_VOLUME)
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


    static DebugTimer t("Meshing");
    t.begin();
    // Sides
    if (isNeighborValid(4)) // (00, -1, 00)
        for (uint32_t z = 0; z < SIZE; ++z)
            for (uint32_t x = 0; x < SIZE; ++x)
                blocks[(z + 1) * SHARED_SIZE + (x + 1)] = neighbors[4]->at(x, EDGE, z);
    if (isNeighborValid(10)) // (00, 00, -1)
        for (uint32_t y = 0; y < SIZE; ++y)
            for (uint32_t x = 0; x < SIZE; ++x)
                blocks[(y + 1) * SHARED_AREA + (x + 1)] = neighbors[10]->at(x, y, EDGE);
    if (isNeighborValid(12)) // (-1, 00, 00)
        for (uint32_t y = 0; y < SIZE; ++y)
            for (uint32_t z = 0; z < SIZE; ++z)
                blocks[(y + 1) * SHARED_AREA + (z + 1) * SHARED_SIZE] = neighbors[12]->at(EDGE, y, z);
    if (isNeighborValid(13)) // (+1, 00, 00)
        for (uint32_t y = 0; y < SIZE; ++y)
            for (uint32_t z = 0; z < SIZE; ++z)
                blocks[(y + 1) * SHARED_AREA + (z + 1) * SHARED_SIZE + SHARED_EDGE] = neighbors[13]->at(0, y, z);
    if (isNeighborValid(15)) // (00, 00, +1)
        for (uint32_t y = 0; y < SIZE; ++y)
            for (uint32_t x = 0; x < SIZE; ++x)
                blocks[(y + 1) * SHARED_AREA + SHARED_EDGE * SHARED_SIZE + (x + 1)] = neighbors[15]->at(x, y, 0);
    if (isNeighborValid(21)) // (00, +1, 00)
        for (uint32_t z = 0; z < SIZE; ++z)
            for (uint32_t x = 0; x < SIZE; ++x)
                blocks[SHARED_EDGE * SHARED_AREA + (z + 1) * SHARED_SIZE + (x + 1)] = neighbors[21]->at(x, 0, z);

    // Edges
    /* (00, -1, -1) */ if (isNeighborValid( 1)) for (uint32_t x = 0; x < SIZE; ++x) blocks[SHARED_EDGE * SHARED_AREA + SHARED_EDGE * SHARED_SIZE + (x + 1)] = neighbors[1]->at(x, 0, 0);
    /* (-1, -1, 00) */ if (isNeighborValid( 3)) for (uint32_t z = 0; z < SIZE; ++z) blocks[SHARED_EDGE * SHARED_AREA + (z + 1) * SHARED_SIZE + SHARED_EDGE] = neighbors[3]->at(0, 0, z);
    /* (+1, -1, 00) */ if (isNeighborValid( 5)) for (uint32_t z = 0; z < SIZE; ++z) blocks[SHARED_EDGE * SHARED_AREA + (z + 1) * SHARED_SIZE] = neighbors[5]->at(EDGE, 0, z);
    /* (00, -1, +1) */ if (isNeighborValid( 7)) for (uint32_t x = 0; x < SIZE; ++x) blocks[SHARED_EDGE * SHARED_AREA + (x + 1)] = neighbors[7]->at(x, 0, EDGE);
    /* (-1, 00, -1) */ if (isNeighborValid( 9)) for (uint32_t y = 0; y < SIZE; ++y) blocks[(y + 1) * SHARED_AREA + SHARED_EDGE * SHARED_SIZE + SHARED_EDGE] = neighbors[9]->at(0, y, 0);
    /* (+1, 00, -1) */ if (isNeighborValid(11)) for (uint32_t y = 0; y < SIZE; ++y) blocks[(y + 1) * SHARED_AREA + SHARED_EDGE * SHARED_SIZE] = neighbors[11]->at(EDGE, y, 0);
    /* (-1, 00, +1) */ if (isNeighborValid(14)) for (uint32_t y = 0; y < SIZE; ++y) blocks[(y + 1) * SHARED_AREA + SHARED_EDGE] = neighbors[14]->at(0, y, EDGE);
    /* (+1, 00, +1) */ if (isNeighborValid(16)) for (uint32_t y = 0; y < SIZE; ++y) blocks[(y + 1) * SHARED_AREA] = neighbors[16]->at(EDGE, y, EDGE);
    /* (00, +1, -1) */ if (isNeighborValid(18)) for (uint32_t x = 0; x < SIZE; ++x) blocks[SHARED_EDGE * SHARED_SIZE + (x + 1)] = neighbors[18]->at(x, EDGE, 0);
    /* (-1, +1, 00) */ if (isNeighborValid(20)) for (uint32_t z = 0; z < SIZE; ++z) blocks[(z + 1) * SHARED_SIZE + SHARED_EDGE] = neighbors[20]->at(0, EDGE, z);
    /* (+1, +1, 00) */ if (isNeighborValid(22)) for (uint32_t z = 0; z < SIZE; ++z) blocks[(z + 1) * SHARED_SIZE] = neighbors[22]->at(EDGE, EDGE, z);
    /* (00, +1, +1) */ if (isNeighborValid(24)) for (uint32_t x = 0; x < SIZE; ++x) blocks[+(x + 1)] = neighbors[24]->at(x, EDGE, EDGE);

    // Corners
    /* (-1, -1, -1) */ if (isNeighborValid( 0)) blocks[0] = neighbors[0]->at(EDGE, EDGE, EDGE);
    /* (+1, -1, -1) */ if (isNeighborValid( 2)) blocks[SHARED_EDGE] = neighbors[2]->at(0, EDGE, EDGE);
    /* (-1, -1, +1) */ if (isNeighborValid( 6)) blocks[SHARED_EDGE * SHARED_SIZE] = neighbors[6]->at(EDGE, EDGE, 0);
    /* (+1, -1, +1) */ if (isNeighborValid( 8)) blocks[SHARED_EDGE + SHARED_EDGE * SHARED_SIZE] = neighbors[8]->at(0, EDGE, 0);
    /* (-1, +1, -1) */ if (isNeighborValid(17)) blocks[SHARED_EDGE * SHARED_AREA] = neighbors[17]->at(EDGE, 0, EDGE);
    /* (+1, +1, -1) */ if (isNeighborValid(19)) blocks[SHARED_EDGE + SHARED_EDGE * SHARED_AREA] = neighbors[19]->at(0, 0, EDGE);
    /* (-1, +1, +1) */ if (isNeighborValid(23)) blocks[SHARED_EDGE * SHARED_AREA + SHARED_EDGE * SHARED_SIZE] = neighbors[23]->at(EDGE, 0, 0);
    /* (+1, +1, +1) */ if (isNeighborValid(25)) blocks[SHARED_VOLUME - 1] = neighbors[25]->at(0, 0, 0);

    thread_local std::vector<Vertex> vertices(MAX_VERTICES);
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
                    if (block_solid[ecast(neighboring_blocks[face])])
                        continue;

                    uint32_t face_data = (face << 2) | (idx << 5) | (block_texture_indices[size_t(block) * 6 + face] << 23);
                    uint32_t face12 = face * 12;
                    Vertex ao0 = getAO(block_solid[ecast(blocks[i + ao_table[face12 +  0]])],
                                       block_solid[ecast(blocks[i + ao_table[face12 +  1]])],
                                       block_solid[ecast(blocks[i + ao_table[face12 +  2]])]);
                    Vertex ao1 = getAO(block_solid[ecast(blocks[i + ao_table[face12 +  3]])],
                                       block_solid[ecast(blocks[i + ao_table[face12 +  4]])],
                                       block_solid[ecast(blocks[i + ao_table[face12 +  5]])]);
                    Vertex ao2 = getAO(block_solid[ecast(blocks[i + ao_table[face12 +  6]])],
                                       block_solid[ecast(blocks[i + ao_table[face12 +  7]])],
                                       block_solid[ecast(blocks[i + ao_table[face12 +  8]])]);
                    Vertex ao3 = getAO(block_solid[ecast(blocks[i + ao_table[face12 +  9]])],
                                       block_solid[ecast(blocks[i + ao_table[face12 + 10]])],
                                       block_solid[ecast(blocks[i + ao_table[face12 + 11]])]);
                    
                    if (ao1 + ao3 > ao0 + ao2)
                    {
                        vertices[vertex_count++] = 2 | face_data | (ao2 << 32);
                        vertices[vertex_count++] = 1 | face_data | (ao1 << 32);
                        vertices[vertex_count++] = 3 | face_data | (ao3 << 32);
                        vertices[vertex_count++] = 3 | face_data | (ao3 << 32);
                        vertices[vertex_count++] = 1 | face_data | (ao1 << 32);
                        vertices[vertex_count++] = 0 | face_data | (ao0 << 32);
                    }
                    else
                    {
                        vertices[vertex_count++] = 2 | face_data | (ao2 << 32);
                        vertices[vertex_count++] = 1 | face_data | (ao1 << 32);
                        vertices[vertex_count++] = 0 | face_data | (ao0 << 32);
                        vertices[vertex_count++] = 0 | face_data | (ao0 << 32);
                        vertices[vertex_count++] = 3 | face_data | (ao3 << 32);
                        vertices[vertex_count++] = 2 | face_data | (ao2 << 32);
                    }
                }
            }
        }
    }
    //vertex_count = vertices.size();
    if (vertex_count)
    {
        //static std::mutex mux;
        //std::scoped_lock lock(mux);
        size_t vertices_size = vertex_count * sizeof(Vertex);
        if (!vertex_buffer || vertices_size > vertex_buffer->getSize())
            vertex_buffer = makeShared<Buffer>(vertices_size, Buffer::VERTEX | Buffer::TRANSFER_DST);
        vertex_buffer->setData(vertices.data());
    }
    else vertex_buffer = nullptr;
    t.end();
    if (t.getSamples() >= 64)
    {
        t.print(t.getAverage());
        t.reset();
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
    push_constant_data.light_color = vec4(1.0);
    push_constant_data.chunk_position = vec4(position, 0);
    RenderContext::getCommandBuffer().pushConstants(Shader::Stage::VERTEX, 0, sizeof(PushConstantData), &push_constant_data);
    vertex_buffer->bindVertex();
    RenderContext::getCommandBuffer().draw(vertex_count);
}