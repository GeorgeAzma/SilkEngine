#pragma once

enum class Block : uint32_t
{
	AIR = 0,
	STONE,
	GRASS,
	DIRT,
	SAND,
	SANDSTONE,
	WATER,
	LEAF,
	OAK_LOG,
	SNOW,

	LAST,
	NONE = std::numeric_limits<std::underlying_type_t<Block>>::max()
};
static constexpr size_t TOTAL_BLOCKS = ecast(Block::LAST);

static constexpr const char* block_names[TOTAL_BLOCKS]
{
	"AIR",
	"STONE",
	"GRASS",
	"DIRT",
	"SAND",
	"SANDSTONE",
	"WATER",
	"LEAF",
	"OAK_LOG",
	"SNOW"
};

static constexpr bool block_solid[TOTAL_BLOCKS]
{
	/* AIR		 */  false,
	/* STONE	 */  true,
	/* GRASS	 */  true,
	/* DIRT		 */  true,
	/* SAND		 */  true,
	/* SANDSTONE */  true,
	/* WATER	 */  false,
	/* LEAF		 */  false,
	/* OAK_LOG	 */  true,
	/* SNOW		 */  true
};

static constexpr uint32_t block_texture_indices[TOTAL_BLOCKS * 6]
{
	/* AIR		 */  0, 0, 0, 0, 0, 0,
	/* STONE	 */  1, 1, 1, 1, 1, 1,
	/* GRASS	 */  4, 3, 3, 3, 3, 2,
	/* DIRT		 */  4, 4, 4, 4, 4, 4,
	/* SAND		 */  5, 5, 5, 5, 5, 5,
	/* SANDSTONE */  6, 6, 6, 6, 6, 7,
	/* WATER	 */  8, 8, 8, 8, 8, 8,
	/* LEAF		 */  9, 9, 9, 9, 9, 9,
	/* OAK_LOG	 */  11, 10, 10, 10, 10, 11,
	/* SNOW		 */  12, 12, 12, 12, 12, 12
};

struct BlockInfo
{
	static bool isSolid(Block block) { return block_solid[ecast(block)]; }
	static uint32_t getTextureIndex(Block block, size_t face) { return block_texture_indices[size_t(block) * 6 + face]; }
};

static inline const auto block_textures = makeArray<fs::path>
(
	"res/images/blocks/Air.png",
	"res/images/blocks/Stone.png",
	"res/images/blocks/Grass.png",
	"res/images/blocks/GrassSide.png",
	"res/images/blocks/Dirt.png",
	"res/images/blocks/Sand.png",
	"res/images/blocks/Sandstone.png",
	"res/images/blocks/SandstoneTop.png",
	"res/images/blocks/Water.png",
	"res/images/blocks/Leaf.png",
	"res/images/blocks/OakLog.png",
	"res/images/blocks/OakTop.png",
	"res/images/blocks/SnowBlock.png"
);