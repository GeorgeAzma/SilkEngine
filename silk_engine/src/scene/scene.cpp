#include "scene.h"
#include "entity.h"
#include "components.h"
#include "scene/resources.h"
#include "gfx/devices/logical_device.h"
#include "gfx/devices/physical_device.h"
#include "gfx/buffers/uniform_buffer.h"
#include "utils/general_utils.h"
#include "utils/time.h"
#include "gfx/window/swap_chain.h"
#include "utils/timers.h"

Scene::Scene()
{
	Dispatcher::subscribe(this, &Scene::onWindowResize);

	registry.on_construct<MeshComponent>().connect<&Scene::onMeshComponentCreate>(this);
	registry.on_construct<ModelComponent>().connect<&Scene::onModelComponentCreate>(this);
	registry.on_construct<LightComponent>().connect<&Scene::onLightComponentCreate>(this);
	registry.on_update<TransformComponent>().connect<&Scene::onTransformComponentUpdate>(this);
	registry.on_update<ColorComponent>().connect<&Scene::onColorComponentUpdate>(this);
	registry.on_destroy<MeshComponent>().connect<&Scene::onMeshComponentDestroy>(this);
	registry.on_destroy<ModelComponent>().connect<&Scene::onModelComponentDestroy>(this);
	registry.on_destroy<LightComponent>().connect<&Scene::onLightComponentDestroy>(this);

	instance_batches.reserve(Graphics::MAX_INSTANCE_BATCHES); //TODO: remove this limitation some time

	indirect_buffer = makeUnique<IndirectBuffer>(Graphics::MAX_INSTANCE_BATCHES * sizeof(vk::DrawIndexedIndirectCommand));

	global_uniform_buffer = makeUnique<UniformBuffer>(sizeof(GlobalUniformData));

	Timers::every(200ms, [] {
		SK_INFO("Render stats: ");
		SK_INFO("Instance batches: {0}", Graphics::stats.instance_batches);
		SK_INFO("Instances: {0}", Graphics::stats.instances);
		SK_INFO("Vertices: {0}", Graphics::stats.vertices);
		SK_INFO("Indices: {0}", Graphics::stats.indices);
		});
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

	GlobalUniformData global_uniform_data{};
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

	global_uniform_buffer->setData(&global_uniform_data, sizeof(GlobalUniformData));

	//Drawing	
	bool any_needs_update = false;
	static std::vector<vk::DrawIndexedIndirectCommand> draw_commands;
	draw_commands.resize(instance_batches.size());
	for (size_t i = 0; i < instance_batches.size(); ++i)
	{
		if (instance_batches[i].needs_update)
		{
			instance_batches[i].instance_buffer->setData(instance_batches[i].instance_data.data(), instance_batches[i].instance_data.size() * sizeof(InstanceData));
			instance_batches[i].needs_update = false;
			vk::DrawIndexedIndirectCommand draw_command{};
			draw_command.instanceCount = instance_batches[i].instance_data.size();
			draw_command.indexCount = instance_batches[i].instance->mesh->indices.size();
			draw_commands[i] = std::move(draw_command);
			any_needs_update = true;
		}
	}
	if (any_needs_update)
		indirect_buffer->setData(draw_commands.data(), draw_commands.size() * sizeof(vk::DrawIndexedIndirectCommand));

	//Draw instances
	const auto& white = Resources::getImage("White")->getDescriptorInfo();
	for (auto& instance_batch : instance_batches)
	{
		instance_batch.descriptor_sets[0].setBufferInfo(0, { *global_uniform_buffer	}); //TODO: Global data doesn't have to update for each batch

		if (instance_batch.images_need_update)
		{
			std::vector<vk::DescriptorImageInfo> descriptor_images(Graphics::MAX_IMAGE_SLOTS, white);
			for (size_t i = 0; i < instance_batch.images.size(); ++i)
				descriptor_images[i] = *instance_batch.images[i];

			size_t index = 0;
			instance_batch.descriptor_sets[1].setImageInfo(0, descriptor_images);
			instance_batch.images_need_update = false;
		}
	}

	Graphics::beginFrame();
	Graphics::swap_chain->beginRenderPass();
	size_t draw_index = 0;
	for (auto& instance_batch : instance_batches)
	{
		instance_batch.bind();
		Graphics::active.command_buffer.drawIndexedIndirect(*indirect_buffer, draw_index * sizeof(vk::DrawIndexedIndirectCommand), 1, sizeof(vk::DrawIndexedIndirectCommand));
		++draw_index;
	}

	//End draw
	Graphics::swap_chain->endRenderPass();
	Graphics::endFrame();

	Graphics::stats.instance_batches += instance_batches.size();
	for (const auto& instance_batch : instance_batches)
	{
		Graphics::stats.instances += instance_batch.instances.size();
		Graphics::stats.vertices += instance_batch.instance->mesh->vertices.size() * instance_batch.instances.size();
		Graphics::stats.indices += instance_batch.instance->mesh->indices.size() * instance_batch.instances.size();
	}
}

