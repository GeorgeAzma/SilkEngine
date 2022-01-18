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

	std::vector<RenderObject> render_objects;
	registry.view<RenderComponent>().each(
		[this, &render_objects](auto entity, auto& render_component)
		{
			if (registry.any_of<TransformComponent>(entity))
			{
				InstanceData instance_data = { registry.get<TransformComponent>(entity).transform };
				render_component.render_object.mesh->vertex_array->getVertexBuffer(1)->setData(&instance_data, sizeof(InstanceData));
			}
			render_objects.push_back(render_component.render_object);
		});

	indirect_batches = batchRenderedObjects(render_objects);
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
		batch.render_object.material->graphics_pipeline->bind();
		batch.render_object.mesh->vertex_array->bind();
		batch.render_object.material->descriptor_set->bind();

		vkCmdDrawIndexed(Graphics::active.command_buffer, batch.render_object.mesh->indices.size(), batch.count, 0, 0, 0);
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

std::vector<IndirectBatch> Scene::batchRenderedObjects(const std::vector<RenderObject>& render_objects)
{
	std::vector<IndirectBatch> batches = { { render_objects.front(), 0, 1}};

	for (size_t i = 0; i < render_objects.size(); ++i)
	{
		if (batches.back() == render_objects[i])
		{
			++batches.back().count;
		}
		else
		{
			batches.emplace_back(render_objects[i], i, 1);
		}
	}

	return batches;
}
