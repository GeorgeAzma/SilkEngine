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
#include "gfx/render_context.h"
#include "meshes/text_mesh.h"
#include "camera/camera.h"
#include "gfx/renderer.h"

Scene::Scene()
{
	Dispatcher::subscribe(this, &Scene::onWindowResize);

	SK_TRACE("Scene created");
}

Scene::~Scene()
{
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
	for (auto& [type_id, system] : systems)
		if (system->enabled)
			system->update();

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