void Scene::onStop()
{
	registry.on_construct<MeshComponent>().disconnect<&Scene::onMeshComponentCreate>(this);
	registry.on_construct<ModelComponent>().disconnect<&Scene::onModelComponentCreate>(this);
	registry.on_construct<LightComponent>().disconnect<&Scene::onLightComponentCreate>(this);
	registry.on_update<TransformComponent>().disconnect<&Scene::onTransformComponentUpdate>(this);
	registry.on_update<ColorComponent>().disconnect<&Scene::onColorComponentUpdate>(this);
	registry.on_destroy<MeshComponent>().disconnect<&Scene::onMeshComponentDestroy>(this);
	registry.on_destroy<ModelComponent>().disconnect<&Scene::onModelComponentDestroy>(this);
	registry.on_destroy<LightComponent>().disconnect<&Scene::onLightComponentDestroy>(this);
	Dispatcher::unsubscribe(this, &Scene::onWindowResize);
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

void Scene::onTransformComponentUpdate(entt::registry& registry, entt::entity entity)
{
	//TODO: Figure out how to get rid of all the stupid transform component child checks (It can be solved via Event system, but for now with this little components this method is faster)
	TransformComponent& transform = registry.get<TransformComponent>(entity);

	if (auto mesh_component = registry.try_get<MeshComponent>(entity))
	{
		instance_batches[mesh_component->instance->instance_batch_index].instance_data[mesh_component->instance->instance_data_index].transform = transform;
		instance_batches[mesh_component->instance->instance_batch_index].needs_update = true;
	}
	if (auto model_component = registry.try_get<ModelComponent>(entity))
	{
		for (size_t i = 0; i < model_component->instances.size(); ++i)
		{
			instance_batches[model_component->instances[i]->instance_batch_index].instance_data[model_component->instances[i]->instance_data_index].transform = transform;
			instance_batches[model_component->instances[i]->instance_batch_index].needs_update = true;
		}
	}
	if (auto light_component = registry.try_get<LightComponent>(entity))
		light_component->light_ptr->position = transform.transform * glm::vec4(light_component->light.position, 1);
}

void Scene::onColorComponentUpdate(entt::registry& registry, entt::entity entity)
{
	ColorComponent& color = registry.get<ColorComponent>(entity);

	if (auto mesh_component = registry.try_get<MeshComponent>(entity))
	{
		instance_batches[mesh_component->instance->instance_batch_index].instance_data[mesh_component->instance->instance_data_index].color = color;
		instance_batches[mesh_component->instance->instance_batch_index].needs_update = true;
	}
	if (auto model_component = registry.try_get<ModelComponent>(entity))
	{
		for (size_t i = 0; i < model_component->instances.size(); ++i)
		{
			instance_batches[model_component->instances[i]->instance_batch_index].instance_data[model_component->instances[i]->instance_data_index].color = color;
			instance_batches[model_component->instances[i]->instance_batch_index].needs_update = true;
		}
	}
}

void Scene::onMeshComponentCreate(entt::registry& registry, entt::entity entity)
{
	MeshComponent& mesh_component = registry.get<MeshComponent>(entity);

	InstanceData instance_data{};

	if (auto transform = registry.try_get<TransformComponent>(entity))
		instance_data.transform = *transform;

	if (auto color = registry.try_get<ColorComponent>(entity))
		instance_data.color = *color;

	mesh_component.instance = makeShared<RenderedInstance>(mesh_component.mesh);

	if (auto material = registry.try_get<MaterialComponent>(entity))
		mesh_component.instance->material = material->material;

	if (auto image = registry.try_get<ImageComponent>(entity))
		mesh_component.instance->images = image->images;

	createMeshInstance(mesh_component.instance, instance_data);
}

void Scene::onMeshComponentDestroy(entt::registry& registry, entt::entity entity)
{
	destroyMeshInstance(*registry.get<MeshComponent>(entity).instance);
}

void Scene::onModelComponentCreate(entt::registry& registry, entt::entity entity)
{
	ModelComponent& model_component = registry.get<ModelComponent>(entity);

	InstanceData instance_data{};

	if (auto transform = registry.try_get<TransformComponent>(entity))
		instance_data.transform = *transform;

	if (auto color = registry.try_get<ColorComponent>(entity))
		instance_data.color = *color;

	shared<ShaderEffect> material = nullptr;
	if (auto mat = registry.try_get<MaterialComponent>(entity))
		material = mat->material;

	model_component.instances.resize(model_component.model->getMeshes().size());
	for (size_t i = 0; i < model_component.model->getMeshes().size(); ++i)
	{
		model_component.instances[i] = makeShared<RenderedInstance>(model_component.model->getMeshes()[i], material);
		model_component.instances[i]->images = model_component.model->getImages()[i];
		createMeshInstance(model_component.instances[i], instance_data);
	}
}

void Scene::onModelComponentDestroy(entt::registry& registry, entt::entity entity)
{
	ModelComponent& model_component = registry.get<ModelComponent>(entity);

	for (size_t i = 0; i < model_component.model->getMeshes().size(); ++i)
	{
		destroyMeshInstance(*model_component.instances[i]);
	}
}

void Scene::onLightComponentCreate(entt::registry& registry, entt::entity entity)
{
	if (light_index >= lights.size())
		return;

	LightComponent& light_component = registry.get<LightComponent>(entity);
	light_component.light_ptr = &lights[light_index];
	lights[light_index] = light_component;
	if (auto transform = registry.try_get<TransformComponent>(entity))
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
	if (!instance->mesh->hasAABB()) instance->mesh->calculateAABB(); //TEMP for now
	if (!instance->material.get()) instance->material = Resources::getShaderEffect("Lit 3D");

	bool need_new_instance_batch = true;
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

				need_new_instance_batch = false;
				break;
			}
		}
	}

	if (need_new_instance_batch)
		addInstanceBatch(instance, instance_data);

	if (instance->images.size())
	{
		uint32_t image_index = instance_batches[instance->instance_batch_index].addImages(instance->images);
		if (image_index == UINT32_MAX) //UINT32_MAX means out of batch images, so here we are creating new batch
		{
			destroyMeshInstance(*instance);
			addInstanceBatch(instance, instance_batches[instance->instance_batch_index].instance_data[instance->instance_data_index]);
			image_index = instance_batches.back().addImages(instance->images);
			SK_ASSERT(image_index != UINT32_MAX, "Entity has {0} images, when max image slot count is {1}.", instance->images.size(), Graphics::MAX_IMAGE_SLOTS);
		}
		instance_batches[instance->instance_batch_index].instance_data[instance->instance_data_index].image_index = image_index;
		size_t last_size = instance->images.size();
		instance->images.clear();
		instance->images.resize(last_size);
	}
}

