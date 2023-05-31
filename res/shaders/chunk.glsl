uint idx(in uint x, in uint y, in uint z) { return (y + 1) * SHARED_AREA + (z + 1) * SHARED_SIZE + (x + 1); }
#define AT(x, y, z) blocks[idx(x, y, z)]
#define BLOCK AT(x, y, z)