#include "chunk.h"
#include "silk_engine/gfx/buffers/buffer.h"
#include "silk_engine/utils/random.h"
#include "world.h"

void Chunk::allocate()
{
	blocks.resize(VOLUME, Block::AIR);
}

void Chunk::generate()
{
	empty = true;
	for (int32_t z = 0; z < Z; ++z)
	{
		for (int32_t x = 0; x < X; ++x)
		{
			int height_map = 32.0f * Random::fbm(4, float(position.x * X + x) * 0.005f, float(position.z * Z + z) * 0.005f, 2.0f, 0.5f);
			for (int32_t y = 0; y < Y; ++y)
			{
				Block& block = at(x, y, z);
				int level = (position.y * Y + y);
				if (level < height_map)
				{
					block = Block::GRASS;
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
	for (uint32_t y = 0; y < Y; ++y)
	{
		for (uint32_t z = 0; z < Z; ++z)
		{
			for (uint32_t x = 0; x < X; ++x)
			{
				uint32_t i = idx(x, y, z);
				Block& block = blocks[i]; 
				if (block == Block::AIR)
					continue;
				
				const Block neighbor_blocks[6]
				{
					(y != EDGE_Y) ? blocks[i + X] : (neighbors[0] ? neighbors[0]->at(x, 0, z) : Block::AIR),
					(y != 0) ? blocks[i - X] : (neighbors[1] ? neighbors[1]->at(x, EDGE_Y, z) : Block::AIR),
					(x != 0) ? blocks[i - 1] : (neighbors[2] ? neighbors[2]->at(EDGE_X, y, z) : Block::AIR),
					(x != EDGE_X) ? blocks[i + 1] : (neighbors[3] ? neighbors[3]->at(0, y, z) : Block::AIR),
					(z != 0) ? blocks[i - AREA] : (neighbors[4] ? neighbors[4]->at(x, y, EDGE_Z) : Block::AIR),
					(z != EDGE_Z) ? blocks[i + AREA] : (neighbors[5] ? neighbors[5]->at(x, y, 0) : Block::AIR)
				};
				for (uint32_t face = 0; face < 6; ++face)
				{
					if (neighbor_blocks[face] == Block::AIR)
					{
						for (uint32_t vert = 0; vert < 4; ++vert)
						{
							vertices.emplace_back(x | (y << 5) | (z << 10) | (vert << 15) | (face << 17) |
								(block_data[size_t(block)].texture_indices[face] << 20));
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
		if (!(vertex_buffer && vertices.size() * sizeof(uint32_t) == vertex_buffer->getSize()))
			vertex_buffer = makeShared<Buffer>(vertices.size() * sizeof(uint32_t), Buffer::VERTEX | Buffer::TRANSFER_DST | Buffer::TRANSFER_SRC);
		vertex_buffer->setData(vertices.data());
	}
	else vertex_buffer = nullptr;
}

uint32_t Chunk::getVertexCount() const { return vertex_buffer->getSize() / sizeof(uint32_t); }