void Scene::addInstanceBatch(shared<RenderedInstance> instance, const InstanceData& instance_data)
{
	instance_batches.emplace_back(instance);
	auto& new_batch = instance_batches.back();
	new_batch.instance_data.reserve(Graphics::MAX_INSTANCES);
	new_batch.instances.reserve(Graphics::MAX_INSTANCES);

	new_batch.images.reserve(Graphics::MAX_IMAGE_SLOTS);
	new_batch.images.emplace_back(Resources::getImage("White"));
	new_batch.image_owners.resize(Graphics::MAX_IMAGE_SLOTS);

	new_batch.instance_data.emplace_back(std::move(instance_data));
	new_batch.instances.emplace_back(instance);

	new_batch.instance_buffer = makeShared<VertexBuffer>(new_batch.instance_data.data(), Graphics::MAX_INSTANCES * sizeof(InstanceData), VMA_MEMORY_USAGE_CPU_TO_GPU);

	for (auto&& [set, descriptor_set] : new_batch.instance->material->pipeline->getShader()->getDescriptorSets())
		new_batch.descriptor_sets[set] = *descriptor_set;


	instance->instance_batch_index = instance_batches.size() - 1;
	instance->instance_data_index = instance_batches.back().instance_data.size() - 1;
}

void Scene::destroyMeshInstance(const RenderedInstance& instance)
{
	auto& instance_batch = instance_batches[instance.instance_batch_index];

	instance_batch.instances.back()->instance_data_index = instance.instance_data_index;

	if (instance_batch.instance_data.size() - 1 == 0)
	{
		if (instance.instance_batch_index != instance_batches.size() - 1)
		{
			InstanceBatch* last_batch = &instance_batch;
			instance_batches.back().needs_update = true; //Instance batches data got swapped around so this forces to also update instance_batch draw commands buffer
			std::swap(instance_batch, instance_batches.back());
			for (size_t j = 0; j < last_batch->instances.size(); ++j)
				last_batch->instances[j]->instance_batch_index = instance_batches.back().instance->instance_batch_index;
		}
		instance_batches.pop_back();
		return;
	}

	instance_batch.removeImages(instance_batch.instance_data[instance.instance_data_index].image_index, instance.images.size());

	instance_batch.needs_update = true;

	std::swap(instance_batch.instance_data[instance.instance_data_index], instance_batch.instance_data.back());
	instance_batch.instance_data.pop_back();

	std::swap(instance_batch.instances[instance.instance_data_index], instance_batch.instances.back());
	instance_batch.instances.pop_back();
}