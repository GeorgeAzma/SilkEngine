#include "scene.h"
#include "entity.h"
#include "components.h"
#include "gfx/graphics.h"
#include "scene/resources.h"
#include "gfx/devices/logical_device.h"
#include "gfx/buffers/uniform_buffer.h"

Scene::Scene()
{
	Dispatcher::subscribe(this, &Scene::onWindowResize);
	registry.on_construct<RenderComponent>().connect<&Scene::onComponentCreate>(this);
}

Scene::~Scene()
{
	Dispatcher::unsubscribe(this, &Scene::onWindowResize);
	registry.on_construct<RenderComponent>().disconnect<&Scene::onComponentCreate>(this);
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

	indirect_batches = batchRenderedObjects(render_objects);

	indirect_buffer = std::make_shared<IndirectBuffer>(indirect_batches.size() * sizeof(VkDrawIndexedIndirectCommand));

	std::vector<VkDrawIndexedIndirectCommand> draw_commands;
	for (const auto& batch : indirect_batches)
	{
		VkDrawIndexedIndirectCommand draw_indexed_indirect_command{};
		draw_indexed_indirect_command.firstIndex = 0;
		draw_indexed_indirect_command.firstInstance = 0;
		draw_indexed_indirect_command.indexCount = batch.render_object.mesh->indices.size();
		draw_indexed_indirect_command.instanceCount = batch.count;
		draw_indexed_indirect_command.vertexOffset = 0;
		draw_commands.emplace_back(std::move(draw_indexed_indirect_command));
	}
	indirect_buffer->setData(draw_commands.data());

}

void Scene::onUpdate()
{
	registry.view<ScriptComponent>().each(
		[&](auto entity, auto& script_component)
		{
			script_component.instance->onUpdate();
		});

	//TODO: determine how to choose main camera
	CameraComponent* main_camera = nullptr;
	registry.view<CameraComponent>().each(
		[&](auto entity, auto& camera_component)
		{
			main_camera = &camera_component;
		});

	//TODO: not too scalable
	if (main_camera)
	{
		Graphics::global_uniform->setDataChecked(&main_camera->projection_view, sizeof(glm::mat4), 0);
	}

	Graphics::beginRenderPass();
	for (auto& batch : indirect_batches)
	{
		batch.render_object.material_data->material->pipeline->bind();
		batch.render_object.mesh->vertex_array->bind();
		batch.render_object.material_data->descriptor_set->bind();
		vkCmdDrawIndexedIndirect(Graphics::active.command_buffer, *indirect_buffer, batch.first * sizeof(VkDrawIndexedIndirectCommand), 1, sizeof(VkDrawIndexedIndirectCommand));
	}
	Graphics::endRenderPass();
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

void Scene::onComponentCreate(entt::registry& registry, entt::entity entity)
{
	auto& render_component = registry.get<RenderComponent>(entity);
	if (registry.any_of<TransformComponent>(entity))
	{
		InstanceData instance_data{};
		instance_data.transform = registry.get<TransformComponent>(entity).transform;
		render_component.render_object.instance_data = std::move(instance_data);
	}
	render_objects.push_back(render_component.render_object);
}

std::vector<IndirectBatch> Scene::batchRenderedObjects(const std::vector<RenderObject>& render_objects)
{
	std::vector<IndirectBatch> batches = { { render_objects.front(), 0, 0 } };

	for (size_t i = 0; i < render_objects.size(); ++i)
	{
		if (batches.back() == render_objects[i])
		{
			batches.back().instance_data.emplace_back(render_objects[i].instance_data);
			++batches.back().count;
		}
		else
		{
			batches.back().render_object.mesh->vertex_array->getVertexBuffer(1)->setData(batches.back().instance_data.data(), batches.back().instance_data.size() * sizeof(InstanceData));
			batches.emplace_back(render_objects[i], batches.size(), 1, std::vector<InstanceData>{ render_objects[i].instance_data });
			batches.back().instance_data.emplace_back(render_objects[i].instance_data);
		}
	}
	batches.back().render_object.mesh->vertex_array->getVertexBuffer(1)->setData(batches.back().instance_data.data(), batches.back().instance_data.size() * sizeof(InstanceData));

	return batches;
}
