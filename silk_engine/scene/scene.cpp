#include "scene.h"
#include "entity.h"
#include "components.h"
#include "scene/resources.h"
#include "gfx/devices/logical_device.h"
#include "gfx/devices/physical_device.h"
#include "utils/general_utils.h"
#include "utils/time.h"
#include "gfx/window/swap_chain.h"
#include "core/event.h"
#include "gfx/window/window.h"
#include "gfx/graphics.h"
#include "meshes/text_mesh.h"
#include "camera/camera.h"
#include "gfx/renderer.h"

Scene::Scene()
{
	Dispatcher::subscribe(this, &Scene::onWindowResize);

	registry.on_construct<MeshComponent>().connect<&Scene::onMeshComponentCreate>(this);
	registry.on_construct<ModelComponent>().connect<&Scene::onModelComponentCreate>(this);
	registry.on_construct<LightComponent>().connect<&Scene::onLightComponentCreate>(this);
	registry.on_construct<TextComponent>().connect<&Scene::onTextComponentCreate>(this);
	
	registry.on_update<TransformComponent>().connect<&Scene::onTransformComponentUpdate>(this);
	registry.on_update<ColorComponent>().connect<&Scene::onColorComponentUpdate>(this);
	registry.on_update<MaterialComponent>().connect<&Scene::onMaterialComponentUpdate>(this);
	registry.on_update<LightComponent>().connect<&Scene::onLightComponentUpdate>(this);
	registry.on_update<ImageComponent>().connect<&Scene::onImageComponentUpdate>(this);
	registry.on_update<TextComponent>().connect<&Scene::onTextComponentUpdate>(this);

	registry.on_destroy<MeshComponent>().connect<&Scene::onMeshComponentDestroy>(this);
	registry.on_destroy<ModelComponent>().connect<&Scene::onModelComponentDestroy>(this);
	registry.on_destroy<LightComponent>().connect<&Scene::onLightComponentDestroy>(this);

	SK_TRACE("Scene created");
}

Scene::~Scene()
{
	registry.on_construct<MeshComponent>().disconnect<&Scene::onMeshComponentCreate>(this);
	registry.on_construct<ModelComponent>().disconnect<&Scene::onModelComponentCreate>(this);
	registry.on_construct<LightComponent>().disconnect<&Scene::onLightComponentCreate>(this);
	registry.on_construct<TextComponent>().disconnect<&Scene::onTextComponentCreate>(this);

	registry.on_update<TransformComponent>().disconnect<&Scene::onTransformComponentUpdate>(this);
	registry.on_update<ColorComponent>().disconnect<&Scene::onColorComponentUpdate>(this);
	registry.on_update<MaterialComponent>().disconnect<&Scene::onMaterialComponentUpdate>(this);
	registry.on_update<LightComponent>().disconnect<&Scene::onLightComponentUpdate>(this);
	registry.on_update<ImageComponent>().disconnect<&Scene::onImageComponentUpdate>(this);
	registry.on_update<TextComponent>().disconnect<&Scene::onTextComponentUpdate>(this);

	registry.on_destroy<MeshComponent>().disconnect<&Scene::onMeshComponentDestroy>(this);
	registry.on_destroy<ModelComponent>().disconnect<&Scene::onModelComponentDestroy>(this);
	registry.on_destroy<LightComponent>().disconnect<&Scene::onLightComponentDestroy>(this);
	
	Dispatcher::unsubscribe(this, &Scene::onWindowResize);
}

void Scene::init()
{
	for (auto& [type_id, system] : systems)
		if (system->enabled)
			system->init();
	onStart();
}

void Scene::update()
{
	//Update systems
	for (auto& [type_id, system] : systems)
		if (system->enabled)
			system->update();

	//Update components
	registry.view<ScriptComponent>().each(
		[&](auto entity, auto& script_component)
		{
			if (!script_component.instance)
			{
				script_component.instance = script_component.instantiate_script();
				script_component.instance->entity = makeShared<Entity>(entity, this);
				script_component.instance->onCreate();
			}
			script_component.instance->onUpdate();
		});
	onUpdate();
}

void Scene::destroy()
{
	registry.view<ScriptComponent>().each([](auto entity, auto& script_component)
		{
			script_component.instance->onDestroy();
		});
	for (auto& [type_id, system] : systems)
		if (system->enabled)
			system->destroy();

	for (auto& e : entities)
		removeEntity(*e);
	entities.clear();
	onStop();
}

