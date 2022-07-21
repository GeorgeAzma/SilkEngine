#pragma once

#include "scene.h"

class SceneManager
{
public:
	static void add(const shared<Scene>& scene);
	static void remove(const shared<Scene>& scene);

	static void update();
	static void destroy();

	static void switchTo(const shared<Scene>& scene);
	static shared<Scene>& getActive() { return active_scene; }

private:
	static inline shared<Scene> active_scene = nullptr;
	static inline std::unordered_map<size_t, shared<Scene>> scenes;
};