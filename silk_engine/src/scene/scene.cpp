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
#include "gfx/renderer.h"

Scene::Scene()
{
	Dispatcher::subscribe(this, &Scene::onWindowResize);
	
	registry.on_construct<MeshComponent>().connect<&Scene::onMeshComponentCreate>(this);
	registry.on_construct<ModelComponent>().connect<&Scene::onModelComponentCreate>(this);
	registry.on_construct<LightComponent>().connect<&Scene::onLightComponentCreate>(this);
	
	registry.on_destroy<MeshComponent>().connect<&Scene::onMeshComponentDestroy>(this);
	registry.on_destroy<ModelComponent>().connect<&Scene::onModelComponentDestroy>(this);
	registry.on_destroy<LightComponent>().connect<&Scene::onLightComponentDestroy>(this);

	shared<DescriptorSet> global_descriptor_set = makeShared<DescriptorSet>();
	global_descriptor_set->addBuffer(0, { *Graphics::global_uniform, 0, VK_WHOLE_SIZE }, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.build();

	auto white_image = Resources::getImage("backpack/diffuse.jpg");
	shared<DescriptorSet> descriptor_set = makeShared<DescriptorSet>();
	descriptor_set->addImages(0, {
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image
		}, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build();

	material_data_3D = makeShared<MaterialData>(Resources::getMaterial("3D"), std::vector<shared<DescriptorSet>>{ global_descriptor_set, descriptor_set });
	material_data_batch3D = makeShared<MaterialData>(Resources::getMaterial("Batch3D"), std::vector<shared<DescriptorSet>>{ global_descriptor_set, descriptor_set });
}

Scene::~Scene()
{
	Dispatcher::unsubscribe(this, &Scene::onWindowResize);
	registry.on_construct<MeshComponent>().disconnect<&Scene::onMeshComponentCreate>(this);
	registry.on_construct<ModelComponent>().disconnect<&Scene::onModelComponentCreate>(this);
	registry.on_construct<LightComponent>().disconnect<&Scene::onLightComponentCreate>(this);

	registry.on_destroy<MeshComponent>().disconnect<&Scene::onMeshComponentDestroy>(this);
	registry.on_destroy<ModelComponent>().disconnect<&Scene::onModelComponentDestroy>(this);
	registry.on_destroy<LightComponent>().disconnect<&Scene::onLightComponentDestroy>(this);
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
	Graphics::swap_chain->beginRenderPass();
	
	//Draw instances
	for (auto& instance_batch : instance_batches)
	{
		if (instance_batch.needs_update && instance_batch.instance_data.size())
		{
			instance_batch.instance->mesh->vertex_array->getVertexBuffer(1)->setData(instance_batch.instance_data.data(), instance_batch.instance_data.size() * sizeof(InstanceData));
			instance_batch.needs_update = false;
		}

		instance_batch.instance->material_data->material->pipeline->bind();
		instance_batch.instance->mesh->vertex_array->bind();
		for (size_t j = 0; j < instance_batch.instance->material_data->descriptor_sets.size(); ++j)
			instance_batch.instance->material_data->descriptor_sets[j]->bind(j);
		
		//NOTE: vkCmdDrawIndexedIndirect is bit faster but it limits us by having a fixed instanced batches size and adds clutter
		if(instance_batch.instance->mesh->vertex_array->hasIndexBuffer())
			vkCmdDrawIndexed(Graphics::active.command_buffer, instance_batch.instance->mesh->indices.size(), instance_batch.instance_data.size(), 0, 0, 0);
		else
			vkCmdDraw(Graphics::active.command_buffer, instance_batch.instance->mesh->vertices.size(), instance_batch.instance_data.size(), 0, 0);
	}
	Graphics::stats.instances = instance_batches.size();
	Graphics::stats.batches = Renderer::batcher.batches.size();
	SK_INFO(Graphics::stats.instances);

	Renderer::updateBatch();
	Renderer::drawLastBatch();
	
	//End draw
	Graphics::swap_chain->endRenderPass();
}

void Scene::onStop()
{
	stopped = true;
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

	mesh_component.instance->instance_data = &instance_data; //NOTE: This becomes unusable when scope ends

	createMeshInstance(mesh_component.instance);
}

void Scene::onMeshComponentDestroy(entt::registry& registry, entt::entity entity)
{
	if (stopped)
		return;

	MeshComponent& mesh_component = registry.get<MeshComponent>(entity);
	
	destroyMeshInstance(mesh_component.instance);
}

void Scene::onModelComponentCreate(entt::registry& registry, entt::entity entity)
{
	//TODO: Fix material hardcode IMPORTANT
	ModelComponent& model_component = registry.get<ModelComponent>(entity);

	InstanceData instance_data{};

	if (auto transform = registry.try_get<TransformComponent>(entity))
		instance_data.transform = *transform;

	if (auto sprite = registry.try_get<SpriteComponent>(entity))
		instance_data.texture_index = *sprite;

	if (auto color = registry.try_get<ColorComponent>(entity))
		instance_data.color = *color;
	
	for (auto& mesh : model_component.model->meshes)
	{
		mesh->instance_data = &instance_data;
		createMeshInstance(mesh);
	}
}

void Scene::onModelComponentDestroy(entt::registry& registry, entt::entity entity)
{
	ModelComponent& model_component = registry.get<ModelComponent>(entity);
	for (auto& mesh : model_component.model->meshes)
	{
		destroyMeshInstance(mesh);
	}
}

void Scene::onLightComponentCreate(entt::registry& registry, entt::entity entity)
{
	LightComponent& light_component = registry.get<LightComponent>(entity);
	lights[light_index] = light_component;
	if(auto transform = registry.try_get<TransformComponent>(entity))
		lights[light_index].position = transform->transform * glm::vec4(light_component.light.position, 1);
	light_index = (light_index + 1) % Graphics::MAX_LIGHTS;
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

void Scene::createMeshInstance(shared<RenderedInstance> instance)
{
	if (Renderer::batcher.active)
	{
		instance->material_data = material_data_batch3D;
		instance->batched = true;

		addBatchedInstance(*instance);
	}
	else
	{
		instance->material_data = material_data_3D;
		instance->batched = false;

		for (auto& instance_batch : instance_batches)
		{
			if (instance_batch == *instance)
			{
				instance_batch.needs_update = true;
				instance_batch.instance_data.emplace_back(*instance->instance_data);			
				return;
			}
		}

		std::vector<InstanceData> new_instance_data;
		new_instance_data.reserve(Graphics::MAX_INSTANCES);
		new_instance_data.emplace_back(*instance->instance_data);
		instance_batches.emplace_back(instance, std::move(new_instance_data));
		instance->instance_data = &instance_batches.back().instance_data.back();
	}
}

void Scene::destroyMeshInstance(shared<RenderedInstance> instance)
{
	for (auto& instance_batch : instance_batches)
	{
		if (*instance_batch.instance == *instance)
		{
			instance_batch.needs_update = true;

			registry.view<MeshComponent>().each(
				[&](entt::entity entity, MeshComponent& rc)
				{
					if (rc.instance->instance_data == &instance_batch.instance_data.back())
						rc.instance->instance_data = instance->instance_data;
				});
			std::swap(*instance->instance_data, instance_batch.instance_data.back());
			instance_batch.instance_data.pop_back();

			return;
		}
	}
}

template<>
void Scene::updateComponent<MeshComponent>(entt::entity entity)
{
	MeshComponent& mesh_component = registry.get<MeshComponent>(entity);
	
	InstanceData instance_data{};
	
	if (auto transform = registry.try_get<TransformComponent>(entity))
		instance_data.transform = *transform;
	
	if (auto sprite = registry.try_get<SpriteComponent>(entity))
		instance_data.texture_index = *sprite;
	
	if (auto color = registry.try_get<ColorComponent>(entity))
		instance_data.color = *color;
	
	*mesh_component.instance->instance_data = std::move(instance_data);

	if (mesh_component.instance->batched)
	{
		for (auto& batch : Renderer::batcher.batches)
		{
			if (batch == *mesh_component.instance)
			{
				batch.needs_update = true;
				break;
			}
		}
	}
	else
	{
		for (auto& instance_batch : instance_batches)
		{
			if (*instance_batch.instance == *mesh_component.instance)
			{
				instance_batch.needs_update = true;
				break;
			}
		}
	}
}

void Scene::addBatchedInstance(const RenderedInstance& instance)
{
	const Mesh& mesh = *instance.mesh;

	SK_ASSERT(mesh.indices.size() < Graphics::MAX_BATCH_INDICES, 
		"Batched instance has too much indices. max indices allowed is: {0}, but mesh has {1}", 
		Graphics::MAX_BATCH_INDICES, mesh.indices.size());

	SK_ASSERT(mesh.vertices.size() < Graphics::MAX_BATCH_VERTICES, 
		"Batched instance has too much vertices. max verticees allowed is: {0}, but mesh has {1}", 
		Graphics::MAX_BATCH_VERTICES, mesh.vertices.size());

	bool should_add_batch = false;

	if (Renderer::batcher.batches.empty())
	{
		should_add_batch = true;
	}
	else if (instance.material_data != Renderer::batcher.batches.back().material_data)
	{
		for (size_t i = 0; i < Renderer::batcher.batches.size(); ++i)
		{
			if (Renderer::batcher.batches[i].material_data == instance.material_data)
			{
				std::swap(Renderer::batcher.batches[i], Renderer::batcher.batches.back());
				break;
			}
		}
	}

	//If batch had not enough vertices/indices to store the mesh create a new batch
	if (!Renderer::batcher.batches.empty() && 
		(Renderer::batcher.batches.back().vertices_index + mesh.vertices.size() >= Renderer::batcher.batches.back().vertices.size() ||
		Renderer::batcher.batches.back().indices_index + mesh.indices.size() >= Renderer::batcher.batches.back().indices.size()))
	{
		should_add_batch = true;
	}

	if (should_add_batch)
	{
		Renderer::addBatch();
		Renderer::batcher.batches.back().material_data = instance.material_data;
	}

	Renderer::batcher.batches.back().addInstance(instance);
}