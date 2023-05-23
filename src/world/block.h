#pragma once

enum class Block : uint8_t
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
	SNOW
};

static inline constexpr auto block_solid = makeArray<bool>
(
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
);

static constexpr std::array<uint32_t, 6> TEX(uint32_t x)
{
	return { x, x, x, x, x, x };
}

static inline constexpr auto block_texture_indices = makeArray<std::array<uint32_t, 6>>
(
	/* AIR		 */  std::array<uint32_t, 6> { TEX(0) },
	/* STONE	 */  std::array<uint32_t, 6> { TEX(1) },
	/* GRASS	 */  std::array<uint32_t, 6> { { 4, 3, 3, 3, 3, 2 } },
	/* DIRT		 */  std::array<uint32_t, 6> { TEX(4) },
	/* SAND		 */  std::array<uint32_t, 6> { TEX(5) },
	/* SANDSTONE */  std::array<uint32_t, 6> { { 6, 6, 6, 6, 6, 7 } },
	/* WATER	 */  std::array<uint32_t, 6> { TEX(8) },
	/* LEAF		 */  std::array<uint32_t, 6> { TEX(9) },
	/* OAK_LOG	 */  std::array<uint32_t, 6> { { 11, 10, 10, 10, 10, 11 } },
	/* SNOW		 */  std::array<uint32_t, 6> { TEX(12) }
);

struct BlockInfo
{
	static bool isSolid(Block block) { return block_solid[size_t(block)]; }
	static uint32_t getTextureIndex(Block block, size_t face) { return block_texture_indices[size_t(block)][face]; }
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