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

	size_t index = 0;
	registry.view<RenderComponent>().each(
		[this, &index](auto entity, auto& render_component)
		{
			if (registry.any_of<TransformComponent>(entity))
			{
				instance_data.emplace_back(registry.get<TransformComponent>(entity));
				render_component.render_object.mesh->vertex_array->getVertexBuffer(1)->setData(&instance_data.back(), sizeof(InstanceData), sizeof(InstanceData) * index);
			}
			render_objects.push_back(render_component.render_object);
			index++;
		});

	indirect_batches = batchRenderedObjects(render_objects);

	indirect_buffer = std::make_shared<IndirectBuffer>(65536 * sizeof(VkDrawIndexedIndirectCommand));
}

void Scene::onUpdate()
{
	registry.view<ScriptComponent>().each(
		[&](auto entity, auto& script_component)
		{
			script_component.instance->onUpdate();
		});

	//vkCmdDispatch(Graphics::active.command_buffer, render_objects.size() / 64 + (render_objects.size() % 64 != 0), 1, 1);	

	for (auto& batch : indirect_batches)
	{
		VkDrawIndexedIndirectCommand draw_indexed_indirect_command{};
		draw_indexed_indirect_command.firstIndex = 0;
		draw_indexed_indirect_command.firstInstance = 0;
		draw_indexed_indirect_command.indexCount = batch.render_object.mesh->indices.size();
		draw_indexed_indirect_command.instanceCount = batch.count;
		draw_indexed_indirect_command.vertexOffset = 0;

		batch.render_object.material_data->material->graphics_pipeline->bind();
		batch.render_object.mesh->vertex_array->bind();
		batch.render_object.material_data->descriptor_set->bind();

		vkCmdDrawIndexedIndirect(Graphics::active.command_buffer, *indirect_buffer, 0, batch.count, sizeof(VkDrawIndexedIndirectCommand));
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
	std::vector<IndirectBatch> batches = { { render_objects.front(), 0, 0}};

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
