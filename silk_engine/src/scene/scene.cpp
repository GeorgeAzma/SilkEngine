#include "scene.h"
#include "entity.h"
#include "components.h"
#include "scene/resources.h"
#include "gfx/devices/logical_device.h"
#include "gfx/devices/physical_device.h"
#include "gfx/buffers/uniform_buffer.h"
#include "utils/general_utils.h"
#include "core/time.h"
#include "gfx/window/swap_chain.h"

Scene::Scene()
{
	Dispatcher::subscribe(this, &Scene::onWindowResize);
	
	registry.on_construct<MeshComponent>().connect<&Scene::onMeshComponentCreate>(this);
	registry.on_construct<ModelComponent>().connect<&Scene::onModelComponentCreate>(this);
	registry.on_construct<LightComponent>().connect<&Scene::onLightComponentCreate>(this);
	
	registry.on_destroy<MeshComponent>().connect<&Scene::onMeshComponentDestroy>(this);
	registry.on_destroy<ModelComponent>().connect<&Scene::onModelComponentDestroy>(this);
	registry.on_destroy<LightComponent>().connect<&Scene::onLightComponentDestroy>(this);
	instance_batches.reserve(Graphics::MAX_INSTANCE_BATCHES); //TODO: remove this limitation some time
	material_data_3D = makeShared<Material>(Resources::getShaderEffect("3D"), std::vector<shared<DescriptorSet>>{ Resources::getDescriptorSet("Global"), Resources::getDescriptorSet("Images") });

	indexed_indirect_buffer = makeShared<IndirectBuffer>(Graphics::MAX_INSTANCE_BATCHES * sizeof(VkDrawIndexedIndirectCommand));
	indirect_buffer = makeShared<IndirectBuffer>(Graphics::MAX_INSTANCE_BATCHES * sizeof(VkDrawIndirectCommand));
}

Scene::~Scene()
{
	onStop();
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
}

void Scene::onUpdate()
{
	//Update components
	registry.view<ScriptComponent>().each(
		[&](auto entity, auto& script_component)
		{
			script_component.instance->onUpdate();
		});

	CameraComponent* main_camera = nullptr;
	registry.view<CameraComponent>().each(
		[&](auto entity, auto& camera_component)
		{
			main_camera = &camera_component;
		});

	Graphics::GlobalUniformData global_uniform_data{};
	if (main_camera)
	{
		global_uniform_data.projection_view = main_camera->camera.projection_view;
		global_uniform_data.camera_position = main_camera->camera.position;
		global_uniform_data.camera_direction = main_camera->camera.direction;
	}
	global_uniform_data.delta_time = Time::dt;
	global_uniform_data.time = Time::runtime;
	global_uniform_data.frame = Time::frame;
	global_uniform_data.resolution = glm::uvec2(Window::getWidth(), Window::getHeight());
	global_uniform_data.light_count = light_index;
	global_uniform_data.lights = lights;

	Graphics::global_uniform->setDataChecked(&global_uniform_data, sizeof(Graphics::GlobalUniformData));

	//Drawing

	std::vector<VkDrawIndexedIndirectCommand> indexed_draw_commands;
	std::vector<VkDrawIndirectCommand> draw_commands;
	
	for (auto& instance_batch : instance_batches)
	{
		if (instance_batch.instance->mesh->vertex_array->hasIndexBuffer())
		{
			VkDrawIndexedIndirectCommand indexed_draw_command{};
			indexed_draw_command.instanceCount = instance_batch.instance_data.size();
			indexed_draw_command.indexCount = instance_batch.instance->mesh->indices.size();
			indexed_draw_commands.emplace_back(indexed_draw_command);
		}
		else
		{
			VkDrawIndirectCommand draw_command{};
			draw_command.instanceCount = instance_batch.instance_data.size();
			draw_command.vertexCount = instance_batch.instance->mesh->indices.size();
			draw_commands.emplace_back(draw_command);
		}
	}
	indexed_indirect_buffer->setDataChecked(indexed_draw_commands.data(), indexed_draw_commands.size() * sizeof(VkDrawIndexedIndirectCommand));
	indirect_buffer->setDataChecked(draw_commands.data(), draw_commands.size() * sizeof(VkDrawIndirectCommand));

	//Draw instances
	Graphics::swap_chain->beginRenderPass();

	size_t indexed_draw_index = 0;
	size_t draw_index = 0;
	for (auto& instance_batch : instance_batches)
	{
		if (instance_batch.instance_data.empty())
			continue;

		if (instance_batch.needs_update)
		{
			instance_batch.instance_buffer->setData(instance_batch.instance_data.data(), instance_batch.instance_data.size() * sizeof(InstanceData));
			instance_batch.needs_update = false;
		}

		instance_batch.instance->mesh->material->shader_effect->pipeline->bind();
		instance_batch.instance->mesh->vertex_array->bind();
		instance_batch.instance_buffer->bind(1);
		instance_batch.instance->mesh->material->bind();
		
		if (instance_batch.instance->mesh->vertex_array->hasIndexBuffer())
		{
			vkCmdDrawIndexedIndirect(Graphics::active.command_buffer, *indexed_indirect_buffer, indexed_draw_index * sizeof(VkDrawIndexedIndirectCommand), 1, sizeof(VkDrawIndexedIndirectCommand));
			++indexed_draw_index;
		}
		else
		{
			vkCmdDrawIndirect(Graphics::active.command_buffer, *indirect_buffer, draw_index * sizeof(VkDrawIndirectCommand), 1, sizeof(VkDrawIndirectCommand));
			++draw_index;
		}
	}
	Graphics::stats.instances = instance_batches.size();
	
	//End draw
	Graphics::swap_chain->endRenderPass();
}

