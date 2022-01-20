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

private:
	static std::vector<IndirectBatch> batchRenderedObjects(const std::vector<RenderObject>& render_object); //TODO: Support materials uniform buffers layouts etc, so convert mesh shared ptr to RenderObject class

private:
	entt::registry registry;
	std::vector<IndirectBatch> indirect_batches;
	std::vector<RenderObject> render_objects;
	std::vector<InstanceData> instance_data;
	std::shared_ptr<IndirectBuffer> indirect_buffer;
	std::shared_ptr<StorageBuffer> instance_buffer;
};