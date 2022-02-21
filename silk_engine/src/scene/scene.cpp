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

Scene::Scene()
{
	Dispatcher::subscribe(this, &Scene::onWindowResize);

	registry.on_construct<MeshComponent>().connect<&Scene::onMeshComponentCreate>(this);
	registry.on_construct<ModelComponent>().connect<&Scene::onModelComponentCreate>(this);
	registry.on_construct<LightComponent>().connect<&Scene::onLightComponentCreate>(this);
	
	registry.on_update<TransformComponent>().connect<&Scene::onTransformComponentUpdate>(this);
	
	registry.on_destroy<MeshComponent>().connect<&Scene::onMeshComponentDestroy>(this);
	registry.on_destroy<ModelComponent>().connect<&Scene::onModelComponentDestroy>(this);
	registry.on_destroy<LightComponent>().connect<&Scene::onLightComponentDestroy>(this);
	
	instance_batches.reserve(Graphics::MAX_INSTANCE_BATCHES); //TODO: remove this limitation some time

	indirect_buffer = makeShared<IndirectBuffer>(Graphics::MAX_INSTANCE_BATCHES * sizeof(VkDrawIndexedIndirectCommand));
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
	bool any_needs_update = false;
	static std::vector<VkDrawIndexedIndirectCommand> draw_commands;
	draw_commands.resize(instance_batches.size());
	for (size_t i = 0; i < instance_batches.size(); ++i)
	{
		if (instance_batches[i].needs_update)
		{
			instance_batches[i].instance_buffer->setData(instance_batches[i].instance_data.data(), instance_batches[i].instance_data.size() * sizeof(InstanceData));
			instance_batches[i].needs_update = false;
			VkDrawIndexedIndirectCommand draw_command{};
			draw_command.instanceCount = instance_batches[i].instance_data.size();
			draw_command.indexCount = instance_batches[i].instance->mesh->indices.size();
			draw_commands[i] = std::move(draw_command);
			any_needs_update = true;
		}
	}
	if (any_needs_update)
		indirect_buffer->setDataChecked(draw_commands.data(), draw_commands.size() * sizeof(VkDrawIndexedIndirectCommand));

	

	vkDeviceWaitIdle(*Graphics::logical_device);
	for (auto& instance_batch : instance_batches)
	{
		std::vector<VkDescriptorImageInfo> descriptor_images(Graphics::MAX_IMAGE_SLOTS);
		for (size_t i = 0; i < instance_batch.images.size(); ++i)
			descriptor_images[i] = *instance_batch.images[i];
		for (size_t i = instance_batch.images.size(); i < descriptor_images.size(); ++i)
			descriptor_images[i] = *Resources::getImage("White");

		auto& descriptor_set = instance_batch.descriptor_sets[0];

		descriptor_set.setBufferInfo(0, { { *Graphics::global_uniform, 0, VK_WHOLE_SIZE } });
		descriptor_set.setImageInfo(1, descriptor_images);
		descriptor_set.update();
	}

	//Draw instances
	Graphics::swap_chain->acquireNextImage();
	Graphics::swap_chain->beginFrame(Graphics::swap_chain->getImageIndex());
	Graphics::swap_chain->beginRenderPass(Graphics::swap_chain->getImageIndex());

	size_t draw_index = 0;
	for (auto& instance_batch : instance_batches)
	{
		instance_batch.bind();
		vkCmdDrawIndexedIndirect(Graphics::active.command_buffer, *indirect_buffer, draw_index * sizeof(VkDrawIndexedIndirectCommand), 1, sizeof(VkDrawIndexedIndirectCommand));
		++draw_index;
	}

	//End draw
	Graphics::swap_chain->endRenderPass();
	Graphics::swap_chain->endFrame(Graphics::swap_chain->getImageIndex());
	Graphics::swap_chain->present();

	Graphics::stats.instance_batches = instance_batches.size();
}

