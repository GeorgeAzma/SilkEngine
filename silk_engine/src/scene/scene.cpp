#include "scene.h"
#include "entity.h"
#include "components.h"
#include "gfx/graphics.h"
#include "scene/resources.h"
#include "gfx/devices/logical_device.h"
#include "gfx/buffers/uniform_buffer.h"
#include "utils/general_utils.h"

Scene::Scene()
{
	Dispatcher::subscribe(this, &Scene::onWindowResize);
	registry.on_construct<RenderComponent>().connect<&Scene::onComponentCreate>(this);
	registry.on_destroy<RenderComponent>().connect<&Scene::onComponentDestroy>(this);
}

Scene::~Scene()
{
	Dispatcher::unsubscribe(this, &Scene::onWindowResize);
	registry.on_construct<RenderComponent>().disconnect<&Scene::onComponentCreate>(this);
	registry.on_destroy<RenderComponent>().disconnect<&Scene::onComponentDestroy>(this);
}

void Scene::onPlay()
{
	registry.view<ScriptComponent>().each(
		[&](auto entity, auto& script_component)
		{
			if (!script_component.instance)
			{
				script_component.instance = script_component.instantiate_script();
				script_component.instance->entity = makeShared<Entity>(entity, this);

				script_component.instance->onCreate();
			}
		});

	indirect_buffer = std::make_shared<IndirectBuffer>(Graphics::MAX_BATCHES * sizeof(VkDrawIndexedIndirectCommand));
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

	//batchRenderObjects();
	std::vector<VkDrawIndexedIndirectCommand> draw_commands;
	for (auto& batch : indirect_batches)
	{
		if (batch.needs_update && batch.instance_datas.size())
		{
			batch.render_object.mesh->vertex_array->getVertexBuffer(1)->setData(batch.instance_datas.data(), batch.instance_datas.size() * sizeof(InstanceData), 0);
			batch.needs_update = false;
		}
		VkDrawIndexedIndirectCommand draw_indexed_indirect_command{};
		draw_indexed_indirect_command.firstIndex = 0;
		draw_indexed_indirect_command.firstInstance = 0;
		draw_indexed_indirect_command.indexCount = batch.render_object.mesh->indices.size();
		draw_indexed_indirect_command.instanceCount = batch.instance_datas.size();
		draw_indexed_indirect_command.vertexOffset = 0;
		draw_commands.emplace_back(std::move(draw_indexed_indirect_command));
	}
	indirect_buffer->setData(draw_commands.data(), draw_commands.size() * sizeof(draw_commands[0]));

	Graphics::beginRenderPass();
	size_t index = 0;
	for (auto& batch : indirect_batches)
	{
		batch.render_object.material_data->material->pipeline->bind();
		batch.render_object.mesh->vertex_array->bind();
		batch.render_object.material_data->descriptor_set->bind();
		vkCmdDrawIndexedIndirect(Graphics::active.command_buffer, *indirect_buffer, index++ * sizeof(VkDrawIndexedIndirectCommand), 1, sizeof(VkDrawIndexedIndirectCommand));
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
}

shared<Entity> Scene::createEntity()
{
	return makeShared<Entity>(registry.create(), this);
}

void Scene::removeEntity(const entt::entity& entity)
{
	registry.destroy(entity);
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
	addBatchRenderObject(render_component.render_object);
}

void Scene::onComponentDestroy(entt::registry& registry, entt::entity entity) //TODO: Fix the error
{
	auto& render_component = registry.get<RenderComponent>(entity);
	removeBatchRenderObject(render_component.render_object);
}

void Scene::addBatchRenderObject(const RenderObject& render_object)
{
	for (size_t i = 0; i < indirect_batches.size(); ++i)
	{
		if (indirect_batches[i].render_object == render_object)
		{
			indirect_batches[i].instance_datas.emplace_back(render_object.instance_data);
			indirect_batches[i].needs_update = true;
			return;
		}
	}

	indirect_batches.emplace_back(render_object, std::vector<InstanceData>{render_object.instance_data});
}

void Scene::removeBatchRenderObject(const RenderObject& render_object)
{
	for (size_t i = 0; i < indirect_batches.size(); ++i)
	{
		if (indirect_batches[i].render_object == render_object)
		{
			for (size_t j = 0; j < indirect_batches[i].instance_datas.size(); ++j)
			{
				if (indirect_batches[i].instance_datas[j] == render_object.instance_data)
				{
					std::swap(indirect_batches[i].instance_datas[j], indirect_batches[i].instance_datas.back());
					indirect_batches[i].instance_datas.pop_back();
					indirect_batches[i].needs_update = true;
					return;
				}
			}
			return;
		}
	}
}
