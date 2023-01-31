#include "scene_manager.h"

void SceneManager::add(const shared<Scene>& scene)
{
	scenes.emplace((size_t)scene.get(), scene);
	if (!active_scene.get())
		switchTo(scene);
}

void SceneManager::remove(const shared<Scene>& scene)
{
	if (scene == active_scene)
		active_scene = nullptr;
	scenes.erase((size_t)scene.get());
}

void SceneManager::update()
{
	if (!active_scene.get())
		return;
	active_scene->update();
}

void SceneManager::destroy()
{
	if (active_scene.get())
		active_scene->destroy();
	scenes.clear();
}

void SceneManager::switchTo(const shared<Scene>& scene)
{
	if (active_scene.get())
		active_scene->destroy();
	active_scene = scenes.at((size_t)scene.get());
	active_scene->init();
}
