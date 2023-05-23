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

#define MULTITHREAD 0

void World::update()
{
	DebugTimer t;
	const vec3& origin = camera->get<CameraComponent>().camera.position;
	const Chunk::Coord& chunk_origin = toChunkCoord((World::Coord)round(origin));

	// Build chunks and regenerate chunks with new neighbors
#if MULTITHREAD
	pool.forEach(chunks.size(), [&](size_t i) { chunks[i]->generateMesh(); });
	RenderContext::getLogicalDevice().wait();
	for (const auto& chunk : chunks)
		chunk->buildVertexBuffer();
#else
	RenderContext::getLogicalDevice().wait();
	for (const auto& chunk : chunks)
	{
		chunk->generateMesh();
		chunk->buildVertexBuffer();
	}
#endif

	constexpr size_t max_chunks = 1024;
	if (chunks.size() >= max_chunks)
		return;

	// Queue up to be generated chunks
	std::vector<Chunk::Coord> old_queued_chunks(queued_chunks.begin(), queued_chunks.end());
	std::ranges::sort(old_queued_chunks, [&](const Chunk::Coord& lhs, const Chunk::Coord& rhs) { 
		constexpr vec3 modifier = vec3(1, 1, 1);
		return distance2(vec3(chunk_origin) * modifier, vec3(lhs) * modifier) < distance2(vec3(chunk_origin) * modifier, vec3(rhs) * modifier);
	});
	queued_chunks.clear();
	if (!findChunk(chunk_origin))
		queued_chunks.emplace(chunk_origin);
	constexpr size_t max_queued_chunks = 6;
	for (size_t i = 0; i < old_queued_chunks.size(); ++i)
	{
		const auto& old_queued_chunk = old_queued_chunks[i];
		std::array<Chunk::Coord, 6> neighbors = Chunk::getAdjacentNeighborCoords(old_queued_chunk);
		for (const auto& neighbor : neighbors)
		{
			if (findChunk(neighbor))
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
		chunks.emplace_back(makeUnique<Chunk>(chunk));
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
	push_constant_data.light_color = vec4(1.0);
	for (const auto& chunk : chunks)
	{
		if (!chunk->getVertexBuffer() || !camera->get<CameraComponent>().camera.frustum.isBoxVisible(toWorldCoord(chunk->getPosition()), toWorldCoord(chunk->getPosition()) + Chunk::DIM))
			continue;
		push_constant_data.chunk_position = ivec4(chunk->getPosition(), 0);
		RenderContext::getCommandBuffer().pushConstants(Shader::Stage::VERTEX, 0, sizeof(PushConstantData), &push_constant_data);
		chunk->getVertexBuffer()->bindVertex();
		RenderContext::getCommandBuffer().drawIndexed(chunk->getIndexCount());
	}
}