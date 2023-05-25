#include "world.h"
#include "silk_engine/gfx/material.h"
#include "silk_engine/gfx/pipeline/graphics_pipeline.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/pipeline/render_graph/render_graph.h"
#include "silk_engine/gfx/pipeline/render_pass.h"
#include "silk_engine/gfx/debug_renderer.h"
#include "silk_engine/gfx/buffers/buffer.h"
#include "silk_engine/gfx/devices/logical_device.h"
#include "silk_engine/scene/camera/camera_controller.h"
#include "silk_engine/scene/components.h"
#include "silk_engine/utils/debug_timer.h"

World::World()
{
	player = Scene::getActive()->createEntity();
	player->add<CameraComponent>();
	player->add<ScriptComponent>().bind<CameraController>();
	player->get<CameraComponent>().camera.position = vec3(0.0f, 0.0f, 8.0f);
	camera = &player->get<CameraComponent>().camera;

	VkRenderPass render_pass = RenderContext::getRenderGraph().getPass("Geometry").getRenderPass();
	shared<GraphicsPipeline> chunk_pipeline = makeShared<GraphicsPipeline>();
	chunk_pipeline->setShader(makeShared<Shader>("chunk", Shader::Defines{ 
		{ "SIZE", std::to_string(Chunk::SIZE) },
		{ "AREA", std::to_string(Chunk::AREA) },
		{ "VOLUME", std::to_string(Chunk::VOLUME) } }))
		.setRenderPass(render_pass)
		.enableTag(GraphicsPipeline::EnableTag::DEPTH_WRITE)
		.enableTag(GraphicsPipeline::EnableTag::DEPTH_TEST)
		.setCullMode(GraphicsPipeline::CullMode::FRONT)
		.setDepthCompareOp(GraphicsPipeline::CompareOp::LESS);
	chunk_pipeline->build();
	material = makeShared<Material>(chunk_pipeline);
	Image::Props props{};
	props.sampler_props.mag_filter = VK_FILTER_NEAREST;
	props.sampler_props.min_filter = VK_FILTER_NEAREST;
	texture_atlas = makeShared<Image>(block_textures, props);
	texture_atlas->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	std::vector<uint32_t> indices(Chunk::MAX_INDICES);
	index_buffer = makeShared<Buffer>(indices.size() * sizeof(uint32_t), Buffer::INDEX | Buffer::TRANSFER_DST | Buffer::TRANSFER_SRC);
	constexpr uint32_t ind[6] = { 2, 1, 3, 3, 1, 0 };
	uint32_t index_offset = 0;
	for (uint32_t i = 0; i < Chunk::VOLUME * 6; ++i)
	{
		for (uint32_t j = 0; j < 6; ++j)
			indices[i * 6 + j] = ind[j] + index_offset;
		index_offset += 4;
	}
	index_buffer->setData(indices.data());
}

#define MULTITHREAD 1

void World::update()
{
	DebugTimer t;
	const vec3& origin = camera->position;
	const Chunk::Coord& chunk_origin = toChunkCoord((World::Coord)round(origin));

	// Delete far chunks
	RenderContext::getLogicalDevice().wait();
	constexpr float max_chunk_distance = 8;
	for (int32_t i = 0; i < chunks.size(); ++i)
	{
		if (distance2(vec3(chunks[i]->getPosition()), vec3(chunk_origin)) > max_chunk_distance * max_chunk_distance)
		{
			//std::swap(chunks[i], chunks.back());
			//chunks.pop_back();
			//--i;
		}
	}

	// Build chunks and regenerate chunks with new neighbors
#if MULTITHREAD
	pool.forEach(chunks.size(), [&](size_t i) {
#else
	for (size_t i = 0; i < chunks.size(); ++i)
#endif
		if (!isChunkVisible(chunks[i]->getPosition()))
			return;
		chunks[i]->generateMesh();
	}
#if MULTITHREAD
	);
#endif

	constexpr size_t max_chunks = 2048;
	if (chunks.size() >= max_chunks)
		return;

	// Queue up to be generated chunks
	std::ranges::sort(chunks, [&](const shared<Chunk>& lhs, const shared<Chunk>& rhs) {
		constexpr vec3 modifier = vec3(1, 1, 1);
		return distance(vec3(chunk_origin) * modifier, vec3(lhs->getPosition()) * modifier) * ((lhs->getFill() != Block::NONE) * 4 + 1) < 
			   distance(vec3(chunk_origin) * modifier, vec3(rhs->getPosition()) * modifier) * ((rhs->getFill() != Block::NONE) * 4 + 1);
	});

	std::vector<Chunk::Coord> queued_chunks;
	if (!findChunk(chunk_origin))
		queued_chunks.emplace_back(chunk_origin);
	constexpr size_t max_queued_chunks = 1;
	for (const auto& chunk : chunks)
	{
		if (!isChunkVisible(chunk->getPosition()))
			continue;
		const std::vector<Chunk::Coord> missing_neighbors = chunk->getMissingNeighborLocations();
		for (const auto& missing : missing_neighbors)
		{
			if (findChunk(chunk->getPosition() + missing))
				continue;
			queued_chunks.emplace_back(chunk->getPosition() + missing);
			if (queued_chunks.size() >= max_queued_chunks)
				break;
		}
		if (queued_chunks.size() >= max_queued_chunks)
			break;
	}

	for (const auto& chunk : queued_chunks)
	{
		chunks.emplace_back(makeShared<Chunk>(chunk));
		std::array<Chunk::Coord, 26> neighbors = Chunk::getNeighborCoords();
		for (size_t i = 0; i < neighbors.size(); ++i)
			chunks.back()->addNeighbor(i, findChunk(chunk + neighbors[i]));
#if MULTITHREAD
	}
	pool.forEach(queued_chunks.size(), [this](size_t i) { chunks[chunks.size() - 1 - i]->allocate(); chunks[chunks.size() - 1 - i]->generate(); });
#else
		chunks.back()->allocate(); 
		chunks.back()->generate();
	}
#endif
}

void World::render()
{
	material->set("GlobalUniform", *DebugRenderer::getGlobalUniformBuffer());
	material->set("texture_atlas", *texture_atlas);
	material->bind();
	index_buffer->bindIndex();
	for (const auto& chunk : chunks)
	{
		if (!isChunkVisible(chunk->getPosition()))
			continue;
		chunk->render();
	}
}

bool World::isChunkVisible(const Chunk::Coord& position) const
{
	return camera->frustum.isBoxVisible(toWorldCoord(position), toWorldCoord(position) + Chunk::DIM);
}