void Scene::onStop()
{
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
	//TODO: Figure out how to get rid of all the stupid transform component child checks (IDEA: have ParentComponent which takes 2 template arguments parent and child, whenever parent component gets updated it calls onTransformUpdate of child component, it's messy but better performance)
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
	{
		light_component->light_ptr->position = transform.transform * glm::vec4(light_component->light.position, 1);
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

	if (!mesh_component.mesh->material.get())
		if (auto material = registry.try_get<MaterialComponent>(entity)) //TODO: Material component is currently same as shader effect component, might have to remove it
			mesh_component.mesh->material = material->material;

	mesh_component.instance = makeShared<RenderedInstance>(mesh_component.mesh);
	createMeshInstance(mesh_component.instance, instance_data);
	
	if (auto image = registry.try_get<ImageComponent>(entity))
	{
		auto& mesh_instance_batch = instance_batches[mesh_component.instance->instance_batch_index];
		auto& mesh_instance_data = mesh_instance_batch.instance_data[mesh_component.instance->instance_data_index];
		uint32_t image_index = mesh_instance_batch.addImages(image->images);
		if (image_index == UINT32_MAX) //UINT32_MAX here means error if you look in addImages()
		{
			//TODO: This is either incorrect or bit inefficient
			destroyMeshInstance(mesh_component.instance);
			addInstanceBatch(mesh_component.instance, mesh_instance_data);
			image_index = instance_batches.back().addImages(image->images);
			SK_ASSERT(image_index != UINT32_MAX, "Entity might have too much images");
		}
		mesh_instance_data.image_index = image_index; //TODO: Implement image/buffer bindings for compute shader effects in onComputeShaderEffectComponentCreate();
	}
}

void Scene::onMeshComponentDestroy(entt::registry& registry, entt::entity entity)
{
	destroyMeshInstance(registry.get<MeshComponent>(entity).instance);
}

void Scene::onModelComponentCreate(entt::registry& registry, entt::entity entity)
{
	ModelComponent& model_component = registry.get<ModelComponent>(entity);

	InstanceData instance_data{};

	if (auto transform = registry.try_get<TransformComponent>(entity))
		instance_data.transform = *transform;

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
	if (!instance->mesh->hasAABB()) instance->mesh->calculateAABB(); //TEMP for now
	if (!instance->mesh->material.get()) instance->mesh->material = Resources::getMaterial("Textured Lit 3D");

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

	addInstanceBatch(instance, instance_data);
}

void Scene::addInstanceBatch(shared<RenderedInstance> instance, const InstanceData& instance_data)
{
	instance_batches.emplace_back(instance);
	auto& new_batch = instance_batches.back();
	new_batch.instance_data.reserve(Graphics::MAX_INSTANCES);
	new_batch.instances.reserve(Graphics::MAX_INSTANCES);

	new_batch.images.reserve(Graphics::MAX_IMAGE_SLOTS);
	new_batch.images.emplace_back(Resources::getImage("White"));

	new_batch.instance_data.emplace_back(std::move(instance_data));
	new_batch.instances.emplace_back(instance);

	new_batch.instance_buffer = makeShared<VertexBuffer>(new_batch.instance_data.data(), Graphics::MAX_INSTANCES * sizeof(InstanceData), VMA_MEMORY_USAGE_CPU_TO_GPU);

	new_batch.descriptor_sets.resize(1);
	new_batch.descriptor_sets[0] = *new_batch.instance->mesh->material->shader_effect->pipeline->getShader()->getDescriptorSet();

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

	//TODO: Update instance_batch images
	//Have another vector containing image owner instances for each image in images if (image_owners[image].size() - 1) == 0 swap that image with latest image in images and pop back, but also update image_index for all the owner instances of images.back() to index where it got swapped to
	
	if (instance_batch.instance_data.empty())
	{
		if (instance->instance_batch_index != instance_batches.size() - 1)
		{
			InstanceBatch* last_batch = &instance_batch;
			std::swap(instance_batch, instance_batches.back());
			for (size_t j = 0; j < last_batch->instances.size(); ++j)
			{
				last_batch->instances[j]->instance_batch_index = instance_batches.back().instance->instance_batch_index;
			}
		}
		instance_batches.pop_back();
	}
}