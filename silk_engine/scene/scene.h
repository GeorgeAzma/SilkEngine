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
	static void addScene(const shared<Scene>& scene);
	static void removeScene(const shared<Scene>& scene);

	static void updateScenes();
	static void destroyScenes();

	static void switchTo(const shared<Scene>& scene);
	static shared<Scene>& getActive() { return active_scene; }

private:
	static inline shared<Scene> active_scene = nullptr;
	static inline std::unordered_map<size_t, shared<Scene>> scenes;
};