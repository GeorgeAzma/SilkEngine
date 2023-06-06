#include "chunk.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/buffers/buffer.h"
#include "silk_engine/gfx/pipeline/shader.h"
#include "silk_engine/gfx/pipeline/graphics_pipeline.h"
#include "silk_engine/gfx/pipeline/compute_pipeline.h"
#include "silk_engine/gfx/pipeline/material.h"
#include "silk_engine/utils/random.h"
#include "silk_engine/utils/debug_timer.h"
#include "silk_engine/gfx/descriptors/descriptor_set.h"
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
    fill = Block::ANY;
    block_buffer = makeShared<Buffer>(SHARED_VOLUME * sizeof(Block) + sizeof(fill), Buffer::STORAGE, Allocation::Props{ Allocation::RANDOM_ACCESS | Allocation::MAPPED });
    block_buffer->setData(&fill, sizeof(fill));
    gen_material->set("Blocks", *block_buffer);
    gen_material->bind();
    RenderContext::getCommandBuffer().pushConstants(ShaderStage::COMPUTE, 0, sizeof(position), &position);
    gen_material->dispatch(SIZE, SIZE, SIZE);
}

void Chunk::generateEnd()
{
    block_buffer->getData(&fill, sizeof(fill));
    if (fill == Block::NONE)
    {
        blocks.resize(SHARED_VOLUME, Block::STONE);
        block_buffer->getData(blocks.data(), SHARED_VOLUME * sizeof(Block), sizeof(fill));
    }
    if (fill != Block::NONE)
    {
        if (fill == Block::ANY)
            fill = Block::STONE;
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
    if (blocks.size() != SHARED_VOLUME)
        return;

    static DebugTimer t("Meshing");
    t.begin();

    for (size_t i = 0; i < 26; ++i) 
        updateNeighboringBlocks(i);

    thread_local std::vector<Vertex> vertices(MAX_VERTICES);
    thread_local std::vector<bool> visited(SHARED_VOLUME * 6);
    visited.assign(visited.size(), 0);
    for (size_t y = 0; y < SIZE; ++y)
    {
        for (size_t z = 0; z < SIZE; ++z)
        {
            for (size_t x = 0; x < SIZE; ++x)
            {
                size_t i = (y + 1) * SHARED_AREA + (z + 1) * SHARED_SIZE + (x + 1);
                Block block = blocks[i];
                if (block == Block::AIR)
                    continue;
            
                static constexpr int32_t axis[6] = { -SHARED_AREA, -SHARED_SIZE, -1, 1, SHARED_SIZE, SHARED_AREA };
                static constexpr int32_t greedy_axis[6] = { 1, 1, SHARED_SIZE, SHARED_SIZE, 1, 1 };
                size_t idx = y * AREA + z * SIZE + x;
                for (size_t face = 0; face < 6; ++face)
                {
                    if (BLOCK_SOLID[ecast(blocks[i + axis[face]])] || visited[i * 6 + face])
                        continue;
                    Vertex run_x = 1; 
                    for (; run_x < SIZE - x; ++run_x)
                    {
                        if (BLOCK_SOLID[ecast(blocks[i + run_x * greedy_axis[face] + axis[face]])])
                            break;
                        size_t ni = i + run_x * greedy_axis[face];
                        Block neighbor = blocks[ni];
                        if (neighbor != block)
                            break;
                        visited[ni * 6 + face] = true;
                    }
            
                    Vertex face_data = (face << 2) | (idx << 5) | (Vertex(BLOCK_TEXTURE_INDICES[size_t(block) * 6 + face]) << 23) | ((run_x - Vertex(1)) << 34);

                    vertices[vertex_count++] = 2 | face_data;
                    vertices[vertex_count++] = 1 | face_data;
                    vertices[vertex_count++] = 3 | face_data;
                    vertices[vertex_count++] = 3 | face_data;
                    vertices[vertex_count++] = 1 | face_data;
                    vertices[vertex_count++] = 0 | face_data;

                }
            }
        }
    }

    if (vertex_count)
    {
        size_t vertices_size = vertex_count * sizeof(Vertex);
        if (!vertex_buffer || vertices_size != vertex_buffer->getSize())
            vertex_buffer = makeShared<Buffer>(vertices_size, Buffer::VERTEX | Buffer::TRANSFER_DST);
        static std::mutex mux;
        std::scoped_lock lock(mux);
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
    RenderContext::getCommandBuffer().pushConstants(ShaderStage::VERTEX, 0, sizeof(position), &position);
    vertex_buffer->bindVertex();
    RenderContext::getCommandBuffer().draw(vertex_count);
}

void Chunk::updateNeighboringBlocks(size_t index)
{
    if (blocks.empty() || !neighbors[index])
        return;   

    switch (index)
    {
    case 0: blocks[0] = neighbors[index]->at(EDGE, EDGE, EDGE); break;
    case 1: for (uint32_t x = 0; x < SIZE; ++x) blocks[SHARED_EDGE * SHARED_AREA + SHARED_EDGE * SHARED_SIZE + (x + 1)] = neighbors[index]->at(x, 0, 0); break;
    case 2: blocks[SHARED_EDGE] = neighbors[index]->at(0, EDGE, EDGE); break;
    case 3: for (uint32_t z = 0; z < SIZE; ++z) blocks[SHARED_EDGE * SHARED_AREA + (z + 1) * SHARED_SIZE + SHARED_EDGE] = neighbors[index]->at(0, 0, z); break;
    case 4:
        for (uint32_t z = 0; z < SIZE; ++z)
            for (uint32_t x = 0; x < SIZE; ++x)
                blocks[(z + 1) * SHARED_SIZE + (x + 1)] = neighbors[index]->at(x, EDGE, z);
        break;
    case 5: for (uint32_t z = 0; z < SIZE; ++z) blocks[SHARED_EDGE * SHARED_AREA + (z + 1) * SHARED_SIZE] = neighbors[index]->at(EDGE, 0, z); break;
    case 6: blocks[SHARED_EDGE * SHARED_SIZE] = neighbors[index]->at(EDGE, EDGE, 0); break;
    case 7: for (uint32_t x = 0; x < SIZE; ++x) blocks[SHARED_EDGE * SHARED_AREA + (x + 1)] = neighbors[index]->at(x, 0, EDGE); break;
    case 8: blocks[SHARED_EDGE + SHARED_EDGE * SHARED_SIZE] = neighbors[index]->at(0, EDGE, 0); break;
    case 9: for (uint32_t y = 0; y < SIZE; ++y) blocks[(y + 1) * SHARED_AREA + SHARED_EDGE * SHARED_SIZE + SHARED_EDGE] = neighbors[index]->at(0, y, 0); break;
    case 10:
        for (uint32_t y = 0; y < SIZE; ++y)
            for (uint32_t x = 0; x < SIZE; ++x)
                blocks[(y + 1) * SHARED_AREA + (x + 1)] = neighbors[index]->at(x, y, EDGE);
        break;
    case 11: for (uint32_t y = 0; y < SIZE; ++y) blocks[(y + 1) * SHARED_AREA + SHARED_EDGE * SHARED_SIZE] = neighbors[index]->at(EDGE, y, 0); break;
    case 12:
        for (uint32_t y = 0; y < SIZE; ++y)
            for (uint32_t z = 0; z < SIZE; ++z)
                blocks[(y + 1) * SHARED_AREA + (z + 1) * SHARED_SIZE] = neighbors[index]->at(EDGE, y, z);
        break;
    case 13:
        for (uint32_t y = 0; y < SIZE; ++y)
            for (uint32_t z = 0; z < SIZE; ++z)
                blocks[(y + 1) * SHARED_AREA + (z + 1) * SHARED_SIZE + SHARED_EDGE] = neighbors[index]->at(0, y, z);
        break;
    case 14: for (uint32_t y = 0; y < SIZE; ++y) blocks[(y + 1) * SHARED_AREA + SHARED_EDGE] = neighbors[index]->at(0, y, EDGE); break;
    case 15:
        for (uint32_t y = 0; y < SIZE; ++y)
            for (uint32_t x = 0; x < SIZE; ++x)
                blocks[(y + 1) * SHARED_AREA + SHARED_EDGE * SHARED_SIZE + (x + 1)] = neighbors[index]->at(x, y, 0);
        break;
    case 16: for (uint32_t y = 0; y < SIZE; ++y) blocks[(y + 1) * SHARED_AREA] = neighbors[index]->at(EDGE, y, EDGE); break;
    case 17: blocks[SHARED_EDGE * SHARED_AREA] = neighbors[index]->at(EDGE, 0, EDGE); break;
    case 18: for (uint32_t x = 0; x < SIZE; ++x) blocks[SHARED_EDGE * SHARED_SIZE + (x + 1)] = neighbors[index]->at(x, EDGE, 0); break;
    case 19: blocks[SHARED_EDGE + SHARED_EDGE * SHARED_AREA] = neighbors[index]->at(0, 0, EDGE); break;
    case 20: for (uint32_t z = 0; z < SIZE; ++z) blocks[(z + 1) * SHARED_SIZE + SHARED_EDGE] = neighbors[index]->at(0, EDGE, z); break;
    case 21:
        for (uint32_t z = 0; z < SIZE; ++z)
            for (uint32_t x = 0; x < SIZE; ++x)
                blocks[SHARED_EDGE * SHARED_AREA + (z + 1) * SHARED_SIZE + (x + 1)] = neighbors[index]->at(x, 0, z);
        break;
    case 22: for (uint32_t z = 0; z < SIZE; ++z) blocks[(z + 1) * SHARED_SIZE] = neighbors[index]->at(EDGE, EDGE, z); break;
    case 23: blocks[SHARED_EDGE * SHARED_AREA + SHARED_EDGE * SHARED_SIZE] = neighbors[index]->at(EDGE, 0, 0); break;
    case 24: for (uint32_t x = 0; x < SIZE; ++x) blocks[x + 1] = neighbors[index]->at(x, EDGE, EDGE); break;
    case 25: blocks[SHARED_VOLUME - 1] = neighbors[index]->at(0, 0, 0); break;
    }
}