void Scene::onStop()
{
	Dispatcher::unsubscribe(this, &Scene::onWindowResize);
	registry.on_construct<MeshComponent>().disconnect<&Scene::onMeshComponentCreate>(this);
	registry.on_construct<ModelComponent>().disconnect<&Scene::onModelComponentCreate>(this);
	registry.on_construct<LightComponent>().disconnect<&Scene::onLightComponentCreate>(this);

	registry.on_destroy<MeshComponent>().disconnect<&Scene::onMeshComponentDestroy>(this);
	registry.on_destroy<ModelComponent>().disconnect<&Scene::onModelComponentDestroy>(this);
	registry.on_destroy<LightComponent>().disconnect<&Scene::onLightComponentDestroy>(this);
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
		[](auto entity, auto& camera_component)
		{
			camera_component.camera.onViewportResize();
		});
}

void Scene::onMeshComponentCreate(entt::registry& registry, entt::entity entity)
{
	MeshComponent& mesh_component = registry.get<MeshComponent>(entity);
	
	InstanceData instance_data{};

	if (auto transform = registry.try_get<TransformComponent>(entity))
		instance_data.transform = *transform;

	if (auto sprite = registry.try_get<SpriteComponent>(entity))
		instance_data.texture_index = *sprite;

	if (auto color = registry.try_get<ColorComponent>(entity))
		instance_data.color = *color;

	if (!mesh_component.mesh->material.get())
		if (auto material = registry.try_get<MaterialComponent>(entity))
			mesh_component.mesh->material = material->material;

	mesh_component.instance = makeShared<RenderedInstance>(mesh_component.mesh);
	createMeshInstance(mesh_component.instance, instance_data);
}

void Scene::onMeshComponentDestroy(entt::registry& registry, entt::entity entity)
{
	destroyMeshInstance(registry.get<MeshComponent>(entity).instance);
}

void Scene::onModelComponentCreate(entt::registry& registry, entt::entity entity)
{
	//TODO: Fix material hardcode IMPORTANT
	ModelComponent& model_component = registry.get<ModelComponent>(entity);

	InstanceData instance_data{};

	if (auto transform = registry.try_get<TransformComponent>(entity))
		instance_data.transform = *transform;

	if (auto sprite = registry.try_get<SpriteComponent>(entity)) //TODO:
		instance_data.texture_index = *sprite;

	if (auto color = registry.try_get<ColorComponent>(entity))
		instance_data.color = *color;
	
	model_component.instances.resize(model_component.model->getMeshes().size());
	for (size_t i = 0; i < model_component.model->getMeshes().size(); ++i)
	{
		model_component.instances[i] = makeShared<RenderedInstance>(model_component.model->getMeshes()[i]);
		createMeshInstance(model_component.instances[i], instance_data);
	}
}

void Scene::onModelComponentDestroy(entt::registry& registry, entt::entity entity)
{
	ModelComponent& model_component = registry.get<ModelComponent>(entity);

	for (size_t i = 0; i < model_component.model->getMeshes().size(); ++i)
	{
		destroyMeshInstance(model_component.instances[i]);
	}
}

void Scene::onLightComponentCreate(entt::registry& registry, entt::entity entity)
{
	if (light_index >= lights.size())
		return;

	LightComponent& light_component = registry.get<LightComponent>(entity);
	light_component.light_ptr = &lights[light_index];
	lights[light_index] = light_component;
	if(auto transform = registry.try_get<TransformComponent>(entity))
		lights[light_index].position = transform->transform * glm::vec4(light_component.light.position, 1);
	++light_index;
}

