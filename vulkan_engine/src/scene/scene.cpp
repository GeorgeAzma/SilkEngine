#include "scene.h"
#include "entity.h"
#include "components.h"

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::onPlay()
{
	registry.view<ScriptComponent>().each(
		[=](auto entity, auto& script_component)
		{
			if (!script_component.instance)
			{
				script_component.instance = script_component.instantiate_script();
				script_component.instance->entity = Entity{ entity, this };

				script_component.instance->onCreate();
			}
		});
}

void Scene::onUpdate()
{
	registry.view<ScriptComponent>().each(
		[=](auto entity, auto& script_component)
		{
			script_component.instance->onUpdate();
		});
}

void Scene::onStop()
{
	registry.view<ScriptComponent>().each(
		[=](auto entity, auto& script_component)
		{
			script_component.instance->onDestroy();
		});
}

Entity Scene::createEntity()
{
	return { registry.create(), this };
}
