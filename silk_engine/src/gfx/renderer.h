#pragma once

#include "scene/batch.h"

class Renderer
{
public:
	static void cleanup();

	static void beginBatch();
	static void endBatch();
	static void updateBatch();
	static void drawLastBatch();
	static void drawBatch(const std::vector<Batch>& batches);
	static void addBatch();

public:
	static inline Batcher batcher;
};