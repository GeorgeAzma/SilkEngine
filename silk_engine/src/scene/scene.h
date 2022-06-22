#pragma once

#include "instance.h"
#include "camera/camera.h"
#include <entt/entt.hpp>

class Entity;
class WindowResizeEvent;

class Scene
{
	friend class Entity;

public:
	Scene();
	~Scene();

	void onPlay();
	void onUpdate();
	void onStop();

	shared<Entity> createEntity();
	void removeEntity(const entt::entity& entity);

	void onTransformComponentUpdate(entt::registry& registry, entt::entity entity);
	void onColorComponentUpdate(entt::registry& registry, entt::entity entity);
	void onMaterialComponentUpdate(entt::registry& registry, entt::entity entity);
	void onLightComponentUpdate(entt::registry& registry, entt::entity entity);
	void onImageComponentUpdate(entt::registry& registry, entt::entity entity);
	void onTextComponentUpdate(entt::registry& registry, entt::entity entity);

	Camera* getMainCamera();

private:
	void onWindowResize(const WindowResizeEvent& e);
	
	void onMeshComponentCreate(entt::registry& registry, entt::entity entity);
	void onTextComponentCreate(entt::registry& registry, entt::entity entity);
	void onModelComponentCreate(entt::registry& registry, entt::entity entity);	
	void onLightComponentCreate(entt::registry& registry, entt::entity entity);
	
	void onMeshComponentDestroy(entt::registry& registry, entt::entity entity);
	void onModelComponentDestroy(entt::registry& registry, entt::entity entity);
	void onLightComponentDestroy(entt::registry& registry, entt::entity entity);

private:
	entt::registry registry;
	bool stopped = false;
};