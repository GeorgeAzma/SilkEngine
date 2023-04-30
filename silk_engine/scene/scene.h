#pragma once

#include "utils/type_info.h"
#include "core/event.h"
#include <entt/entt.hpp>

class Entity;
class Camera;

class Scene
{
	friend class Entity;
	friend class SceneManager;

public:
	Scene();
	~Scene();

	virtual void onStart() {}
	virtual void onUpdate() {}
	virtual void onStop() {}

	shared<Entity> createEntity();
	void removeEntity(const entt::entity& entity);

	Camera* getMainCamera();

private:
	void init();
	void update();
	void destroy();

	void onWindowResize(const WindowResizeEvent& e);

private:
	std::vector<shared<Entity>> entities;
	Camera* main_camera = nullptr;
	entt::registry registry;

public:
	static Scene* getActive() { return active_scene; }
	static void setActive(Scene* scene) 
	{
		if (active_scene == scene)
			return;
		if (active_scene)
			active_scene->onStop();
		active_scene = scene; 
		if (active_scene)
			active_scene->onStart(); 
	}
	
private:
	static inline Scene* active_scene = nullptr;
};