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
	chunk_defines.emplace_back("SHARED_SIZE", std::to_string(Chunk::SHARED_SIZE));
	chunk_defines.emplace_back("SHARED_EDGE", std::to_string(Chunk::SHARED_EDGE));
	chunk_defines.emplace_back("SHARED_AREA", std::to_string(Chunk::SHARED_AREA));
	chunk_defines.emplace_back("SHARED_VOLUME", std::to_string(Chunk::SHARED_VOLUME));
	chunk_defines.emplace_back("SHARED_DIM", std::format("ivec3({}, {}, {})", std::to_string(Chunk::SHARED_DIM.x), std::to_string(Chunk::SHARED_DIM.y), std::to_string(Chunk::SHARED_DIM.z)));
	chunk_defines.emplace_back("TOTAL_BLOCKS", std::to_string(TOTAL_BLOCKS));
	for (size_t i = 0; i < TOTAL_BLOCKS; ++i)
		chunk_defines.emplace_back(BLOCK_NAMES[i], std::to_string(i));
	chunk_defines.emplace_back("NONE", std::to_string(uint32_t(Block::NONE)));

	VkRenderPass render_pass = RenderContext::getRenderGraph().getPass("Geometry").getRenderPass();
	shared<GraphicsPipeline> chunk_pipeline = makeShared<GraphicsPipeline>();
	chunk_pipeline->setShader(makeShared<Shader>("chunk", chunk_defines))
		.setRenderPass(render_pass)
		.setSamples(RenderContext::getPhysicalDevice().getMaxSampleCount())
		.setTopology(GraphicsPipeline::Topology::TRIANGLE)
		.enableTag(GraphicsPipeline::EnableTag::DEPTH_WRITE)
		.enableTag(GraphicsPipeline::EnableTag::DEPTH_TEST)
		//.enableTag(GraphicsPipeline::EnableTag::SAMPLE_SHADING)
		.setCullMode(GraphicsPipeline::CullMode::FRONT)
		.setDepthCompareOp(GraphicsPipeline::CompareOp::LESS);
	chunk_pipeline->build();
	GraphicsPipeline::add("Chunk", chunk_pipeline);
	material = makeShared<Material>(chunk_pipeline);

	Image::Props props{};
	props.sampler_props.mag_filter = VK_FILTER_NEAREST;
	props.sampler_props.min_filter = VK_FILTER_NEAREST;
	props.sampler_props.mipmap_mode = Sampler::MipmapMode::LINEAR;
	props.sampler_props.anisotropy = 0.0f;
	texture_atlas = makeShared<Image>(BLOCK_TEXTURES, props);
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
	constexpr float max_chunk_distance = 64;
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
	static DebugTimer t("Total Meshing");
	t.begin();
#if MULTITHREAD
	pool.forEach(chunks.size(), [&](size_t i) {
		chunks[i]->visible = isChunkVisible(chunks[i]->getPosition());
		if (!chunks[i]->visible)
			return;
		chunks[i]->generateMesh();
	});
	pool.wait();
#else
	for (size_t i = 0; i < chunks.size(); ++i)
	{
		chunks[i]->visible = isChunkVisible(chunks[i]->getPosition());
		if (!chunks[i]->visible)
			continue;
		chunks[i]->generateMesh();
	}
#endif
	t.end();

	static DebugTimer t1("Total Generating");
	static DebugTimer t2("Total Queueing");
	constexpr size_t max_chunks = 4096;
	if (chunks.size() < max_chunks)
	{
		// Queue up to be generated chunks
		t2.begin();
		std::ranges::sort(chunks, [&](const shared<Chunk>& lhs, const shared<Chunk>& rhs) {
			constexpr vec3 modifier = vec3(1, 2, 1);
			return ((100.0f + distance2(vec3(chunk_origin) * modifier, vec3(lhs->getPosition()) * modifier)) * ((lhs->getFill() != Block::NONE) * 256.0f + 1.0f)) <
				((100.0f + distance2(vec3(chunk_origin) * modifier, vec3(rhs->getPosition()) * modifier)) * ((lhs->getFill() != Block::NONE) * 256.0f + 1.0f));
			});

		std::unordered_set<Chunk::Coord> queued_chunks;
		if (!findChunk(chunk_origin))
			queued_chunks.emplace(chunk_origin);
		constexpr size_t max_queued_chunks = 8;
		for (const auto& chunk : chunks)
		{
			if (distance2(vec3(chunk_origin), vec3(chunk->getPosition())) > (max_chunk_distance2 - 1.0f) || !isChunkVisible(chunk->getPosition()))
				continue;
			std::vector<Chunk::Coord> missing_neighbors = chunk->getMissingAdjacentNeighborLocations();
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
		t2.end();
		t1.begin();
		for (const auto& chunk : queued_chunks)
		{
			chunks.emplace_back(makeShared<Chunk>(chunk));
			for (size_t i = 0; i < 26; ++i)
				chunks.back()->addNeighbor(i, findChunk(chunk + Chunk::NEIGHBORS[i]));
			chunks.back()->generateStart();
		}
		RenderContext::executeCompute();
		for (size_t i = chunks.size() - queued_chunks.size(); i < chunks.size(); ++i)
		{
			chunks[i]->generateEnd();
		}
		t1.end();
	}
	else if (!t.isStopped())
	{
		t.stop();
		t.print(t.getElapsed());
		t1.stop();
		t1.print(t1.getElapsed());
		t2.stop();
		t2.print(t2.getElapsed());
	}
	float w = float(Window::get().getWidth()) * 0.5f;
	float h = float(Window::get().getHeight()) * 0.5f;
	float s = 24.0f;
	const vec4& d = vec4(camera->direction, 0.0);
	const mat4& v = camera->view;
	vec4 dx = v * vec4(1, 0, 0, 0);
	vec4 dy = v * vec4(0, 1, 0, 0);
	vec4 dz = v * vec4(0, 0, 1, 0);
	//DebugRenderer::color({ 1.0f, 0.2f, 0.2f, 0.7f });
	//DebugRenderer::line(w, h, w + dx.x * s, h + dx.y * s, 2.0f);
	//DebugRenderer::color({ 0.2f, 1.0f, 0.2f, 0.7f });
	//DebugRenderer::line(w, h, w + dy.x * s, h + dy.y * s, 2.0f);
	//DebugRenderer::color({ 0.2f, 0.5f, 1.0f, 0.7f });
	//DebugRenderer::line(w, h, w + dz.x * s, h + dz.y * s, 2.0f);
}

void World::render()
{
	static DebugTimer t("Rendering");
	t.begin();
	material->set("GlobalUniform", *DebugRenderer::getGlobalUniformBuffer());
	material->set("texture_atlas", *texture_atlas);
	material->bind();
	for (const auto& chunk : chunks)
	{
		if (chunk->getVertexCount() == 0 || !chunk->visible)
			continue;
		chunk->render();
	}
	t.end();
	if (t.getSamples() >= 64)
	{
		t.print(t.getAverage());
		t.reset();
	}
}

bool World::isChunkVisible(const Chunk::Coord& position) const
{
	return camera->frustum.isBoxVisible(Chunk::toWorldCoord(position), Chunk::toWorldCoord(position) + Chunk::DIM);
}
