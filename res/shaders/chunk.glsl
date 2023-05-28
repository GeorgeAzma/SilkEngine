uint idx(in uint x, in uint y, in uint z) { return z * SIZE + y * AREA + x; }
uint idx(in uvec3 position) { return position.z * SIZE + position.y * AREA + position.x; }
#define AT(x, y, z) blocks[idx(x, y, z)]
#define BLOCK blocks[idx(x, y, z)]