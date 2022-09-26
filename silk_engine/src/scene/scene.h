#pragma once

#include "system.h"
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

	template<typename T>
	T* getSystem() const
	{
		auto it = systems.find(TypeInfo::getTypeID<T>());
		if (it == systems.end() || !it->second)
			return nullptr;
		return (T*)it->second.get();
	}

	template<typename T>
	void addSystem()
	{
		remove<T>();
		systems.emplace(TypeInfo<System>::getTypeID<T>(), makeUnique<T>());
	}

	template<typename T>
	void removeSystem()
	{
		systems.erase(TypeInfo<System>::getTypeID<T>());
	}

	void clearSystems()
	{
		systems.clear();
	}

	shared<Entity> createEntity();
	void removeEntity(const entt::entity& entity);

	Camera* getMainCamera();

private:
	void init();
	void update();
	void destroy();

	void onWindowResize(const WindowResizeEvent& e);
	
	void onMeshComponentCreate(entt::registry& registry, entt::entity entity);
	void onTextComponentCreate(entt::registry& registry, entt::entity entity);
	void onModelComponentCreate(entt::registry& registry, entt::entity entity);	
	void onLightComponentCreate(entt::registry& registry, entt::entity entity);

	void onTransformComponentUpdate(entt::registry& registry, entt::entity entity);
	void onColorComponentUpdate(entt::registry& registry, entt::entity entity);
	void onMaterialComponentUpdate(entt::registry& registry, entt::entity entity);
	void onLightComponentUpdate(entt::registry& registry, entt::entity entity);
	void onImageComponentUpdate(entt::registry& registry, entt::entity entity);
	void onTextComponentUpdate(entt::registry& registry, entt::entity entity);
	
	void onMeshComponentDestroy(entt::registry& registry, entt::entity entity);
	void onModelComponentDestroy(entt::registry& registry, entt::entity entity);
	void onLightComponentDestroy(entt::registry& registry, entt::entity entity);

private:
	std::vector<shared<Entity>> entities;
	Camera* main_camera = nullptr;
	std::unordered_map<TypeID, unique<System>> systems;
	entt::registry registry;
};