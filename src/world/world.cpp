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
	camera = Scene::getActive()->createEntity();
	camera->add<CameraComponent>();
	camera->add<ScriptComponent>().bind<CameraController>();
	camera->get<CameraComponent>().camera.position = vec3(0.0f, 0.0f, 8.0f);

	VkRenderPass render_pass = RenderContext::getRenderGraph().getPass("Geometry").getRenderPass();
	shared<GraphicsPipeline> chunk_pipeline = makeShared<GraphicsPipeline>();
	chunk_pipeline->setShader(makeShared<Shader>("chunk", Shader::Defines{ 
		{ "SIZE", std::to_string(Chunk::SIZE) }}))
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
	std::vector<uint32_t> indices(Chunk::VOLUME * 6 * 6);
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
	const vec3& origin = camera->get<CameraComponent>().camera.position;
	const Chunk::Coord& chunk_origin = toChunkCoord((World::Coord)round(origin));

	// Delete far chunks
	RenderContext::getLogicalDevice().wait();
	constexpr float max_chunk_distance = 8;
	for (size_t i = 0; i < chunks.size(); ++i)
	{
		auto& chunk = chunks[i];
		if (distance2(vec3(chunk->getPosition()), vec3(chunk_origin)) > max_chunk_distance * max_chunk_distance)
		{
			std::swap(chunk, chunks.back());
			chunks.pop_back();
		}
	}

	// Build chunks and regenerate chunks with new neighbors
#if MULTITHREAD
	pool.forEach(chunks.size(), [&](size_t i) {
		if (!isChunkVisible(chunks[i]->getPosition()))
			return;
		chunks[i]->generateMesh(); 
	});
#endif
	for (const auto& chunk : chunks)
	{
		if (!isChunkVisible(chunk->getPosition()))
			continue;
#if !MULTITHREAD
		chunk->generateMesh();
#endif
		chunk->buildVertexBuffer();
	}

	constexpr size_t max_chunks = 2048;
	if (chunks.size() >= max_chunks)
		return;

	// Queue up to be generated chunks
	std::vector<Chunk::Coord> old_queued_chunks(queued_chunks.begin(), queued_chunks.end());
	std::ranges::sort(old_queued_chunks, [&](const Chunk::Coord& lhs, const Chunk::Coord& rhs) { 
		constexpr vec3 modifier = vec3(1, 1, 1);
		return distance(vec3(chunk_origin) * modifier, vec3(lhs) * modifier) < distance(vec3(chunk_origin) * modifier, vec3(rhs) * modifier);
	});
	queued_chunks.clear();
	if (!findChunk(chunk_origin))
		queued_chunks.emplace(chunk_origin);
	constexpr size_t max_queued_chunks = 3;
	for (size_t i = 0; i < old_queued_chunks.size(); ++i)
	{
		const Chunk::Coord& old_queued_chunk = old_queued_chunks[i];
		std::array<Chunk::Coord, 6> neighbors_arr = Chunk::getAdjacentNeighborCoords(old_queued_chunk);
		std::vector<Chunk::Coord> neighbors(neighbors_arr.begin(), neighbors_arr.end());
		static Random rand;
		std::shuffle(neighbors.begin(), neighbors.end(), rand);
		for (const auto& neighbor : neighbors)
		{
			if (findChunk(neighbor) || !isChunkVisible(neighbor))
				continue;
			queued_chunks.emplace(neighbor);
			if (queued_chunks.size() >= max_queued_chunks)
				break;
		}
		if (queued_chunks.size() >= max_queued_chunks)
			break;
	}

	std::vector<Chunk::Coord> new_queued_chunks(queued_chunks.begin(), queued_chunks.end());
#if MULTITHREAD
	for (const auto& chunk : new_queued_chunks)
	{
		chunks.emplace_back(makeShared<Chunk>(chunk));
		std::array<Chunk::Coord, 26> neighbors = Chunk::getNeighborCoords(chunk);
		for (size_t i = 0; i < neighbors.size(); ++i)
			chunks.back()->addNeighbor(i, findChunk(neighbors[i]));
	}
	pool.forEach(new_queued_chunks.size(), [this](size_t i) { chunks[chunks.size() - 1 - i]->allocate(); chunks[chunks.size() - 1 - i]->generate(); });
#else
	for (const auto& chunk : new_queued_chunks)
	{
		chunks.emplace_back(makeUnique<Chunk>(chunk));
		std::array<Chunk::Coord, 26> neighbors = Chunk::getNeighborCoords(chunk);
		for (size_t i = 0; i < neighbors.size(); ++i)
			chunks.back()->addNeighbor(i, findChunk(neighbors[i])); 
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
	PushConstantData push_constant_data{};
	push_constant_data.light_position =  vec4(1000, 3000, -2000, 0);
	push_constant_data.light_color = vec4(0.8);
	for (const auto& chunk : chunks)
	{
		if (!chunk->getVertexBuffer() || !isChunkVisible(chunk->getPosition()))
			continue;
		push_constant_data.chunk_position = ivec4(chunk->getPosition(), 0);
		RenderContext::getCommandBuffer().pushConstants(Shader::Stage::VERTEX, 0, sizeof(PushConstantData), &push_constant_data);
		chunk->getVertexBuffer()->bindVertex();
		RenderContext::getCommandBuffer().drawIndexed(chunk->getIndexCount());
	}
}

bool World::isChunkVisible(const Chunk::Coord& position) const
{
	return camera->get<CameraComponent>().camera.frustum.isBoxVisible(toWorldCoord(position), toWorldCoord(position) + Chunk::DIM);
}
