#pragma once

#include "core/event.h"

class Entity;

class Scene
{
	friend class Entity;
public:
	Scene();
	~Scene();

	void onPlay();
	void onUpdate();
	void onStop();

	Entity createEntity();

private:
	void onWindowResize(const WindowResizeEvent& e);

private:
	entt::registry registry;
};