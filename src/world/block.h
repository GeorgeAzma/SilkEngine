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

struct BlockData
{
	bool culled;
	std::array<uint32_t, 6> texture_indices; // Top, Bottom, Left, Right, Front, Back
};

constexpr std::array<uint32_t, 6> TEX(uint32_t x)
{
	return { x, x, x, x, x, x };
}

static inline const auto block_data = makeArray<BlockData>
(
	 /* AIR			*/   BlockData{ false, TEX(0) },						  
	 /* STONE		*/   BlockData{ false, TEX(1) },						  
	 /* GRASS		*/   BlockData{ false, { 2, 4, 3, 3, 3, 3 } },		  
	 /* DIRT		*/   BlockData{ false, TEX(4) },						  
	 /* SAND		*/   BlockData{ false, TEX(5) },						  
	 /* SANDSTONE   */   BlockData{ false, { 7, 6, 6, 6, 6, 6 } },		  
	 /* WATER		*/   BlockData{ false, TEX(8) },						  
	 /* LEAF		*/   BlockData{ false, TEX(9) },						  
	 /* OAK_LOG		*/   BlockData{ false, { 11, 11, 10, 10, 10, 10 } },	  
	 /* SNOW		*/   BlockData{ false, TEX(12) }					  
);

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