shared<Entity> Scene::createEntity()
{
	entities.emplace_back(makeShared<Entity>(registry.create(), this));
	return entities.back();
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

Camera* Scene::getMainCamera()
{
	if (main_camera)
		return main_camera;

	if(!registry.view<CameraComponent>().empty())
		main_camera = &registry.get<CameraComponent>(registry.view<CameraComponent>().front()).camera;

	return main_camera;
}

void Scene::onTransformComponentUpdate(entt::registry& registry, entt::entity entity)
{
	//TODO: Figure out how to get rid of all the stupid transform component child checks (It can be solved via Event system, or entity child system, but for now with this little components this method is faster)
	TransformComponent& transform = registry.get<TransformComponent>(entity);

	if (auto mesh_component = registry.try_get<MeshComponent>(entity))
	{
		Renderer::getInstanceBatch(mesh_component->instance->instance_batch_index).instance_data[mesh_component->instance->instance_data_index].transform = transform;
		Renderer::getInstanceBatch(mesh_component->instance->instance_batch_index).needs_update = true;
	}
	if (auto model_component = registry.try_get<ModelComponent>(entity))
	{
		for (size_t i = 0; i < model_component->instances.size(); ++i)
		{
			Renderer::getInstanceBatch(model_component->instances[i]->instance_batch_index).instance_data[model_component->instances[i]->instance_data_index].transform = transform;
			Renderer::getInstanceBatch(model_component->instances[i]->instance_batch_index).needs_update = true;
		}
	}
	if (auto light_component = registry.try_get<LightComponent>(entity))
		light_component->light_ptr->position = transform.transform * vec4(light_component->light.position, 1);
}

void Scene::onColorComponentUpdate(entt::registry& registry, entt::entity entity)
{
	ColorComponent& color = registry.get<ColorComponent>(entity);

	if (auto mesh_component = registry.try_get<MeshComponent>(entity))
	{
		Renderer::getInstanceBatch(mesh_component->instance->instance_batch_index).instance_data[mesh_component->instance->instance_data_index].color = color;
		Renderer::getInstanceBatch(mesh_component->instance->instance_batch_index).needs_update = true;
	}
	if (auto model_component = registry.try_get<ModelComponent>(entity))
	{
		for (size_t i = 0; i < model_component->instances.size(); ++i)
		{
			Renderer::getInstanceBatch(model_component->instances[i]->instance_batch_index).instance_data[model_component->instances[i]->instance_data_index].color = color;
			Renderer::getInstanceBatch(model_component->instances[i]->instance_batch_index).needs_update = true;
		}
	}
}

void Scene::onMaterialComponentUpdate(entt::registry& registry, entt::entity entity)
{
	MaterialComponent& material = registry.get<MaterialComponent>(entity);

	if (auto mesh_component = registry.try_get<MeshComponent>(entity))
	{
		if (mesh_component->instance->pipeline != material.material)
		{
			InstanceData instance_data = Renderer::getInstanceBatch(mesh_component->instance->instance_batch_index).instance_data[mesh_component->instance->instance_data_index];
			Renderer::destroyInstance(*mesh_component->instance);
			mesh_component->instance->pipeline = material.material;
			Renderer::createInstance(mesh_component->instance, mesh_component->mesh, std::move(instance_data));
		}
	}
	if (auto model_component = registry.try_get<ModelComponent>(entity))
	{
		if (model_component->instances.size() && model_component->instances[0]->pipeline != material.material)
		{
			for (size_t i = 0; i < model_component->instances.size(); ++i)
			{
				shared<RenderedInstance>& instance = model_component->instances[i];
				InstanceData instance_data = Renderer::getInstanceBatch(instance->instance_batch_index).instance_data[instance->instance_data_index];
				Renderer::destroyInstance(*instance);
				instance->pipeline = material.material;
				Renderer::createInstance(instance, model_component->model->getMeshes()[i], std::move(instance_data));
			}
		}
	}
}

void Scene::onLightComponentUpdate(entt::registry& registry, entt::entity entity)
{
	LightComponent& light = registry.get<LightComponent>(entity);
	(*light.light_ptr) = light.light;
}

void Scene::onImageComponentUpdate(entt::registry& registry, entt::entity entity)
{
	if (auto mesh_component = registry.try_get<MeshComponent>(entity))
	{
		ImageComponent& image = registry.get<ImageComponent>(entity); 
		SK_VERIFY(mesh_component->instance->images.size() >= image.images.size(), "Couldn't update image component because amount of images specified is more than instance's image count");
		auto& instance = mesh_component->instance;
		bool same = true;
		for (size_t i = 0; i < instance->images.size(); ++i)
		{
			if (image.images[i] != instance->images[i])
			{
				same = false;
				break;
			}
		}
		if (same)
			return;
		instance->images = image.images;
		auto& instance_batch = Renderer::getInstanceBatch(instance->instance_batch_index);
		instance_batch.instance_images.remove(instance_batch.instance_data[instance->instance_data_index].image_index, instance->images.size());
		uint32_t image_index = instance_batch.instance_images.add(instance->images);
		if (image_index == UINT32_MAX)
		{
			InstanceData instance_data = instance_batch.instance_data[instance->instance_data_index];
			Renderer::destroyInstance(*instance);
			Renderer::addInstanceBatch(instance, mesh_component->mesh, std::move(instance_data));
			image_index = Renderer::getInstanceBatch(instance->instance_batch_index).instance_images.add(instance->images);
		}
		Renderer::getInstanceBatch(instance->instance_batch_index).instance_data[instance->instance_data_index].image_index = image_index;
	}
}

void Scene::onTextComponentUpdate(entt::registry& registry, entt::entity entity)
{
	TextComponent& text = registry.get<TextComponent>(entity);
	MeshComponent& mesh = registry.get<MeshComponent>(entity);
	InstanceData instance_data = Renderer::getInstanceBatch(mesh.instance->instance_batch_index).instance_data[mesh.instance->instance_data_index];
	Renderer::destroyInstance(*mesh.instance);
	*mesh.mesh = TextMesh(text.text, text.size, text.font);
	Renderer::createInstance(mesh.instance, mesh.mesh, std::move(instance_data));
}

void Scene::onMeshComponentCreate(entt::registry& registry, entt::entity entity)
{
	MeshComponent& mesh_component = registry.get<MeshComponent>(entity);

	InstanceData instance_data{};
	if (auto transform = registry.try_get<TransformComponent>(entity))
		instance_data.transform = *transform;
	if (auto color = registry.try_get<ColorComponent>(entity))
		instance_data.color = *color;

	mesh_component.instance = makeShared<RenderedInstance>();
	if (auto material = registry.try_get<MaterialComponent>(entity))
		mesh_component.instance->pipeline = material->material;
	if (auto image = registry.try_get<ImageComponent>(entity))
		mesh_component.instance->images = image->images;

	Renderer::createInstance(mesh_component.instance, mesh_component.mesh, std::move(instance_data));
}

void Scene::onMeshComponentDestroy(entt::registry& registry, entt::entity entity)
{
	Renderer::destroyInstance(*registry.get<MeshComponent>(entity).instance);
}

void Scene::onTextComponentCreate(entt::registry& registry, entt::entity entity)
{
	TextComponent& text_component = registry.get<TextComponent>(entity);
	if (!text_component.font.get())
		text_component.font = Resources::get<Font>("Arial");
	registry.emplace<MaterialComponent>(entity, MaterialComponent{ Resources::get<GraphicsPipeline>("Font") });
	registry.emplace<ImageComponent>(entity, ImageComponent{ std::vector<shared<Image>>{ text_component.font->getAtlas() } });
	registry.emplace<MeshComponent>(entity, MeshComponent{ makeShared<Mesh>(TextMesh(text_component.text, text_component.size, text_component.font)) });
}

void Scene::onModelComponentCreate(entt::registry& registry, entt::entity entity)
{
	ModelComponent& model_component = registry.get<ModelComponent>(entity);

	InstanceData instance_data{};
	if (auto transform = registry.try_get<TransformComponent>(entity))
		instance_data.transform = *transform;
	if (auto color = registry.try_get<ColorComponent>(entity))
		instance_data.color = *color;
	shared<GraphicsPipeline> material = nullptr;
	if (auto mat = registry.try_get<MaterialComponent>(entity))
		material = mat->material;

	model_component.instances.resize(model_component.model->getMeshes().size());
	for (size_t i = 0; i < model_component.model->getMeshes().size(); ++i)
	{
		InstanceData instance_data_copy = instance_data;
		model_component.instances[i] = makeShared<RenderedInstance>(material);
		model_component.instances[i]->images = model_component.model->getImages()[i];
		Renderer::createInstance(model_component.instances[i], model_component.model->getMeshes()[i], std::move(instance_data_copy));
	}
}

void Scene::onModelComponentDestroy(entt::registry& registry, entt::entity entity)
{
	ModelComponent& model_component = registry.get<ModelComponent>(entity);
	for (size_t i = 0; i < model_component.model->getMeshes().size(); ++i)
		Renderer::destroyInstance(*model_component.instances[i]);
}

void Scene::onLightComponentCreate(entt::registry& registry, entt::entity entity)
{
	LightComponent& light_component = registry.get<LightComponent>(entity);

	Light* light = Renderer::addLight(light_component.light);
	light_component.light_ptr = light;
	if (auto transform = registry.try_get<TransformComponent>(entity))
		light->position = transform->transform * vec4(light_component.light.position, 1);
}

void Scene::onLightComponentDestroy(entt::registry& registry, entt::entity entity)
{
	(*registry.get<LightComponent>(entity).light_ptr) = Light{};
}