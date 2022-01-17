#include "scene.h"
#include "entity.h"
#include "components.h"
#include "gfx/graphics.h"

Scene::Scene()
{
	Dispatcher::subscribe(this, &Scene::onWindowResize);
}

Scene::~Scene()
{
	Dispatcher::unsubscribe(this, &Scene::onWindowResize);
}

void Scene::onPlay()
{
	registry.view<ScriptComponent>().each(
		[&](auto entity, auto& script_component)
		{
			if (!script_component.instance)
			{
				script_component.instance = script_component.instantiate_script();
				script_component.instance->entity = Entity{ entity, this };

				script_component.instance->onCreate();
			}
		});

	std::vector<std::shared_ptr<Mesh>> meshes;
	registry.view<MeshComponent>().each(
		[&](auto entity, auto& render_component, auto& mesh_component)
		{
			meshes.emplace_back(mesh_component.mesh->name, mesh_component.mesh);
		});

	indirect_batches = batchRenderedObjects(meshes);
}

void Scene::onUpdate()
{
	registry.view<ScriptComponent>().each(
		[&](auto entity, auto& script_component)
		{
			script_component.instance->onUpdate();
		});

	for (auto& batch : indirect_batches)
	{

	}
}

void Scene::onStop()
{
	registry.view<ScriptComponent>().each(
		[&](auto entity, auto& script_component)
		{
			script_component.instance->onDestroy();
		});
	registry.clear();
}

Entity Scene::createEntity()
{
	return { registry.create(), this };
}

void Scene::onWindowResize(const WindowResizeEvent& e)
{
	registry.view<CameraComponent>().each(
		[&](auto entity, auto& camera_component)
		{
			camera_component.onViewportResize();
		});
}

std::vector<IndirectBatch> Scene::batchRenderedObjects(const std::vector<std::shared_ptr<Mesh>>& meshes)
{
	std::vector<IndirectBatch> batches = { { meshes.front(), 0, 1 } };

	for (size_t i = 0; i < meshes.size(); ++i)
	{
		if (batches.back() == meshes[i])
		{
			++batches.back().count;
		}
		else
		{
			batches.emplace_back(meshes[i], i, 1);
		}
	}

	return batches;
}
