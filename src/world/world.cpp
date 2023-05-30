#include "world.h"
#include "silk_engine/gfx/material.h"
#include "silk_engine/gfx/pipeline/graphics_pipeline.h"
#include "silk_engine/gfx/pipeline/compute_pipeline.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/pipeline/render_graph/render_graph.h"
#include "silk_engine/gfx/pipeline/render_pass.h"
#include "silk_engine/gfx/debug_renderer.h"
#include "silk_engine/gfx/devices/logical_device.h"
#include "silk_engine/scene/camera/camera_controller.h"
#include "silk_engine/scene/camera/camera.h"
#include "silk_engine/scene/components.h"
#include "silk_engine/utils/debug_timer.h"
#include "silk_engine/gfx/window/window.h"

World::World()
{
	player = Scene::getActive()->createEntity();
	player->add<CameraComponent>();
	player->add<ScriptComponent>().bind<CameraController>();
	player->get<CameraComponent>().camera.position = vec3(0.0f, 0.0f, 8.0f);
	camera = &player->get<CameraComponent>().camera;

	Shader::Defines chunk_defines{};
	chunk_defines.emplace_back("SIZE", std::to_string(Chunk::SIZE));
	chunk_defines.emplace_back("EDGE", std::to_string(Chunk::EDGE));
	chunk_defines.emplace_back("AREA", std::to_string(Chunk::AREA));
	chunk_defines.emplace_back("VOLUME", std::to_string(Chunk::VOLUME));
	chunk_defines.emplace_back("DIM", std::format("ivec3({}, {}, {})", std::to_string(Chunk::DIM.x), std::to_string(Chunk::DIM.y), std::to_string(Chunk::DIM.z)));
	for (size_t i = 0; i < TOTAL_BLOCKS; ++i)
		chunk_defines.emplace_back(block_names[i], std::to_string(i));
	chunk_defines.emplace_back("NONE", std::to_string(uint32_t(Block::NONE)));

	VkRenderPass render_pass = RenderContext::getRenderGraph().getPass("Geometry").getRenderPass();
	shared<GraphicsPipeline> chunk_pipeline = makeShared<GraphicsPipeline>();
	chunk_pipeline->setShader(makeShared<Shader>("chunk", chunk_defines))
		.setRenderPass(render_pass)
		.setSamples(RenderContext::getPhysicalDevice().getMaxSampleCount())
		.enableTag(GraphicsPipeline::EnableTag::DEPTH_WRITE)
		.enableTag(GraphicsPipeline::EnableTag::DEPTH_TEST)
		.setCullMode(GraphicsPipeline::CullMode::FRONT)
		.setDepthCompareOp(GraphicsPipeline::CompareOp::LESS);
	chunk_pipeline->build();
	GraphicsPipeline::add("Chunk", chunk_pipeline);
	material = makeShared<Material>(chunk_pipeline);

	Image::Props props{};
	props.sampler_props.mag_filter = VK_FILTER_NEAREST;
	props.sampler_props.min_filter = VK_FILTER_NEAREST;
	texture_atlas = makeShared<Image>(block_textures, props);
	texture_atlas->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	ComputePipeline::add("Chunk Gen", makeShared<ComputePipeline>(makeShared<Shader>("chunk_gen", chunk_defines)));
}

#define MULTITHREAD 1

