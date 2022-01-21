#pragma once

#include "core/event.h"
#include "render_object.h"
#include "gfx/buffers/indirect_buffer.h"
#include "gfx/buffers/storage_buffer.h"

class Entity;

class Scene
{
	friend class Entity;
public:
	Scene();
	~Scene();

	void onPlay();
	void onUpdate();
	void onStop();

	Entity createEntity();

private:
	void onWindowResize(const WindowResizeEvent& e);
	void onComponentCreate(entt::registry& registry, entt::entity entity);

private:
	void batchRenderObjects();
	void addBatchRenderObject(const RenderObject& object);

private:
	entt::registry registry;
	std::vector<IndirectBatch> indirect_batches;
	std::vector<RenderObject> render_objects;
	std::shared_ptr<IndirectBuffer> indirect_buffer;
};