void Scene::onLightComponentDestroy(entt::registry& registry, entt::entity entity)
{
	LightComponent& light_component = registry.get<LightComponent>(entity);
	
	for (size_t i = 0; i < light_index; ++i)
	{
		if (light_component == lights[i])
		{
			lights[i] = Light{}; //if color is black, light is inactive and shader skips it
			std::swap(lights[i], lights[light_index - 1]);
			--light_index;
			break;
		}
	}
}

void Scene::createMeshInstance(shared<RenderedInstance> instance, const InstanceData& instance_data)
{
	if (!instance->mesh->vertex_array.get()) instance->mesh->createVertexArray();
	if (!instance->mesh->material.get()) instance->mesh->material = material_data_3D;

	for (size_t i = 0; i < instance_batches.size(); ++i)
	{
		auto& instance_batch = instance_batches[i];
		if (instance_batch == *instance)
		{
			if (instance_batch.instance_data.size() < Graphics::MAX_INSTANCES)
			{
				instance_batch.needs_update = true;
				instance_batch.instance_data.emplace_back(std::move(instance_data));
				instance_batch.instances.emplace_back(instance);

				instance->instance_batch_index = i;
				instance->instance_data_index = instance_batch.instance_data.size() - 1;

				return;
			}
		}
	}

	instance_batches.emplace_back(instance);
	auto& new_batch = instance_batches.back();
	new_batch.instance_data.reserve(Graphics::MAX_INSTANCES);
	new_batch.instances.reserve(Graphics::MAX_INSTANCES);
	
	new_batch.instance_data.emplace_back(std::move(instance_data));
	new_batch.instances.emplace_back(instance);

	new_batch.instance_buffer = makeShared<VertexBuffer>(new_batch.instance_data.data(), Graphics::MAX_INSTANCES * sizeof(InstanceData), VMA_MEMORY_USAGE_CPU_TO_GPU);

	instance->instance_batch_index = instance_batches.size() - 1;
	instance->instance_data_index = instance_batches.back().instance_data.size() - 1;
}

void Scene::destroyMeshInstance(shared<RenderedInstance> instance)
{
	auto& instance_batch = instance_batches[instance->instance_batch_index];
	instance_batch.needs_update = true;
	
	std::swap(instance_batch.instance_data[instance->instance_data_index], instance_batch.instance_data.back());
	instance_batch.instance_data.pop_back();
	
	std::swap(instance_batch.instances[instance->instance_data_index], instance_batch.instances.back());
	instance_batch.instances[instance->instance_data_index]->instance_data_index = instance->instance_data_index;
	instance_batch.instances.pop_back();
	
	if (instance_batch.instance_data.empty())
	{
		if (instance->instance_batch_index != instance_batches.size() - 1)
		{
			InstanceBatch* last_batch = &instance_batch;
			std::swap(instance_batch, instance_batches.back());
			for (size_t j = 0; j < last_batch->instances.size(); ++j)
			{
				last_batch->instances[j]->instance_batch_index = instance_batches.back().instance->instance_batch_index; //TODO: This might be incorrect??
			}
		}
		instance_batches.pop_back();
	}
}

template<>
void Scene::updateComponent<TransformComponent>(entt::entity entity)
{
	TransformComponent& transform = registry.get<TransformComponent>(entity);

	if (registry.any_of<MeshComponent>(entity))
	{
		MeshComponent& mesh_component = registry.get<MeshComponent>(entity);
		
		instance_batches[mesh_component.instance->instance_batch_index].instance_data[mesh_component.instance->instance_data_index].transform = transform;
		instance_batches[mesh_component.instance->instance_batch_index].needs_update = true;
	}
	else if (registry.any_of<ModelComponent>(entity))
	{
		ModelComponent& model_component = registry.get<ModelComponent>(entity);

		for (size_t i = 0; i < model_component.instances.size(); ++i)
		{
			instance_batches[model_component.instances[i]->instance_batch_index].instance_data[model_component.instances[i]->instance_data_index].transform = transform;
			instance_batches[model_component.instances[i]->instance_batch_index].needs_update = true;
		}
	}
	if (registry.any_of<LightComponent>(entity))
	{
		LightComponent& light_component = registry.get<LightComponent>(entity);
		light_component.light_ptr->position = transform.transform * glm::vec4(light_component.light.position, 1);
	}
}