void World::update()
{
	const vec3& origin = camera->position;
	const Chunk::Coord& chunk_origin = Chunk::toChunkCoord((Chunk::Coord)round(origin));

	// Delete far chunks
	RenderContext::getLogicalDevice().wait();
	constexpr float max_chunk_distance = 8;
	constexpr float max_chunk_distance2 = max_chunk_distance * max_chunk_distance;
	for (int32_t i = 0; i < chunks.size(); ++i)
	{
		if (distance2(vec3(chunks[i]->getPosition()), vec3(chunk_origin)) > max_chunk_distance2)
		{
			// TODO: When removing chunk, other chunks neighbors are made invalid, fix that
			//std::swap(chunks[i], chunks.back());
			//chunks.pop_back();
			//--i;
		}
	}

	// Build chunks and regenerate chunks with new neighbors
	static DebugTimer t;
	t.reset();
#if MULTITHREAD
	pool.forEach(chunks.size(), [&](size_t i) {
		if (!isChunkVisible(chunks[i]->getPosition()))
			return;
		chunks[i]->generateMesh();
	});
	pool.wait();
#else
	for (size_t i = 0; i < chunks.size(); ++i)
	{
		if (!isChunkVisible(chunks[i]->getPosition()))
			continue;
		chunks[i]->generateMesh();
	}
#endif

	constexpr size_t max_chunks = 16 * 16 * 16; // 4096
	if (chunks.size() < max_chunks)
	{
		// Queue up to be generated chunks
		std::ranges::sort(chunks, [&](const shared<Chunk>& lhs, const shared<Chunk>& rhs) {
			constexpr vec3 modifier = vec3(1, 1, 1);
			return ((100.0f + distance2(vec3(chunk_origin) * modifier, vec3(lhs->getPosition()) * modifier)) * ((lhs->getFill() != Block::NONE) * 256.0f + 1.0f)) <
				((100.0f + distance2(vec3(chunk_origin) * modifier, vec3(rhs->getPosition()) * modifier)) * ((lhs->getFill() != Block::NONE) * 256.0f + 1.0f));
			});

		std::unordered_set<Chunk::Coord> queued_chunks;
		if (!findChunk(chunk_origin))
			queued_chunks.emplace(chunk_origin);
		constexpr size_t max_queued_chunks = 4;
		for (const auto& chunk : chunks)
		{
			if (distance2(vec3(chunk_origin), vec3(chunk->getPosition())) > (max_chunk_distance2 - 1.0f) || !isChunkVisible(chunk->getPosition()))
				continue;
			const std::vector<Chunk::Coord> missing_neighbors = chunk->getMissingNeighborLocations();
			for (const auto& missing : missing_neighbors)
			{
				if (findChunk(chunk->getPosition() + missing))
					continue;
				queued_chunks.emplace(chunk->getPosition() + missing);
				if (queued_chunks.size() >= max_queued_chunks)
					break;
			}
			if (queued_chunks.size() >= max_queued_chunks)
				break;
		}

		for (const auto& chunk : queued_chunks)
		{
			chunks.emplace_back(makeShared<Chunk>(chunk));
			for (size_t i = 0; i < 26; ++i)
				chunks.back()->addNeighbor(i, findChunk(chunk + Chunk::NEIGHBORS[i]));
			chunks.back()->generate();
		}
	}
	t.sample(32);
	float w = float(Window::get().getWidth()) * 0.5f;
	float h = float(Window::get().getHeight()) * 0.5f;
	float s = 24.0f;
	const vec4& d = vec4(camera->direction, 0.0);
	const mat4& v = camera->view;
	vec4 dx = v * vec4(1, 0, 0, 0);
	vec4 dy = v * vec4(0, 1, 0, 0);
	vec4 dz = v * vec4(0, 0, 1, 0);
	DebugRenderer::color({ 1.0f, 0.2f, 0.2f, 0.7f });
	DebugRenderer::line(w, h, w + dx.x * s, h + dx.y * s, 2.0f);
	DebugRenderer::color({ 0.2f, 1.0f, 0.2f, 0.7f });
	DebugRenderer::line(w, h, w + dy.x * s, h + dy.y * s, 2.0f);
	DebugRenderer::color({ 0.2f, 0.5f, 1.0f, 0.7f });
	DebugRenderer::line(w, h, w + dz.x * s, h + dz.y * s, 2.0f);
}

void World::render()
{
	material->set("GlobalUniform", *DebugRenderer::getGlobalUniformBuffer());
	material->set("texture_atlas", *texture_atlas);
	material->bind();
	for (const auto& chunk : chunks)
	{
		if (chunk->getIndexCount() == 0 || !isChunkVisible(chunk->getPosition()))
			continue;
		chunk->render();
	}
}

bool World::isChunkVisible(const Chunk::Coord& position) const
{
	return camera->frustum.isBoxVisible(Chunk::toWorldCoord(position), Chunk::toWorldCoord(position) + Chunk::DIM);
}
